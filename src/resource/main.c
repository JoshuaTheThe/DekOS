#include <resource/main.h>
#include <memory/string.h>

KRNLRES grResources = {0};
RESULT resLast = RESULT_OK;
int gInvalidCount = 0;
static RID counter = 0;

/**
 * ResourceRecursiveSearch - basically seach by rid
 */
static KRNLRES *ResourceRecursiveSearch(KRNLRES *rpBase, RID targetRid, int depth, BOOL *found)
{
        PROCID procIdSelf = schedGetCurrentPid();
        BOOL temp = FALSE;
        if (depth > MAX_RESOURCE_DEPTH)
                return NULL;

        if (!rpBase)
                return NULL;

        KRNLRES *rp = rpBase->FirstChild, *rp2;
        while (rp)
        {
                if (rp == rpBase)
                        return NULL;

                if (rp->rid == targetRid)
                {

                        if (found)
                                *found = TRUE;
                        if (rp->Owner.num == procIdSelf.num)
                        {
                                return rp;
                        }
                        return NULL;
                }

                rp2 = ResourceRecursiveSearch(rp, targetRid, depth + 1, &temp);
                if (rp2 || temp)
                        return rp2;

                rp = rp->NextSibling;
        }

        return NULL;
}

/**
 * ResourceGetFromRID - return the pointer
 * to a resource, find by its id.
 */
KRNLRES *ResourceGetFromRID(RID rdResourceId)
{
        KRNLRES *rp;
        PROCID current = schedGetCurrentPid();

        if (rdResourceId == INVALID_RID)
                return NULL;

        rp = ResourceRecursiveSearch(&grResources, rdResourceId, 0, NULL);
        if (!rp)
                return NULL;

        if (rp->Owner.num != current.num)
                return NULL;

        return rp;
}

/**
 * ResourceBlit - write a buffer to the resource.
 */
RESULT ResourceBlitK(KRNLRES *rpResource,
                     RAWPTR rBuffer,
                     SIZE szBufferSize,
                     SIZE szCopy,
                     SIZE szOffset)
{
        PROCID pdSelf = schedGetCurrentPid();

        /**
         * Null Check.
         */
        if (!rpResource || !rBuffer || !szBufferSize)
        {
                return RESULT_INVALID_ARGUMENTS;
        }

        /**
         * Ownership Check.
         */
        if (rpResource->Owner.num != pdSelf.num)
        {
                return RESULT_PERMISSIONS_ERROR;
        }

        /**
         * Integrity Check.
         */
        if ((uintptr_t)rpResource->Region.ptr > _heap_end)
        {
                return RESULT_CORRUPTED;
        }

        /**
         * Bounding Checks
         */
        if (szBufferSize < szCopy || (rpResource->Region.size - szOffset) < szCopy)
        {
                return RESULT_BOUNDING_OVERFLOW;
        }

        memcpy((uint8_t *)rpResource->Region.ptr + szOffset, rBuffer, szCopy);
        return RESULT_OK;
}

/**
 * ResourceReleaseK - give a resource to the kernel, and mark as unused.
 */
RESULT ResourceReleaseK(KRNLRES *rpResource)
{
        PROCID pdSelf = schedGetCurrentPid();
        PROCID pdKernel = schedGetKernelPid();
        RESULT res;

        /**
         * Null Check.
         */
        if (!rpResource)
        {
                return RESULT_INVALID_ARGUMENTS;
        }

        /**
         * Ownership Check.
         */
        if (rpResource->Owner.num != pdSelf.num)
        {
                return RESULT_PERMISSIONS_ERROR;
        }

        /**
         * Cannot delete an entire tree.
         */
        if (rpResource->FirstChild)
        {
                return RESULT_HAS_CHILDREN;
        }

        /**
         * Integrity Check.
         */
        if ((uintptr_t)rpResource->Region.ptr > _heap_end)
        {
                return RESULT_CORRUPTED;
        }

        /**
         * Unlink, Handover and mark unused
         */
        res = ResourceUnlinkK(rpResource);
        if (res == RESULT_OK)
        {
                rpResource->Owner = pdKernel;
                rpResource->Used = FALSE;
        }
        return res;
}

/**
 * ResourceHandover - give a resource to another proc.
 */
RESULT ResourceHandoverK(KRNLRES *rpResource, PROCID dest)
{
        PROCID pdSelf = schedGetCurrentPid();

        /**
         * IDK why this would happen.
         */
        if (dest.num == pdSelf.num)
        {
                return RESULT_NO_OPERATION;
        }

        /**
         * Check the destination
         */
        if (!schedValidatePid(dest))
        {
                return RESULT_ARGUMENT_DOES_NOT_EXIST;
        }

        /**
         * Null Check.
         */
        if (!rpResource)
        {
                return RESULT_INVALID_ARGUMENTS;
        }

        /**
         * Ownership Check.
         */
        if (rpResource->Owner.num != pdSelf.num)
        {
                return RESULT_PERMISSIONS_ERROR;
        }

        /**
         * Cannot handover an entire tree.
         * (for now)
         */
        if (rpResource->FirstChild)
        {
                return RESULT_HAS_CHILDREN;
        }

        /**
         * Integrity Check.
         */
        if (((uintptr_t)rpResource->Region.ptr > _heap_end) && rpResource->Type != RESOURCE_TYPE_RAW_FAR)
        {
                return RESULT_CORRUPTED;
        }

        /**
         * No need to unlink
         */
        rpResource->Owner = dest;

        /**
         * Explicitly set because i don't trust myself.
         * Or the compiler.
         * Or the CPU.
         */
        rpResource->Used = TRUE;
        return RESULT_OK;
}

