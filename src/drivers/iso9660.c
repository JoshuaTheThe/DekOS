#include <drivers/iso9660.h>
#include <tty/output/output.h>
#include <drivers/math.h>
#include <memory/string.h>
#include <memory/alloc.h>

static int cd_port = 0x1F0;
static bool cd_slave = false;
static iso9660PrimaryDesc_t primary_desc;
static bool iso9660_initialized = false;

bool iso9660Initialized(void)
{
        return iso9660_initialized;
}

bool iso9660Init(void)
{
        cdInit(&cd_port, &cd_slave);
        if (!iso9660ReadVolumeDescriptors())
        {
                return false;
        }

        iso9660_initialized = true;
        return true;
}

bool iso9660ReadVolumeDescriptors(void)
{
        uint8_t buffer[2048];
        uint32_t lba = ISO9660_FIRST_VOLUME_DESC;
        bool found_pvd = false;

        while (lba < 32)
        {
                if (cdRead((uint16_t)cd_port, cd_slave, lba, 1, (uint16_t*)buffer) != 0)
                {
                        printf("Failed to read LBA %d\n", lba);
                        return false;
                }

                iso9660VolumeDesc_t *desc = (iso9660VolumeDesc_t *)buffer;

                if (memcmp(desc->iden, "CD001", 5) != 0)
                {
                        printf("Invalid identifier at LBA %d: %s\n", lba, desc->iden);
                        lba++;
                        continue;
                }

                switch ((unsigned char)desc->type)
                {
                case ISO9660_BOOT_RECORD:
                        printf("Found Boot Record at LBA %d\n", lba);
                        break;

                case ISO9660_PRIMARY_DESC:
                        printf("Found Primary Volume Descriptor at LBA %d\n", lba);
                        if (!iso9660ParsePrimaryDescriptor(desc))
                        {
                                return false;
                        }
                        found_pvd = true;
                        break;

                case ISO9660_VOLUME_DESC_SET_TERMINATOR:
                        printf("Found Volume Descriptor Terminator at LBA %d\n", lba);
                        return found_pvd;

                default:
                        printf("Found unknown descriptor type %d at LBA %d\n", desc->type, lba);
                        break;
                }

                lba++;
        }

        printf("Reached end of search without finding terminator\n");
        return found_pvd;
}

bool iso9660ParsePrimaryDescriptor(const iso9660VolumeDesc_t *desc)
{
        if (desc->type != ISO9660_PRIMARY_DESC)
        {
                return false;
        }

        // Copy the primary descriptor
        memcpy(&primary_desc, desc, sizeof(iso9660PrimaryDesc_t));

        // Verify it's a valid primary descriptor
        if (memcmp(primary_desc.iden, "CD001", 5) != 0 || primary_desc.version != 0x1)
        {
                return false;
        }

        return true;
}

uint32_t iso9660GetRootLba(void)
{
        if (!iso9660_initialized)
        {
                return 0;
        }
        return primary_desc.root_directory_record.extend_lba[0];
}

uint32_t iso9660GetLogicalBlockSize(void)
{
        if (!iso9660_initialized)
        {
                return 0;
        }

        return primary_desc.logical_block_size[0];
}

bool iso9660ReadDirectory(uint32_t lba, uint16_t *buffer)
{
        if (!iso9660_initialized)
        {
                return false;
        }

        if (cdRead((uint16_t)cd_port, cd_slave, lba, 1, buffer) != 0)
        {
                return false;
        }

        return true;
}

void *iso9660ReadFile(iso9660Dir_t *file_entry)
{
        if (!iso9660_initialized || !file_entry)
        {
                return NULL;
        }

        uint32_t file_size = file_entry->data_length[0] + 128;
        uint32_t lba = file_entry->extend_lba[0];
        uint32_t sectors_to_read = (file_size + 2047) / 2048;

        void *file_data = malloc(file_size);
        if (!file_data)
        {
                return NULL;
        }

        uint16_t *buffer = (uint16_t *)malloc(2048);
        if (!buffer)
        {
                free(file_data);
                return NULL;
        }

        uint32_t bytes_read = 0;

        for (uint32_t i = 0; i < sectors_to_read; i++)
        {
                if (cdRead((uint16_t)cd_port, cd_slave, lba + i, 1, buffer) != 0)
                {
                        free(file_data);
                        free(buffer);
                        return NULL;
                }

                uint32_t bytes_to_copy = (file_size - bytes_read) > 2048 ? 2048 : (file_size - bytes_read);
                memcpy((uint8_t *)file_data + bytes_read, buffer, (int)bytes_to_copy);
                bytes_read += bytes_to_copy;
        }

        free(buffer);
        return file_data;
}

