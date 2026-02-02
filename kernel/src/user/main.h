#ifndef USER_H
#define USER_H

#include <utils.h>
#include <drivers/storage.h>

#define USER_NAME_LENGTH (0x0C)

#define USER_FLAG_ADMINISTRATOR (1 << 0)
#define USER_FLAG_SYSTEM (1 << 1)
#define USER_FLAG_SERVICE (1 << 2)

#define USER_MAXIMUM_SUPPORTED_MAJOR (1)
#define USER_MAXIMUM_SUPPORTED_MINOR (0)
#define USER_MAXIMUM_SUPPORTED_PATCH (0)

/* Loaded from /dev(N)/System/Users/users.dat */
typedef struct __attribute__((__packed__))
{
        char Name[USER_NAME_LENGTH];
        DWORD Flags;
        DWORD UserId;
} USER;

typedef struct __attribute__((__packed__))
{
        char Magic[4]; /* "USER" */
        BYTE VersionMajor; /* 1 */
        BYTE VersionMinor; /* 0 */
        BYTE VersionPatch; /* 0 */
        BYTE Reserved[1];
        DWORD UserCount;

        /* Followed by Users */
} USERInfoHeader;

#endif
