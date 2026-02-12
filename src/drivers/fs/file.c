#include <drivers/fs/file.h>
#include <drivers/math.h>
#include <memory/string.h>
#include <memory/alloc.h>

struct _iobuf _iofiles[MAX_FILES] = {0};

SYSFILE *FAllocate(void)
{
        for (size_t i = 0; i < MAX_FILES; ++i)
        {
                if (_iofiles[i].flags & FILE_PRESENT)
                        continue;
                return &_iofiles[i];
        }

        return (SYSFILE *)NULL;
}

SYSFILE *FOpen(const char *const Path,
               const size_t PathLength,
               const int Flags)
{
        DRIVE *Drive = SMGetDrive();
        void *Dat = NULL;
        SYSFILE *File = NULL;
        if (!Path || !PathLength || PathLength >= MAX_PATH)
                return NULL;
        if ((Flags & FILE_PRESENT) == 0)
                return NULL;
        if (strnlen(Path, PathLength + 1) != PathLength)
                return NULL;
        if (!Drive)
                return NULL;
        const size_t FileSize = Drive->FileSize(Drive, Path);
        Dat = Drive->ReadFile(Drive, Path);
        if (!Dat || !FileSize)
                return NULL;
        File = FAllocate();
        if (!File)
                return NULL;
        memset(File, 0, sizeof(*File));
        File->flags = Flags;
        File->ptr = File->base = Dat;
        File->remaining = FileSize;
        File->size = FileSize;
        return File;
}

void FClose(SYSFILE *File)
{
        const DRIVE *Drive = SMGetDrive();
        if (!Drive)
                return;
        if (!File->ptr || !File->base)
                return;
        if (!(File->flags & FILE_PRESENT))
                return;
        if (File->flags & FILE_WRITABLE)
        {
                /**
                 * TODO - Implement writing to FS.
                 */
        }

        free(File->base);
        memset(File, 0, sizeof(*File));
}

size_t FRead(char *Buf, size_t N,
             size_t Size, SYSFILE *File)
{
        if (!File || !File->base ||
            !File->remaining || !N ||
            !Size || !Buf ||
            ~File->flags & FILE_READABLE)
                return -1;
        const size_t bytes_requested = N * Size;
        const size_t bytes_to_read = minu(bytes_requested, File->remaining);

        memcpy(Buf, File->ptr, bytes_to_read);
        File->ptr += bytes_to_read;
        return bytes_to_read;
}

        /** YOLO */
size_t FWrite(char *Buf, size_t N,
              size_t Size, SYSFILE *File)
{
        if (!N || !Size || !Buf || !File ||
            ~File->flags & FILE_WRITABLE)
                return -1;

        const size_t bytes_to_write = N * Size;
        const size_t new_size = bytes_to_write + File->size;
        char *dat = malloc(new_size);
        if (!dat)
                return -1;

        memcpy(dat, File->base, File->size);
        memcpy(&dat[File->size], Buf, bytes_to_write);
        free(File->base);

        File->base = dat;

        size_t old_offset = File->ptr - File->base;
        if (old_offset > File->size)
        {
                old_offset = File->size;
        }
        File->ptr = &File->base[old_offset];
        File->remaining = new_size - old_offset;
        File->size = new_size;
        return bytes_to_write;
}
