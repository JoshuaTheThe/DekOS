#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#define alignment 16
#define align(size) (((size) + (alignment - 1)) & ~(alignment - 1))

extern int memcmp(const void*,const void*,int);
extern int memcpy(void*,const void*,int);
extern int strncmp(const void*,const void*,int);
extern void *malloc(int);
extern void free(void*);
extern void memory_init(uint32_t);
extern char *strtok(char *str, const char *delim);
extern char *strchr(const char *str, int c);

size_t strlen(const char *str);
char *strncpy(char *dest, const char *src, size_t n);
char *strrchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
char *strtok(char *str, const char *delim);
char *strchr(const char *s, int c);
int toupper(int c);
int tolower(int c);
void memset(void *d, uint8_t v, uint32_t len);

typedef struct
{
        uint8_t *raw;
        uint8_t *map;
        uint32_t heap_size;
        uint32_t mem_size;
} memory_information_t;

typedef struct
{
        void *ptr;
        int size;
} region_t;

// Heap boundaries (set by linker script)
extern uint8_t _heap_start[];
extern uint8_t _heap_end[];
extern uint8_t _heap_map_start[];
extern uint8_t _heap_map_end[];
extern region_t _allocations[];
extern uint8_t _allocations_end[];

#endif
