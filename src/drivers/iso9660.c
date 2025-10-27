#include <drivers/iso9660.h>

/* strA -- String with only ASCII a-characters, padded to the right with spaces. */
/* strD -- String with only ASCII d-characters, padded to the right with spaces. */

bool is_a_character(char x)
{
        return is_d_character(x) || x == '!' || x == '"' || (x >= '%' || x <= '/') || (x >= ':' || x <= '?');
}

bool is_d_character(char x)
{
        return (x >= 'A' && x <= 'Z') || x == '_';
}

// Global variables for CD-ROM information
int cd_port = 0x1F0;
bool cd_slave = false;
iso9660_primary_desc primary_desc;
bool iso9660_initialized = false;

// Implementation
bool iso9660_init(void)
{
        // Initialize CD-ROM drive
        cd_drive_init(&cd_port, &cd_slave);

        // Read volume descriptors
        if (!iso9660_read_volume_descriptors())
        {
                return false;
        }

        iso9660_initialized = true;
        return true;
}

bool iso9660_read_volume_descriptors(void)
{
        uint8_t buffer[2048]; // Use byte buffer, not uint16_t
        uint32_t lba = ISO9660_FIRST_VOLUME_DESC;
        bool found_pvd = false;

        while (lba < 32) // Reasonable limit
        {
                // Read one sector
                if (read_cdrom(cd_port, cd_slave, lba, 1, (uint16_t *)buffer) != 0)
                {
                        printf("Failed to read LBA %d\n", lba);
                        return false;
                }

                iso9660_volume_desc *desc = (iso9660_volume_desc *)buffer;

                // Check identifier first
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
                        if (!iso9660_parse_primary_desc(desc))
                        {
                                return false;
                        }
                        found_pvd = true;
                        break;

                case ISO9660_VOLUME_DESC_SET_TERMINATOR:
                        printf("Found Volume Descriptor Terminator at LBA %d\n", lba);
                        return found_pvd; // Return true if we found PVD

                default:
                        printf("Found unknown descriptor type %d at LBA %d\n", desc->type, lba);
                        break;
                }

                lba++;
        }

        printf("Reached end of search without finding terminator\n");
        return found_pvd;
}

bool iso9660_parse_primary_desc(const iso9660_volume_desc *desc)
{
        if (desc->type != ISO9660_PRIMARY_DESC)
        {
                return false;
        }

        // Copy the primary descriptor
        memcpy(&primary_desc, desc, sizeof(iso9660_primary_desc));

        // Verify it's a valid primary descriptor
        if (memcmp(primary_desc.iden, "CD001", 5) != 0 || primary_desc.version != 0x1)
        {
                return false;
        }

        return true;
}

uint32_t iso9660_get_root_directory_lba(void)
{
        if (!iso9660_initialized)
        {
                return 0;
        }
        return primary_desc.root_directory_record.extend_lba[0];
}

uint32_t iso9660_get_logical_block_size(void)
{
        if (!iso9660_initialized)
        {
                return 0;
        }

        return primary_desc.logical_block_size[0];
}

bool iso9660_read_directory(uint32_t lba, uint16_t *buffer)
{
        if (!iso9660_initialized)
        {
                return false;
        }

        // Read the directory sector
        if (read_cdrom(cd_port, cd_slave, lba, 1, buffer) != 0)
        {
                return false;
        }

        return true;
}

void *iso9660_read_file(iso9660_directory *file_entry)
{
        if (!iso9660_initialized || !file_entry)
        {
                return NULL;
        }

        uint32_t file_size = file_entry->data_length[0] + 128;
        uint32_t lba = file_entry->extend_lba[0];
        uint32_t sectors_to_read = (file_size + 2047) / 2048; // Round up to nearest sector

        // Allocate memory for the file
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
                if (read_cdrom(cd_port, cd_slave, lba + i, 1, buffer) != 0)
                {
                        free(file_data);
                        free(buffer);
                        return NULL;
                }

                uint32_t bytes_to_copy = (file_size - bytes_read) > 2048 ? 2048 : (file_size - bytes_read);
                memcpy((uint8_t *)file_data + bytes_read, buffer, bytes_to_copy);
                bytes_read += bytes_to_copy;
        }

        free(buffer);
        return file_data;
}

