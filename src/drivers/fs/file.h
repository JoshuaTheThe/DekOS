#ifndef FILE_H_
#define FILE_H_

#include <utils.h>
#include <drivers/fs/storage.h>

/**
 *	A Maximum of 2^13 files may be open at once.
 * */
#define MAX_FILES (8192)
#define FILE_PRESENT (0x01)
#define FILE_READABLE (0x02)
#define FILE_WRITABLE (0x04)
#define FILE_EXECUTABLE (0x08)

typedef struct _iobuf
{
	char *ptr;
	size_t remaining;
	size_t size;
	char *base;
	int flags;
} SYSFILE;

extern struct _iobuf _iofiles[MAX_FILES];

SYSFILE *FAllocate(void);
SYSFILE *FOpen(const char *const Path,
	       const size_t PathLength,
	       const int Flags);
void FClose(SYSFILE *File);

size_t FRead(char *Buf, size_t N, size_t Size, SYSFILE *File);
size_t FWrite(char *Buf, size_t N, size_t Size, SYSFILE *File);

#endif

