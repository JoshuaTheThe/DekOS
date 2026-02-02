/**
 * This is very secure, trust
 */

#include <user/main.h>

#define PASSWORD_SALT 0x8095634 /* keyboard smash */

static USER users[MAX_USERS];
static size_t user_count;

uint32_t fnv1a_hash(const char *str)
{
        uint32_t hash = 2166136261u ^ PASSWORD_SALT;
        for (size_t i = 0; str[i]; i++)
        {
                hash ^= (uint8_t)str[i];
                hash *= 16777619u;
        }
        return hash;
}

static USER *FindInvalidUser(void)
{
        for (size_t i = 0; i < MAX_USERS; ++i)
        {
                if (users[i].Flags & USER_FLAG_VALID)
                        continue;
                return &users[i];
        }

        return NULL;
}

void UserSetPassword(USERID id, char *to, char *current)
{
        if (id >= MAX_USERS)
                return;

        DWORD currentHash = fnv1a_hash(current);
        if (users[id].PassHash != currentHash)
                return;

        users[id].PassHash = fnv1a_hash(to);
}

bool UserMatch(USERID id, char *with)
{
        if (id >= MAX_USERS)
                return false;
        DWORD hash = fnv1a_hash(with);
        if (users[id].PassHash == hash)
        {
                return true;
        }

        return false;
}

size_t UserAdd(USER NewUser)
{
        USER *user = FindInvalidUser();
        if (!user)
                return -1;
        *user = NewUser;
        user_count += 1;
        return user - users;
}

void UserRem(size_t idx)
{
        if (idx > 0 && idx < MAX_USERS)
                memset(&users[idx], 0, sizeof(USER));
}

int UserTest(size_t idx, int mask)
{
        if (idx > 0 && idx < MAX_USERS)
                return users[idx].Flags & mask;
        if (idx == 0)
                return 0xFF;
        return 0;
}
