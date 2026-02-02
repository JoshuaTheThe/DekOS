#ifndef STRING_H
#define STRING_H

#include <utils.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <utils.h>

int memcmp(const void*,const void*,int);
int memcpy(void*,const void*,int);
int strncmp(const void*,const void*,int);
char *strtok(char *str, const char *delim);
char *strchr(const char *str, int c);
size_t strlen(const char *str);
char *strncpy(char *dest, const char *src, size_t n);
char *strrchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
char *strtok(char *str, const char *delim);
char *strchr(const char *s, int c);
int toupper(int c);
int tolower(int c);
void memset(void *d, uint8_t v, size_t len);
void memsetdw(void *d, uint32_t v, size_t len);
size_t strnlen(const char *x, size_t max);

/* Non Case Sensitive str-n compare */
int ncsstrncmp(char *restrict s1, char *restrict s2, size_t len);

#endif
