/**
 * This is very secure, trust
 */

#include <user/main.h>
#include <memory/string.h>
#include <drivers/dev/ps2/ps2.h>
#include <drivers/fs/storage.h>
#include <ini.h>

#define PASSWORD_SALT 0x8095634 /* keyboard smash */
#define MAX_LOGIN_ATTEMPTS 4

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

USERID UserFind(const char *Name)
{
        for (size_t i = 0; i < MAX_USERS; ++i)
        {
                if (users[i].Flags & USER_FLAG_VALID && !ncsstrncmp(Name, users[i].Name, USER_NAME_LENGTH))
                {
                        return i;
                }
        }

        return -1;
}

USERID UserLogin(void)
{
        char name[USER_NAME_LENGTH + 1] = {0};
        char pass[32] = {0};
        int attempts = 0;

        while (attempts < MAX_LOGIN_ATTEMPTS)
        {
                printf("\n--- DekOS Login ---\n");
                printf("Username: ");
                gets(name, USER_NAME_LENGTH);

                printf("Password: ");
                gets(pass, 31);

                USERID id = UserFind(name);
                if (id != (size_t)-1 && UserMatch(id, pass))
                {
                        memset(pass, 0, sizeof(pass));
                        printf("Login successful. Welcome, %s!\n", name);
                        return id;
                }

                memset(pass, 0, sizeof(pass));
                memset(name, 0, sizeof(name));
                printf("Invalid username or password.\n");
                attempts++;
        }

        printf("Maximum login attempts reached. Access denied.\n");
        return -1;
}

void UserName(char *buf, size_t len, USERID Id)
{
        if (Id < MAX_USERS)
                strncpy(buf, users[Id].Name, len);
}

void UsersLoad(void)
{
        memset(users, 0, sizeof(users));
        user_count = 0;

        Ini newusers = IniRead("users/users.ini");

        for (size_t i = 0; i < newusers.count; ++i)
        {
                USER User = {0};
                strncpy(User.Name, newusers.vars[i].name, USER_NAME_LENGTH-1);
                User.Name[USER_NAME_LENGTH-1] = '\0';
                User.UserId = i;
                User.Flags = USER_FLAG_VALID | USER_FLAG_CAN_LAUNCH;

                if (!strncmp(newusers.vars[i].value, "administrator", 14))
                {
                        User.Flags |= USER_FLAG_ADMINISTRATOR;
                }
                else if (!strncmp(newusers.vars[i].value, "service", 8)) /* for future driver shi */
                {
                        User.Flags |= USER_FLAG_SERVICE;
                }
                else if (!strncmp(newusers.vars[i].value, "system", 7)) /* for future system shi */
                {
                        User.Flags |= USER_FLAG_SYSTEM;
                }
                else if (!strncmp(newusers.vars[i].value, "guest", 6)) /* not allowed to do shi */
                {
                        User.Flags &= ~USER_FLAG_CAN_LAUNCH;
                }

                char path[512];
                snprintf(path, sizeof(path), "users/%s/user.ini", User.Name);
                Ini userData = IniRead(path);

                const char *pswrd = IniGet(&userData, "pswrdhash");

                if (pswrd)
                {
                        size_t len;
                        User.PassHash = atoi(pswrd, &len);
                }
                else
                {
                        User.PassHash = fnv1a_hash("1");
                }

                UserAdd(User);
        }

        for (size_t i = 0; i < user_count; ++i)
        {
                printf("%s\n", users[i].Name);
        }
}