// Function to list directory contents
// Function to list directory contents
bool iso9660_list_directory(uint32_t directory_lba)
{
        if (!iso9660_initialized)
        {
                return false;
        }

        uint16_t buffer[1024]; // 2048 bytes buffer
        uint8_t *dir_data = (uint8_t *)buffer;

        if (!iso9660_read_directory(directory_lba, buffer))
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
                iso9660_directory *dir_entry = (iso9660_directory *)(dir_data + offset);

                // Check for end of directory or invalid entry
                if (dir_entry->length_of_dir == 0)
                {
                        break;
                }

                // Skip current directory (.) and parent directory (..) entries
                if (dir_entry->length_of_dir_ident == 1 &&
                    (dir_data[offset + sizeof(iso9660_directory)] == 0x00 ||
                     dir_data[offset + sizeof(iso9660_directory)] == 0x01))
                {
                        offset += dir_entry->length_of_dir;
                        continue;
                }

                // Extract and clean filename
                char *name = (char *)(dir_data + offset + sizeof(iso9660_directory));
                uint8_t name_length = dir_entry->length_of_dir_ident;

                char clean_name[256];
                if (name_length > 0)
                {
                        memcpy(clean_name, name, name_length);
                        clean_name[name_length] = '\0';

                        // Remove version suffix (everything after semicolon)
                        char *semicolon = strchr(clean_name, ';');
                        if (semicolon)
                        {
                                *semicolon = '\0';
                        }

                        // Remove trailing spaces
                        for (int i = strlen(clean_name) - 1; i >= 0 && clean_name[i] == ' '; i--)
                        {
                                clean_name[i] = '\0';
                        }
                }
                else
                {
                        strncpy(clean_name, "(no name)", 255);
                }

                // Calculate file size
                uint32_t file_size = dir_entry->data_length[0];

                // Calculate LBA
                uint32_t lba = dir_entry->extend_lba[0];

                // Determine file type and flags
                char type_char = (dir_entry->flags.raw & 0x02) ? 'D' : 'F';   // Directory or File
                char hidden_char = (dir_entry->flags.raw & 0x01) ? 'H' : ' '; // Hidden

                // Format and print the entry
                printf("%-28s %-8d %-8d %c%c\n",
                        clean_name, file_size, lba, type_char, hidden_char);

                // Move to next entry
                offset += dir_entry->length_of_dir;
        }

        printf("----------------------------------------------------------------\n");
        return true;
}

// Convenience function to list root directory
bool iso9660_list_root(void)
{
        uint32_t root_lba = iso9660_get_root_directory_lba();
        if (root_lba == 0)
        {
                printf("Failed to get root directory LBA\n");
                return false;
        }
        return iso9660_list_directory(root_lba);
}

// Enhanced version that can list any directory by path
bool iso9660_list_dir(const char *path)
{
        if (!iso9660_initialized)
        {
                printf("ISO9660 not initialized\n");
                return false;
        }

        uint32_t directory_lba;

        if (path == NULL || strcmp(path, "") == 0 || strcmp(path, "/") == 0)
        {
                // List root directory
                directory_lba = iso9660_get_root_directory_lba();
        }
        else
        {
                // Find the directory by path
                if (!iso9660_find_directory(path, &directory_lba))
                {
                        printf("Directory not found: %s\n", path);
                        return false;
                }
        }

        printf("Listing directory: %s\n", path);
        return iso9660_list_directory(directory_lba);
}
bool iso9660_traverse_path_table(const char *path)
{
        if (!iso9660_initialized)
                return false;

        // Read path table
        uint32_t path_table_lba = primary_desc.type_l_path_table;
        uint32_t path_table_size = primary_desc.path_table_size[0];
        uint32_t sectors = (path_table_size + 2047) / 2048;
        uint8_t *buffer = malloc(sectors * 2048);

        if (!buffer || read_cdrom(cd_port, cd_slave, path_table_lba, sectors, (uint16_t *)buffer) != 0)
        {
                free(buffer);
                return false;
        }

        // Split path into components
        char *components[32];
        int count = 0;
        char *token = strtok((char *)path, "/");
        while (token && count < 32)
        {
                components[count++] = token;
                token = strtok(NULL, "/");
        }

        // Start from root directory (index 1)
        uint16_t current_index = 1;
        uint32_t current_lba = primary_desc.root_directory_record.extend_lba[0];

        for (int i = 0; i < count; i++)
        {
                bool found = false;
                uint32_t offset = 0;

                while (offset < path_table_size)
                {
                        iso9660_path_table_record *rec = (iso9660_path_table_record *)(buffer + offset);
                        if (offset + sizeof(*rec) + rec->name_len > path_table_size)
                                break;

                        // Check if this record belongs to current directory and matches name
                        if (rec->parent == current_index)
                        {
                                char name[256];
                                memcpy(name, (char *)(rec + 1), rec->name_len);
                                name[rec->name_len] = '\0';

                                if (strncmp(name, components[i], rec->name_len) == 0)
                                {
                                        current_index = (offset / sizeof(iso9660_path_table_record)) + 1;
                                        current_lba = rec->lba;
                                        found = true;
                                        break;
                                }
                        }

                        // Move to next record
                        offset += sizeof(*rec) + rec->name_len;
                        if (rec->name_len & 1)
                                offset++; // Padding byte
                }

                if (!found)
                {
                        free(buffer);
                        return false;
                }
        }

        free(buffer);
        return iso9660_list_directory(current_lba);
}