/**
 * ResourceCleanup - Cleanup unused resources.
 * Can only be used by the kernel process.
 */
RESULT ResourceCleanupK(int *invalidCount)
{
        KRNLRES *rp, *rpNxt;
        PROCID procId = schedGetCurrentPid();
        PROCID krnlId = schedGetKernelPid();

        /**
         * Some dumbass may pass NULL
         * instead of wanting to know.
         */
        if (!invalidCount)
        {
                invalidCount = &gInvalidCount;
        }

        (*invalidCount) = 0;

        /**
         * Only the kernel can cleanup unused resources.
         */
        if (procId.num != krnlId.num)
        {
                return RESULT_PERMISSIONS_ERROR;
        }

        /**
         * The "holy Fuck" Error.
         * For it is impossible to
         * A. call this from user functions* unless it is exposed.
         * B. be executing code from an invalid proc.
         * TODO - we should cause a kernel panic here tbh
         * but that is not for today
         */
        else if (!procId.valid)
        {
                return RESULT_CORRUPTED_SCHEDULER;
        }

        /**
         * Only the kernel should be able to get here.
         * Start look for dead resources.
         * It should* be impossible for children to have different owners.
         * ResourceHandover() will manage changing in ownership.
         */
        rp = grResources.FirstChild;
        while (rp)
        {
                /**
                 * If it is unused or if
                 * the owner no longer exists
                 * delete.
                 */
                if (!rp->Used || !schedValidatePid(rp->Owner))
                {
                        /* Save next Sibling */
                        rpNxt = rp->NextSibling;
                        /**
                         * Fully unlink.
                         */
                        if (rp->NextSibling)
                                rp->NextSibling->PreviousSibling = rp->PreviousSibling;
                        if (rp->PreviousSibling)
                                rp->PreviousSibling->NextSibling = rp->NextSibling;
                        rp->NextSibling = (KRNLRES *)NULL;
                        rp->PreviousSibling = (KRNLRES *)NULL;

                        /**
                         * Cleanup - we clear data before releasing.
                         * Some dumbasses may try to free the data before-hand.
                         */
                        if (rp->Region.size > 0 && rp->Region.ptr && rp->OnHeap)
                        {
                                memset(rp->Region.ptr, 0, rp->Region.size);
                                u_map(rp->Region);
                        }
                        else
                        {
                                (*invalidCount)++;
                        }

                        memset(rp, 0, sizeof(*rp));
                        free(rp);

                        rp = rpNxt;
                }
                else
                        rp = rp->NextSibling;
        }

        return (*invalidCount) > 0 ? RESULT_OK_WITH_ERRORS : RESULT_OK;
}

BOOL ResourceIsValidType(RESTYPE rtType)
{
        /**
         * There are no unordered types.
         * It is entirely sequential, therefore
         * we can get away with bounds.
         * 0 is Invalid.
         */
        return (rtType > RESOURCE_TYPE_ERROR && rtType < RESOURCE_TYPE_COUNT);
}

/**
 * ResourceUnlinkK - Unlink a resource from the linked list, and place in the global pool
 * Resources MUST ALWAYS be at least in the pool, for otherwise the kernel does not
 * know of their existence.
 */
RESULT ResourceUnlinkK(KRNLRES *rpResource)
{
        PROCID procId = schedGetCurrentPid();

        /**
         * Null Check.
         */
        if (!rpResource)
        {
                return RESULT_INVALID_ARGUMENTS;
        }

        /**
         * Permissions Check.
         */
        if (rpResource->Owner.num != procId.num)
        {
                return RESULT_PERMISSIONS_ERROR;
        }

        /**
         * Cannot Unlink something with child resources.
         */
        if (rpResource->FirstChild)
        {
                return RESULT_HAS_CHILDREN;
        }

        /**
         * No point doing it twice.
         */
        if (!rpResource->Parent &&
            !rpResource->NextSibling &&
            !rpResource->PreviousSibling)
        {
                return RESULT_ALREADY_UNLINKED;
        }

        /**
         * For when it is a child
         * with no siblings
         * that isn't pointed to.
         */
        if (rpResource->Parent &&
            rpResource->Parent->FirstChild != rpResource &&
            !rpResource->PreviousSibling)
        {
                return RESULT_CORRUPTED;
        }

        /**
         * Shift siblings together.
         */
        if (rpResource->NextSibling)
                rpResource->NextSibling->PreviousSibling = rpResource->PreviousSibling;
        if (rpResource->PreviousSibling)
                rpResource->PreviousSibling->NextSibling = rpResource->NextSibling;

        /**
         * Adjust Parent if they're pointing to us.
         */
        if (rpResource->Parent && rpResource->Parent->FirstChild == rpResource)
                rpResource->Parent->FirstChild = rpResource->NextSibling;

        /**
         * We do not allow hanging resources.
         */
        rpResource->NextSibling = grResources.FirstChild;
        rpResource->PreviousSibling = (KRNLRES *)NULL;
        rpResource->Parent = &grResources;
        grResources.FirstChild = rpResource;

        return RESULT_OK;
}

