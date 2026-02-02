#include <ini/main.h>

Ini IniRead(const char *iniPath)
{
        Ini config = {0};

        char *Data = SMGetDrive()->ReadFile(SMGetDrive(), iniPath);
        if (!Data)
                return config;

        char *line = strtok(Data, "\n");
        while (line != NULL && config.count < MAX_CONFIG_VARS)
        {
                if (line[0] == '#' || line[0] == '\0')
                {
                        line = strtok(NULL, "\n");
                        continue;
                }

                char *equals = strchr(line, '=');
                if (equals)
                {
                        size_t name_len = equals - line;
                        if (name_len >= MAX_VAR_NAME)
                                name_len = MAX_VAR_NAME - 1;
                        strncpy(config.vars[config.count].name, line, name_len);
                        config.vars[config.count].name[name_len] = '\0';

                        strncpy(config.vars[config.count].value, equals + 1, MAX_VAR_VALUE - 1);
                        config.vars[config.count].value[MAX_VAR_VALUE - 1] = '\0';

                        config.count++;
                }

                line = strtok(NULL, "\n");
        }

        free(Data);
        return config;
}

const char *IniGet(Ini *config, const char *name)
{
        for (int i = 0; i < config->count; i++)
        {
                if (strcmp(config->vars[i].name, name) == 0)
                        return config->vars[i].value;
        }
        return NULL;
}