bool iso9660ListDirectory(uint32_t directory_lba)
{
        if (!iso9660_initialized)
        {
                return false;
        }

        uint16_t buffer[1024];
        uint8_t *dir_data = (uint8_t *)buffer;

        if (!iso9660ReadDirectory(directory_lba, buffer))
        {
                printf("Failed to read directory at LBA %d\n", directory_lba);
                return false;
        }

        uint32_t offset = 0;

        printf("Directory listing for LBA %d:\n", directory_lba);
        printf("----------------------------------------------------------------\n");
        printf("%-28s %-8s %-8s %s\n", "Name", "Size", "LBA", "Flags");
        printf("----------------------------------------------------------------\n");

        while (offset < 2048)
        {
                iso9660Dir_t *dir_entry = (iso9660Dir_t *)(dir_data + offset);

                if (dir_entry->length_of_dir == 0)
                {
                        break;
                }

                if (dir_entry->length_of_dir_ident == 1 &&
                    (dir_data[offset + sizeof(iso9660Dir_t)] == 0x00 ||
                     dir_data[offset + sizeof(iso9660Dir_t)] == 0x01))
                {
                        offset += dir_entry->length_of_dir;
                        continue;
                }

                char *name = (char *)(dir_data + offset + sizeof(iso9660Dir_t));
                uint8_t name_length = dir_entry->length_of_dir_ident;

                char clean_name[256];
                if (name_length > 0)
                {
                        memcpy(clean_name, name, name_length);
                        clean_name[name_length] = '\0';

                        char *semicolon = strchr(clean_name, ';');
                        if (semicolon)
                        {
                                *semicolon = '\0';
                        }

                        for (int i = strlen(clean_name) - 1; i >= 0 && clean_name[i] == ' '; i--)
                        {
                                clean_name[i] = '\0';
                        }
                }
                else
                {
                        strncpy(clean_name, "(no name)", 255);
                }

                uint32_t file_size = dir_entry->data_length[0];
                uint32_t lba = dir_entry->extend_lba[0];

                char type_char = (dir_entry->flags.raw & 0x02) ? 'D' : 'F';   // Directory or File
                char hidden_char = (dir_entry->flags.raw & 0x01) ? 'H' : ' '; // Hidden

                printf("%-28s %-8d %-8d %c%c\n",
                       clean_name, file_size, lba, type_char, hidden_char);

                offset += (int32_t)dir_entry->length_of_dir;
        }

        printf("----------------------------------------------------------------\n");
        return true;
}

bool iso9660ListRoot(void)
{
        uint32_t root_lba = iso9660GetRootLba();
        if (root_lba == 0)
        {
                printf("Failed to get root directory LBA\n");
                return false;
        }
        return iso9660ListDirectory(root_lba);
}

bool iso9660ListDir(const char *path)
{
        if (!iso9660_initialized)
        {
                printf("ISO9660 not initialized\n");
                return false;
        }

        uint32_t directory_lba;

        if (path == NULL || strcmp(path, "") == 0 || strcmp(path, "/") == 0)
        {
                directory_lba = iso9660GetRootLba();
        }
        else
        {
                if (!iso9660FindDirectory(path, &directory_lba))
                {
                        printf("Directory not found: %s\n", path);
                        return false;
                }
        }

        printf("Listing directory: %s\n", path);
        return iso9660ListDirectory(directory_lba);
}

bool iso9660TraversePathTable(const char *path)
{
        if (!iso9660_initialized)
                return false;

        uint32_t path_table_lba = primary_desc.type_l_path_table;
        uint32_t path_table_size = primary_desc.path_table_size[0];
        uint32_t sectors = (path_table_size + 2047) / 2048;
        uint8_t *buffer = malloc(sectors * 2048);

        if (!buffer || cdRead((uint16_t)cd_port, cd_slave, path_table_lba, sectors, (uint16_t *)buffer) != 0)
        {
                free(buffer);
                return false;
        }

        char *components[32];
        int count = 0;
        char *token = strtok((char *)path, "/");
        while (token && count < 32)
        {
                components[count++] = token;
                token = strtok(NULL, "/");
        }

        uint16_t current_index = 1;
        uint32_t current_lba = primary_desc.root_directory_record.extend_lba[0];

        for (int i = 0; i < count; i++)
        {
                bool found = false;
                uint32_t offset = 0;

                while (offset < path_table_size)
                {
                        iso9660PathTableRecord_t *rec = (iso9660PathTableRecord_t *)(buffer + offset);
                        if (offset + sizeof(*rec) + rec->name_len > path_table_size)
                                break;

                        if (rec->parent == current_index)
                        {
                                char name[256];
                                memcpy(name, (char *)(rec + 1), rec->name_len);
                                name[rec->name_len] = '\0';

                                if (strncmp(name, components[i], rec->name_len) == 0)
                                {
                                        current_index = (offset / sizeof(iso9660PathTableRecord_t)) + 1;
                                        current_lba = rec->lba;
                                        found = true;
                                        break;
                                }
                        }

                        offset += sizeof(*rec) + rec->name_len;
                        if (rec->name_len & 1)
                                offset++;
                }

                if (!found)
                {
                        free(buffer);
                        return false;
                }
        }

        free(buffer);
        return iso9660ListDirectory(current_lba);
}