/**
 * ResourceCreateK - Create a new resource for a
 * program, of a given type,
 * with a given (optional) parent.
 * the type must be valid, on failiure it returns null.
 * otherwise, it returns the pointer to the resource.
 * the get the error code, there is the optional result argument.
 */
KRNLRES *ResourceCreateK(KRNLRES *rpParentResource,
                         RESTYPE rtType,
                         SIZE szBytes,
                         PROCID idOwner,
                         RESULT *result)
{
        /**
         * Variables
         */
        KRNLRES *rpSibling, *rpResource, *rpParent = rpParentResource;
        HREGION rDataRegion = {0};

        if (!result)
                result = &resLast;

        /**
         * Validate input
         */
        if (!schedValidatePid(idOwner) ||
            !ResourceIsValidType(rtType))
        {
                *result = RESULT_INVALID_ARGUMENTS;
                return (KRNLRES *)NULL;
        }

        /**
         * Memory allocation for data and null check.
         */
        if (!szBytes)
                goto AfterAllocation;
        rDataRegion = m_map(szBytes);
        if (!rDataRegion.ptr || !rDataRegion.size)
        {
                *result = RESULT_FAILED_TO_MAP;
                return (KRNLRES *)NULL;
        }
AfterAllocation:

        /**
         * Memory allocation for body and null check.
         */
        rpResource = malloc(sizeof(KRNLRES));
        if (!rpResource)
        {
                *result = RESULT_FAILED_TO_ALLOCATE;
                u_map(rDataRegion);
                return (KRNLRES *)NULL;
        }

        /**
         * Basic Initialisation
         */
        rpResource->Owner = idOwner;
        rpResource->Region = rDataRegion;
        rpResource->Type = rtType;
        rpResource->NextSibling = (KRNLRES *)NULL;
        rpResource->PreviousSibling = (KRNLRES *)NULL;
        rpResource->FirstChild = (KRNLRES *)NULL;
        rpResource->Parent = (KRNLRES *)NULL;
        rpResource->OnHeap = TRUE;
        rpResource->rid = counter++;

        /**
         * When you are done with a resource, simply
         * unlink and mark as unused.
         *
         * Do NOT call FREE() yourself.
         */
        rpResource->Used = TRUE;

        /**
         * IF (A parent resource is NOT provided)
         * { place into unordered pool }
         * */
        if (!rpParent)
                rpParent = &grResources /* Global Variable */;
        rpResource->Parent = rpParent;

        rpSibling = rpParent->FirstChild;
        rpParent->FirstChild = rpResource;
        rpResource->NextSibling = rpSibling;

        /**
         * Update sibling if present
         */
        if (rpSibling)
                rpSibling->PreviousSibling = rpResource;
        *result = RESULT_OK;
        return rpResource;
}

/**
 * Non K variants
 */
RID ResourceCreate(RID rdParentResource,
                   RESTYPE rtType,
                   SIZE szBytes,
                   PROCID idOwner,
                   RESULT *result)
{
        KRNLRES *parent = ResourceGetFromRID(rdParentResource);
        KRNLRES *new = ResourceCreateK(parent, rtType, szBytes, idOwner, result);
        if (!new || *result != RESULT_OK)
                return INVALID_RID;
        return new->rid;
}

RESULT ResourceHandover(RID rdResource, PROCID dest)
{
        KRNLRES *rp = ResourceGetFromRID(rdResource);
        return ResourceHandoverK(rp, dest);
}

RESULT ResourceRelease(RID rdResource)
{
        KRNLRES *rp = ResourceGetFromRID(rdResource);
        return ResourceReleaseK(rp);
}

/**
 * ResourceData - return the pointer to the data in the resource.
 */
RAWPTR ResourceData(RID rdResource)
{
        KRNLRES *rp = ResourceGetFromRID(rdResource);
        if (!rp)
                return NULL;
        return rp->Region.ptr;
}

/**
 * ResourceSize - return the size of the data in the resource.
 */
SIZE ResourceSize(RID rdResource)
{
        KRNLRES *rp = ResourceGetFromRID(rdResource);
        return rp->Region.size;
}

/**
 * ResourceNextOfType - return the pointer to the next resource of a given type,
 * Does not check child resources.
 */
KRNLRES *ResourceNextOfType(KRNLRES **P, RESTYPE Type)
{
        if(!P||!*P||Type>=RESOURCE_TYPE_COUNT)        
                return(NULL);
        do
        {
                (*P)=(*P)->NextSibling;
        }
        while ((*P)&&(*P)->Type!=Type);

        return(*P);
}
