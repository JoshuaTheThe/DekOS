#ifndef USER_H
#define USER_H

#include <utils.h>
#include <drivers/storage.h>
#include <drivers/serial.h>

#define USER_NAME_LENGTH (0x0C)

#define USER_FLAG_ADMINISTRATOR (1 << 0)
#define USER_FLAG_SYSTEM (1 << 1)
#define USER_FLAG_SERVICE (1 << 2)
#define USER_FLAG_VALID (1 << 3)
#define USER_FLAG_CAN_LAUNCH (1 << 4)

#define USER_MAXIMUM_SUPPORTED_MAJOR (1)
#define USER_MAXIMUM_SUPPORTED_MINOR (0)
#define USER_MAXIMUM_SUPPORTED_PATCH (0)

#define MAX_USERS (8)

typedef size_t USERID;

/* Loaded from /dev(N)/System/Users/users.dat */
typedef struct __attribute__((__packed__))
{
        char Name[USER_NAME_LENGTH];
        DWORD Flags;
        DWORD UserId;
        DWORD PassHash;
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

void UserRem(size_t idx);
size_t UserAdd(USER NewUser);
int UserTest(size_t idx, int mask);
void UserSetPassword(USERID id, char *to, char *current);
bool UserMatch(USERID id, char *with);
uint32_t fnv1a_hash(const char *str);
USERID UserLogin(void);
void UserName(char *buf, size_t len, USERID Id);
void UsersLoad(void);

#endif
