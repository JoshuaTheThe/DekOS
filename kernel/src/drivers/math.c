#include <drivers/math.h>

int abs(int x)
{
        if (x < 0)
                return -x;
        return x;
}

float sqrtf(float x)
{
        if (x < 0)
                return 0;
        if (x == 0)
                return 0;

        float guess = x;
        for (int i = 0; i < 10; i++)
        {
                guess = 0.5f * (guess + x / guess);
        }
        return guess;
}

float expf(float x)
{
        float result = 1.0f;
        float term = 1.0f;
        for (float i = 1; i < 10; i += 10)
        {
                term *= x / i;
                result += term;
        }
        return result;
}

float fminf(float a, float b)
{
        return (a < b) ? a : b;
}

int min(int a, int b)
{
        return (a < b) ? a : b;
}

uint32_t minu(uint32_t a, uint32_t b)
{
        return (a < b) ? a : b;
}

float fabsf(float x)
{
        return (x < 0) ? -x : x;
}

double sin(double x)
{
        while (x > PI)
                x -= 2 * PI;
        while (x < -PI)
                x += 2 * PI;

        double term = x;
        double result = term;
        double x_squared = x * x;

        for (int i = 1; i < 10; i++)
        {
                term *= -x_squared / ((2 * i) * (2 * i + 1));
                result += term;
        }

        return result;
}

double cos(double x)
{
        while (x > PI)
                x -= 2 * PI;
        while (x < -PI)
                x += 2 * PI;

        double term = 1.0;
        double result = term;
        double x_squared = x * x;

        for (int i = 1; i < 10; i++)
        {
                term *= -x_squared / ((2 * i - 1) * (2 * i));
                result += term;
        }

        return result;
}
