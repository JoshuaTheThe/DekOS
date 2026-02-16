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
        char name[32];
        username(name, 31);
        print("Hello, World!\n");

        snprintf(current_dir, sizeof(current_dir), "users/%s/", name);
        snprintf(buf, sizeof(buf), "users/%s/user.ini", name);

        for (size_t i = 0; i < argc; ++i)
        {
                char buf[1024];
                snprintf(buf, sizeof(buf), " [INFO] launched with argument: %s\n", argv[i]);
                print(buf);
        }

        snprintf(buf, sizeof(buf), " [INFO] launched from: %d\n", ParentProc);
        print(buf);
        snprintf(buf, sizeof(buf), " [INFO] launched by: %d\n", UserID);
        print(buf);

        /**
         * Removing this causes an issue (literally, what), yes, this currently causes stack corruption, for no reason
         */
        // int rid = dlload("users/root/video");
        // if (rid == -1)
        // {
        //         print("could not load video\n");
        // }
        // else
        // {
        //         void (*_video_start)(int,char **,USERID,PID) = dlfind(rid, "main");
        //         snprintf(buf, sizeof(buf), " [DEBUG] VIDEO main found at 0x%x\n", _video_start);
        //         print(buf);
        //         dlunload(rid);
        // }

        bool running = true;

        while (running)
        {
                snprintf(buf, sizeof(buf), "%s:%s$ ", name, current_dir);
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
                                snprintf(buf, sizeof(buf), "%s/%s", current_dir, &command_buffer[0][2]);
                                pid = create(largc, arg_v, buf);
                        }

                        /* User/Bin */
                        else
                        {
                                snprintf(buf, sizeof(buf), "users/%s/%s", name, command_buffer[0]);
                                pid = create(largc, arg_v, buf);
                        }

                        if (!pid)
                        {
                                snprintf(buf, sizeof(buf), " [ERROR] could not launch: %s", command_buffer[0]);
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
