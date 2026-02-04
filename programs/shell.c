#ifdef __WIN32
#include "../dekoslibc/stdio.h"
#include "../dekoslibc/string.h"
#include "../dekoslibc/ini.h"
#else
#include <stdio.h>
#include <string.h>
#include <ini.h>
#endif

void print(const char *s)
{
        for (unsigned int i = 0; s[i]; ++i)
                putc(s[i]);
}

#define SHELL_KBD_BUFF_SIZE 64
#define SHELL_MAX_ARGS 8

static char keyboard_buffer[SHELL_KBD_BUFF_SIZE];
static char command_buffer[SHELL_MAX_ARGS][SHELL_KBD_BUFF_SIZE];

static size_t shellParse(char *b, char cmd[SHELL_MAX_ARGS][SHELL_KBD_BUFF_SIZE])
{
        size_t N = 0, i = 0;

        while (*b && N < SHELL_MAX_ARGS)
        {
                while (*b == ' ')
                        b++;
                if (!*b)
                        break;

                i = 0;

                if (*b == '"')
                {
                        b++;
                        while (*b && *b != '"' && i < SHELL_KBD_BUFF_SIZE - 1)
                        {
                                if (*b == '\\' && *(b + 1))
                                {
                                        b++;
                                        if (*b == 'n')
                                                cmd[N][i++] = '\n';
                                        else
                                                cmd[N][i++] = *b;
                                }
                                else
                                {
                                        cmd[N][i++] = *b;
                                }
                                b++;
                        }
                        if (*b == '"')
                                b++;
                }
                else
                {
                        while (*b && *b != ' ' && i < SHELL_KBD_BUFF_SIZE - 1)
                                cmd[N][i++] = *b++;
                }

                cmd[N][i] = '\0';
                N++;
        }

        return N;
}

int main(USERID UserID, PID ParentProc, size_t argc, char **argv)
{
        char name[32];
        getusername(name, 31);
        char prompt[256];
        char current_dir[256] = "";

        snprintf(current_dir, sizeof(prompt), "users/%s/", name);
        snprintf(prompt, sizeof(prompt), "users/%s/user.ini", name);
        Ini config = IniRead(prompt);
        memset(prompt, 0, sizeof(prompt));

        for (size_t i = 0; i < argc; ++i)
        {
                print(argv[i]);
                print("\n");
        }

        bool running = true;

        while (running)
        {
                snprintf(prompt, sizeof(prompt), "%s:%s$ ", name, current_dir);
                print(prompt);
                memset(keyboard_buffer, 0, SHELL_KBD_BUFF_SIZE);
                memset(command_buffer, 0, sizeof(command_buffer));
                size_t len = (size_t)gets(keyboard_buffer, SHELL_KBD_BUFF_SIZE - 1);
                keyboard_buffer[len] = '\0';
                size_t largc = shellParse(keyboard_buffer, (char(*)[SHELL_KBD_BUFF_SIZE]) & command_buffer);

                if (largc == 0)
                        continue;
                else if (!strcmp(command_buffer[0], "cd") && largc == 2)
                {
                        if (!strcmp(command_buffer[1], ".."))
                        {
                                char *last_slash = strrchr(current_dir, '/');
                                if (last_slash != current_dir)
                                {
                                        *last_slash = '\0';
                                }
                                else
                                {
                                        current_dir[1] = '\0';
                                }
                        }
                        else if (!strcmp(command_buffer[1], "/"))
                        {
                                strncpy(current_dir, "", 512);
                        }
                        else
                        {
                                char new_dir[512];
                                if (command_buffer[1][0] == '/')
                                {
                                        strncpy(new_dir, command_buffer[1], 512);
                                }
                                else
                                {
                                        snprintf(new_dir, sizeof(new_dir), "%s/%s", current_dir, command_buffer[1]);
                                }

                                len = strlen(new_dir);
                                while (len > 1 && new_dir[len - 1] == '/')
                                {
                                        new_dir[len - 1] = '\0';
                                        len--;
                                }

                                strncpy(current_dir, new_dir, 512);
                        }
                }
                else if (!strcmp(command_buffer[0], "pwd"))
                {
                        print(current_dir);
                        putc('\n');
                }
                else if (!strcmp(command_buffer[0], "exit"))
                {
                        running = false;
                }
                else if (strlen(command_buffer[0]) != 0)
                {
                        char **arg_v = malloc(largc * sizeof(char *));
                        for (size_t i = 0; i < largc; ++i)
                        {
                                size_t len = strlen(command_buffer[i]) + 1;
                                arg_v[i] = malloc(len);
                                memcpy(arg_v[i], command_buffer[i], len);
                        }

                        PID pid = CreateProcess(arg_v, largc);
                        while (progexists(pid))
                                ;
                        for (size_t i = 0; i < largc; ++i)
                                free(arg_v[i]);
                        free(arg_v);
                }
        }
        return 69;
}
