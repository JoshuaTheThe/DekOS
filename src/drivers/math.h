#ifndef MATH_H
#define MATH_H

#include <stdint.h>
#include <stdbool.h>
#define M_PI 3.14159265358979323846
#define PI M_PI

int abs(int x);
float sqrtf(float x);
float expf(float x);
float fminf(float a, float b);
int min(int a, int b);
float fabsf(float x);
uint32_t minu(uint32_t a, uint32_t b);
double cos(double x);
double sin(double x);

#endif
