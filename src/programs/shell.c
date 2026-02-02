#include <programs/shell.h>
#include <isr/system.h>
#include <memory/alloc.h>
#include <memory/string.h>
#include <tty/input/input.h>
#include <tty/output/output.h>
#include <drivers/storage.h>
#include <programs/ex.h>
#include <programs/delangue.h>
#include <programs/elf/elf.h>
#include <forth.h>

static char keyboard_buffer[SHELL_KBD_BUFF_SIZE];
static char command_buffer[SHELL_MAX_ARGS][SHELL_KBD_BUFF_SIZE];

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

void shellCoproc(void)
{
        delangueState_t state;
        char code[] = "DO 7+8. END.";
        state.current_working_file.current_offset = 0;
        state.current_working_file.length = sizeof(code);
        state.current_working_file.remaining = sizeof(code);
        state.current_working_file.raw_source = code;
        memset(&state.virtual_machine, 0, sizeof(state.virtual_machine));
        delangueParse(&state);
        exit(0);
}

void shell(void)
{
        char current_dir[512] = "";
        while (1)
        {
                printf("%s\\`(owo`)o -> ", current_dir);
                memset(keyboard_buffer, 0, SHELL_KBD_BUFF_SIZE);
                memset(command_buffer, 0, sizeof(command_buffer));
                size_t len = (size_t)gets(keyboard_buffer, SHELL_KBD_BUFF_SIZE - 1);
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
                                SMGetDrive()->DirectoryListing(SMGetDrive(), current_dir);
                        }
                        else if (argc == 2)
                        {
                                char path[512];
                                snprintf(path, 512, "%s/%s", current_dir, command_buffer[1]);
                                SMGetDrive()->DirectoryListing(SMGetDrive(), path);
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
                        printf("%s\n", current_dir);
                }
                else if (!strcmp(command_buffer[0], "type") && argc == 2)
                {
                        char path[512];
                        snprintf(path, 512, "%s/%s", current_dir, command_buffer[1]);
                        char *data = SMGetDrive()->ReadFile(SMGetDrive(), path);
                        if (data)
                        {
                                printf("%s\n", data);
                                free(data);
                        }
                        else
                        {
                                printf("could not find file '%s'\n", path);
                        }
                }
                else if (!strcmp(command_buffer[0], "size") && argc == 2)
                {
                        char path[512];
                        snprintf(path, 512, "%s/%s", current_dir, command_buffer[1]);
                        printf("%d\n", SMGetDrive()->FileSize(SMGetDrive(), path));
                }
                else if (!strcmp(command_buffer[0], "forth") && argc == 2)
                {
                        char path[512];
                        snprintf(path, 512, "%s/%s", current_dir, command_buffer[1]);
                        char *data = SMGetDrive()->ReadFile(SMGetDrive(), path);
                        if (data)
                        {
                                forth(data);
                                printf("\n");
                        }
                        else
                        {
                                printf("could not find file '%s'\n", path);
                        }
                }
                else if (!strcmp(command_buffer[0], "\\ex") && argc == 2)
                {
                        /* OLD EX FORMAT */
                        char path[512];
                        snprintf(path, 512, "%s/%s", current_dir, command_buffer[1]);
                        char *data = SMGetDrive()->ReadFile(SMGetDrive(), path);
                        if (data)
                        {
                                schedPid_t sysPid = {.num = 0, .valid = true};
                                int pid = exExecute(path, data, SMGetDrive()->FileSize(SMGetDrive(), path), sysPid);
                                while (progexists(pid))
                                {
                                        if (kbhit() && getc() == '\e')
                                        {
                                                schedPid_t p = {.num = pid, .valid = true};
                                                schedKillProcess(p);
                                                break;
                                        }
                                        hlt();
                                }

                                printf("\n");
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
                        char *data = SMGetDrive()->ReadFile(SMGetDrive(), path);
                        if (data)
                        {
                                schedPid_t sysPid = {.num = 0, .valid = true};
                                bool iself;
                                schedPid_t pid = elfLoadProgram((uint8_t*)data, SMGetDrive()->FileSize(SMGetDrive(), path), &iself);
                                if (!iself)
                                {
                                        pid.num = exExecute(path, data, SMGetDrive()->FileSize(SMGetDrive(), path), sysPid);
                                        pid.valid = pid.num != -1;
                                }
                                while (pid.valid && progexists(pid.num))
                                {
                                        if (kbhit() && getc() == '\e')
                                        {
                                                schedPid_t p = {.num = pid.num, .valid = true};
                                                schedKillProcess(p);
                                                break;
                                        }
                                        hlt();
                                }

                                printf("\n");
                        }
                        else
                        {
                                printf("could not find file '%s'\n", path);
                        }
                }
        }

        while (1);
}
