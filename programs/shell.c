#include <stdio.h>
#include <string.h>
#include <ini.h>

#define SHELL_KBD_BUFF_SIZE 16
#define SHELL_MAX_ARGS 8

static char buf[MAX_PATH + 32];
static char current_dir[MAX_PATH + 32];
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

int create(int arg_c, char **arg_v, char *path)
{
        PID pid = createproc(path, arg_c, arg_v);
        if (pid == -1)
                return 0;
        return pid;
}

int main(uint32_t argc, char **argv, USERID UserID, PID ParentProc)
{
        (void)argc;
        (void)argv;
        char name[32];
        username(name, 31);
        print("Hello, World!\n");

        snprintf(current_dir, sizeof(current_dir)-1, "users/%s/", name);

        // KStack is fucked, removing this causes crash // early exit
        snprintf(buf, sizeof(buf)-1, "users/%s/user.ini", name);

        // for (size_t i = 0; i < argc; ++i)
        // {
        //         snprintf(buf, sizeof(buf)-1, " [INFO] launched with argument: %s\n", argv[i]);
        //         print(buf);
        // }

        snprintf(buf, sizeof(buf)-1, " [INFO] launched from: %d\n", ParentProc);
        print(buf);
        snprintf(buf, sizeof(buf)-1, " [INFO] launched by: %d\n", UserID);
        print(buf);
        snprintf(buf, sizeof(buf)-1, " [INFO] argc: %d\n", argc);
        print(buf);
        snprintf(buf, sizeof(buf)-1, " [INFO] argv: %d\n", argv);
        print(buf);

        bool running = true;

        while (running)
        {
                snprintf(buf, sizeof(buf)-1, "%s:%s$ ", name, current_dir);
                print(buf);
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
                                if (command_buffer[1][0] == '/')
                                {
                                        strncpy(buf, command_buffer[1], sizeof(buf)-1);
                                }
                                else
                                {
                                        snprintf(buf, sizeof(buf)-1, "%s/%s", current_dir, command_buffer[1]);
                                }

                                len = strlen(buf);
                                while (len > 1 && buf[len - 1] == '/')
                                {
                                        buf[len - 1] = '\0';
                                        len--;
                                }

                                strncpy(current_dir, buf, MAX_PATH);
                        }
                }
                else if (!strcmp(command_buffer[0], "pwd"))
                {
                        print(current_dir);
                        putch('\n');
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

                        int pid = 0;

                        /* Absolute */
                        if (command_buffer[0][0] == '/')
                        {
                                pid = create(largc, arg_v, &command_buffer[0][1]);
                        }

                        /* Local */
                        else if (command_buffer[0][0] == '.' && command_buffer[0][1] == '/')
                        {
                                snprintf(buf, sizeof(buf)-1, "%s/%s", current_dir, &command_buffer[0][2]);
                                pid = create(largc, arg_v, buf);
                        }

                        /* User/Bin */
                        else
                        {
                                snprintf(buf, sizeof(buf)-1, "users/%s/%s", name, command_buffer[0]);
                                pid = create(largc, arg_v, buf);
                        }

                        if (!pid)
                        {
                                snprintf(buf, sizeof(buf)-1, " [ERROR] could not launch: %s", command_buffer[0]);
                                print(buf);
                                putch('\n');
                        }

                        while (pid && checkproc(pid))
                                ;
                        for (size_t i = 0; i < largc; ++i)
                                free(arg_v[i]);
                        free(arg_v);
                }
        }
        return 0;
}
