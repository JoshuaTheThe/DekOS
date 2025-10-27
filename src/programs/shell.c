#include <programs/shell.h>

char keyboard_buffer[SHELL_KBD_BUFF_SIZE];
char command_buffer[SHELL_MAX_ARGS][SHELL_KBD_BUFF_SIZE];

static int shellParse(char *b, char cmd[SHELL_MAX_ARGS][SHELL_KBD_BUFF_SIZE])
{
        int N = 0, i = 0;

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

void shell(void)
{
        iso9660Dir_t file;
        char current_dir[512] = "/boot";
        while (1)
        {
                printf("%s\\`(owo`)o -> ", current_dir);
                memset(keyboard_buffer, 0, SHELL_KBD_BUFF_SIZE);
                int len = gets(keyboard_buffer, SHELL_KBD_BUFF_SIZE - 1);
                keyboard_buffer[len] = '\0';
                int argc = shellParse(keyboard_buffer, (char(*)[SHELL_KBD_BUFF_SIZE]) & command_buffer);

                if (!strcmp(command_buffer[0], "fetch"))
                {
                        printf("\n        /\\_/\\\n");
                        printf("       | o o |\n");
                        printf("/-----/-------\\------\\  DekOS\n");
                        printf("|    This Is The,    |  Version %s\n", __VER__);
                        printf("| The The The The :3 |\n");
                        printf("|              -josh |\n");
                        printf("\\--------------------/\n\n");
                }
                else if (!strcmp(command_buffer[0], "tasks"))
                {
                        schedListProcesses();
                }
                else if (!strcmp(command_buffer[0], "ls"))
                {
                        if (argc == 1)
                        {
                                iso9660ListDir(current_dir);
                        }
                        else if (argc == 2)
                        {
                                char path[512];
                                snprintf(path, 512, "%s/%s", current_dir, command_buffer[1]);
                                iso9660ListDir(path);
                        }
                }
                else if (!strcmp(command_buffer[0], "cd") && argc == 2)
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
                                strncpy(current_dir, "/boot", 512);
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
                                        if (strcmp(current_dir, "/") == 0)
                                        {
                                                snprintf(new_dir, sizeof(new_dir), "/%s", command_buffer[1]);
                                        }
                                        else
                                        {
                                                snprintf(new_dir, sizeof(new_dir), "%s/%s", current_dir, command_buffer[1]);
                                        }
                                }

                                int len = strlen(new_dir);
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
                        printf("%s\n", current_dir);
                }
                else if (!strcmp(command_buffer[0], "type") && argc == 2)
                {
                        char path[512];
                        snprintf(path, 512, "%s/%s", current_dir, command_buffer[1]);
                        bool success = iso9660FindFile(path, &file);
                        if (success)
                        {
                                char *data = iso9660ReadFile(&file);
                                printf("%s\n", data);
                                free(data);
                        }
                        else
                        {
                                printf("could not find file '%s'\n", path);
                        }
                }
                else if (strlen(command_buffer[0]) != 0)
                {
                        char path[512];
                        snprintf(path, 512, "%s/%s", current_dir, command_buffer[0]);
                        bool success = iso9660FindFile(path, &file);
                        if (success)
                        {
                                char *data = iso9660ReadFile(&file);
                                exExecute(path, data, file.data_length[0]);
                        }
                        else
                        {
                                printf("could not find file '%s'\n", path);
                        }
                }
        }
}
