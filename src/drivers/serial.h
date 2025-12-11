#ifndef SERIAL_H
#define SERIAL_H

#include<utils.h>
#include<io.h>

void SerialInit(void);
void SerialPut(char c);
void SerialPrint(const char *str);

#endif