bool iso9660FindFile(const char *path, iso9660Dir_t *file_entry)
{
        if (!iso9660_initialized || !file_entry)
                return false;

        char path_copy[256];
        strncpy(path_copy, path, sizeof(path_copy) - 1);
        path_copy[sizeof(path_copy) - 1] = '\0';

        char *last_slash = strrchr(path_copy, '/');
        if (!last_slash)
        {
                return iso9660FindInDirectory(iso9660GetRootLba(), path, file_entry);
        }

        *last_slash = '\0';
        char *filename = last_slash + 1;

        uint32_t directory_lba;
        if (!iso9660FindDirectory(path_copy, &directory_lba))
        {
                return false;
        }

        return iso9660FindInDirectory(directory_lba, filename, file_entry);
}

bool iso9660FindDirectory(const char *path, uint32_t *directory_lba)
{
        if (!iso9660_initialized || !directory_lba)
                return false;

        if (strncmp(path, "", 8) == 0 || strncmp(path, "/", 8) == 0)
        {
                *directory_lba = iso9660GetRootLba();
                return true;
        }

        uint32_t path_table_lba = primary_desc.type_l_path_table;
        uint32_t path_table_size = primary_desc.path_table_size[0];
        uint32_t sectors = (path_table_size + 2047) / 2048;
        uint8_t *buffer = malloc(sectors * 2048);

        if (!buffer || cdRead((uint16_t)cd_port, cd_slave, path_table_lba, sectors, (uint16_t *)buffer) != 0)
        {
                free(buffer);
                return false;
        }

        char *components[32];
        int count = 0;
        char path_copy[256];
        strncpy(path_copy, path, sizeof(path_copy) - 1);

        char *token = strtok(path_copy, "/");
        while (token && count < 32)
        {
                components[count++] = token;
                token = strtok(NULL, "/");
        }

        uint32_t current_index = 1;

        for (int i = 0; i < count; i++)
        {
                bool found = false;
                uint32_t offset = 0;

                while (offset < path_table_size)
                {
                        iso9660PathTableRecord_t *rec = (iso9660PathTableRecord_t *)(buffer + offset);
                        if (offset + sizeof(*rec) + rec->name_len > path_table_size)
                                break;

                        if (rec->parent == current_index)
                        {
                                char name[256];
                                memcpy(name, (char *)(rec + 1), rec->name_len);
                                name[rec->name_len] = '\0';

                                for (char *p = name; *p; p++)
                                        *p = (char)toupper((int)*p);
                                for (char *p = components[i]; *p; p++)
                                        *p = (char)toupper((int)*p);

                                if (strncmp(name, components[i], rec->name_len) == 0)
                                {
                                        current_index = (offset / sizeof(iso9660PathTableRecord_t)) + 1;
                                        *directory_lba = rec->lba;
                                        found = true;
                                        break;
                                }
                        }

                        offset += sizeof(*rec) + rec->name_len;
                        if (rec->name_len & 1)
                                offset++;
                }

                if (!found)
                {
                        free(buffer);
                        printf("Directory not found: %s\n", components[i]);
                        return false;
                }
        }

        free(buffer);
        return true;
}

bool iso9660FindInDirectory(uint32_t directory_lba, const char *filename, iso9660Dir_t *file_entry)
{
        uint16_t buffer[1024];
        uint8_t *dir_data = (uint8_t *)buffer;

        if (!iso9660ReadDirectory(directory_lba, buffer))
        {
                return false;
        }

        uint32_t offset = 0;
        char search_name[256];

        strncpy(search_name, filename, sizeof(search_name) - 1);
        for (char *p = search_name; *p; p++)
                *p = (char)toupper((int)*p);

        while (offset < 2048)
        {
                iso9660Dir_t *dir_entry = (iso9660Dir_t *)(dir_data + offset);

                if (dir_entry->length_of_dir == 0)
                {
                        break;
                }

                char *name = (char *)(dir_data + offset + sizeof(iso9660Dir_t));
                uint8_t name_length = dir_entry->length_of_dir_ident;

                char clean_name[256];
                memcpy(clean_name, name, name_length);
                clean_name[name_length] = '\0';

                char *semicolon = strchr(clean_name, ';');
                if (semicolon)
                        *semicolon = '\0';

                for (char *p = clean_name; *p; p++)
                        *p = (char)toupper((int)*p);

                if (strcmp(clean_name, search_name) == 0)
                {
                        memcpy(file_entry, dir_entry, sizeof(iso9660Dir_t));
                        return true;
                }

                offset += (uint32_t)dir_entry->length_of_dir;
        }

        return false;
}
