/* Kernel Resource Handler */
#ifndef KERNEL_RESOURCE_H
#define KERNEL_RESOURCE_H

/**
 * Kernel Resource Manager API
 * Version: 1.0
 */

#define MAX_RESOURCE_DEPTH (32)
#define INVALID_RID (SIZE)(-1L)

#include <utils.h>
#include <memory/alloc.h>
#include <programs/scheduler.h>

/* Resource ID */
typedef SIZE RID;

typedef enum
{
        RESOURCE_TYPE_ERROR,
        RESOURCE_TYPE_RAW,
        RESOURCE_TYPE_PROGRAM,
        RESOURCE_TYPE_BITMAP_IMAGE,
        RESOURCE_TYPE_WINDOW,
        RESOURCE_TYPE_ELEMENT,
        RESOURCE_TYPE_COUNT,
} RESTYPE;

typedef struct KRNLRES
{
        region_t Region;
        struct KRNLRES *Parent;
        struct KRNLRES *NextSibling;
        struct KRNLRES *PreviousSibling;
        struct KRNLRES *FirstChild;
        RESTYPE Type;
        PROCID Owner;
        BOOL Used;
        BOOL OnHeap; /* why did we even have a non heap type anyway */
        RID rid;
} KRNLRES;

typedef void NONE;

/**
 * All Suffixed with K give the actual resource
 * Others give/use the handle
 */

/**
 * ResourceCreate(K) - Create a new resource for a
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
                         RESULT *result);

RID ResourceCreate(RID rdParentResource,
                   RESTYPE rtType,
                   SIZE szBytes,
                   PROCID idOwner,
                   RESULT *result);

/**
 * ResourceUnlinkK - Unlink a resource from the linked list, and place in the global pool
 * Resources MUST ALWAYS be at least in the pool, for otherwise the kernel does not
 * know of their existence.
 */
RESULT ResourceUnlinkK(KRNLRES *rpResource);
/* No User Equivilant */

/**
 * ResourceCleanupK - Cleanup unused resources.
 * Can only be used by the kernel process.
 */
RESULT ResourceCleanupK(int *invalidCount);
/* No User Equivilant */

/**
 * ResourceHandoverK - give a resource to another proc.
 */
RESULT ResourceHandoverK(KRNLRES *rpResource, PROCID dest);
RESULT ResourceHandover(RID rdResource, PROCID dest);

/**
 * ResourceReleaseK - give a resource to the kernel, and mark as unused.
 */
RESULT ResourceReleaseK(KRNLRES *rpResource);
RESULT ResourceRelease(RID rdResource);

BOOL ResourceIsValidType(RESTYPE rtType);
KRNLRES *ResourceGetFromRID(RID rdResourceId, BOOL Override);

/**
 * ResourceData - return the pointer to the data in the resource.
 */
RAWPTR ResourceData(RID rdResource);

/**
 * ResourceSize - return the size of the data in the resource.
 */
SIZE ResourceSize(RID rdResource);

/**
 * ResourceNextOfType - return the pointer to the next resource of a given type,
 * Does not check child resources.
 */
KRNLRES *ResourceNextOfType(KRNLRES **P, RESTYPE Type);

RESULT ResourceBlitK(KRNLRES *rpResource,
                     RAWPTR rBuffer,
                     SIZE szBufferSize,
                     SIZE szCopy,
                     SIZE szOffset);

extern RID counter;

#endif
