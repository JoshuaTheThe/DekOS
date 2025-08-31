#include <shell.h>

char keyboard_buffer[SHELL_KBD_BUFF_SIZE];
char command_buffer[SHELL_MAX_ARGS][SHELL_KBD_BUFF_SIZE];

int parse(char *b, char cmd[SHELL_MAX_ARGS][SHELL_KBD_BUFF_SIZE])
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
        iso9660_directory file;
        while (1)
        {
                k_print("\\`(owo`)o -> ");
                gets(keyboard_buffer, SHELL_KBD_BUFF_SIZE - 1);
                keyboard_buffer[SHELL_KBD_BUFF_SIZE - 1] = '\0';
                int argc = parse(keyboard_buffer, (char(*)[SHELL_KBD_BUFF_SIZE]) & command_buffer);
                if (!strcmp(command_buffer[0], "fetch"))
                {
                        k_print("\n        /\\_/\\\n");
                        k_print("       | o o |\n");
                        k_print("/-----/-------\\------\\  DekOS\n");
                        k_print("|    This Is The,    |  Version %s\n", __VER__);
                        k_print("| The The The The :3 |\n");
                        k_print("|              -josh |\n");
                        k_print("\\--------------------/\n\n");
                }
                else
                {
                        bool success = iso9660_find_file(command_buffer[0], &file);
                        if (success)
                        {
                                k_print("file size=%d\n", file.data_length[0]);
                                char *data = iso9660_read_file(&file);
                                data[file.data_length[0]] = 0;
                                k_print("data:\n%s\n", data);
                                free(data);
                        }
                        else
                        {
                                k_print("could not find file '%s'\n", command_buffer[0]);
                        }
                }
        }
}