bool iso9660_find_file(const char *path, iso9660_directory *file_entry)
{
        if (!iso9660_initialized || !file_entry)
                return false;

        char path_copy[256];
        strncpy(path_copy, path, sizeof(path_copy) - 1);
        path_copy[sizeof(path_copy) - 1] = '\0';

        char *last_slash = strrchr(path_copy, '/');
        if (!last_slash)
        {
                return iso9660_find_in_directory(iso9660_get_root_directory_lba(), path, file_entry);
        }

        *last_slash = '\0';
        char *filename = last_slash + 1;

        uint32_t directory_lba;
        if (!iso9660_find_directory(path_copy, &directory_lba))
        {
                return false;
        }

        return iso9660_find_in_directory(directory_lba, filename, file_entry);
}

bool iso9660_find_directory(const char *path, uint32_t *directory_lba)
{
        if (!iso9660_initialized || !directory_lba)
                return false;

        if (strncmp(path, "", 8) == 0 || strncmp(path, "/", 8) == 0)
        {
                *directory_lba = iso9660_get_root_directory_lba();
                return true;
        }

        uint32_t path_table_lba = primary_desc.type_l_path_table;
        uint32_t path_table_size = primary_desc.path_table_size[0];
        uint32_t sectors = (path_table_size + 2047) / 2048;
        uint8_t *buffer = malloc(sectors * 2048);

        if (!buffer || read_cdrom(cd_port, cd_slave, path_table_lba, sectors, (uint16_t *)buffer) != 0)
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

        uint16_t current_index = 1;

        for (int i = 0; i < count; i++)
        {
                bool found = false;
                uint32_t offset = 0;

                while (offset < path_table_size)
                {
                        iso9660_path_table_record *rec = (iso9660_path_table_record *)(buffer + offset);
                        if (offset + sizeof(*rec) + rec->name_len > path_table_size)
                                break;

                        // Check if this record belongs to current directory and matches name
                        if (rec->parent == current_index)
                        {
                                char name[256];
                                memcpy(name, (char *)(rec + 1), rec->name_len);
                                name[rec->name_len] = '\0';

                                // Convert to uppercase for comparison (ISO9660 names are uppercase)
                                for (char *p = name; *p; p++)
                                        *p = toupper(*p);
                                for (char *p = components[i]; *p; p++)
                                        *p = toupper(*p);

                                if (strncmp(name, components[i], rec->name_len) == 0)
                                {
                                        current_index = (offset / sizeof(iso9660_path_table_record)) + 1;
                                        *directory_lba = rec->lba;
                                        found = true;
                                        break;
                                }
                        }

                        // Move to next record
                        offset += sizeof(*rec) + rec->name_len;
                        if (rec->name_len & 1)
                                offset++; // Padding byte
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

bool iso9660_find_in_directory(uint32_t directory_lba, const char *filename, iso9660_directory *file_entry)
{
        uint16_t buffer[1024];
        uint8_t *dir_data = (uint8_t *)buffer;

        if (!iso9660_read_directory(directory_lba, buffer))
        {
                return false;
        }

        uint32_t offset = 0;
        char search_name[256];

        strncpy(search_name, filename, sizeof(search_name) - 1);
        for (char *p = search_name; *p; p++)
                *p = toupper(*p);

        while (offset < 2048)
        {
                iso9660_directory *dir_entry = (iso9660_directory *)(dir_data + offset);

                if (dir_entry->length_of_dir == 0)
                {
                        break;
                }

                char *name = (char *)(dir_data + offset + sizeof(iso9660_directory));
                uint8_t name_length = dir_entry->length_of_dir_ident;

                char clean_name[256];
                memcpy(clean_name, name, name_length);
                clean_name[name_length] = '\0';

                char *semicolon = strchr(clean_name, ';');
                if (semicolon)
                        *semicolon = '\0';

                for (char *p = clean_name; *p; p++)
                        *p = toupper(*p);

                if (strcmp(clean_name, search_name) == 0)
                {
                        memcpy(file_entry, dir_entry, sizeof(iso9660_directory));
                        return true;
                }

                offset += dir_entry->length_of_dir;
        }

        return false;
}
