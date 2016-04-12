// BitArrayPerformanceTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

long long milliseconds_now() {
    static LARGE_INTEGER s_frequency;
    static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
    if (s_use_qpc) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / s_frequency.QuadPart;
    }
    else {
        return GetTickCount();
    }
}


int main()
{
    int number_iterations = 1000;
    unsigned int width = 1920;
    unsigned int hight = 1080;
    char* image = new char[width * hight];
    //ZeroMemory(image, width * hight);

    char* imageDest = new char[width * hight];
    //ZeroMemory(imageDest, width * hight);

    //image[0] = 0x55;
    
    __int64 *image64 = (__int64*)image;
    __int64 *imageDesc64 = (__int64*)imageDest;

    long long start = 0;
    long long end = 0;

    start = milliseconds_now();
    for (size_t i = 0; i < number_iterations; i++)
    {
        int lenght64 = (width * hight) / sizeof(__int64);
        while (lenght64--)
        {
            imageDesc64[lenght64] ^= image64[lenght64];
        }
    }
    end = milliseconds_now();
    std::cout << "Pointer 64---:" << end - start << "ms." << std::endl;

    //image[1] = 0xAA;

    start = milliseconds_now();
    for (size_t i = 0; i < number_iterations; i++)
    {
        int lenght = (width * hight);
        while (lenght--)
        {
            imageDest[lenght] ^= image[lenght];
        }
    }
    end = milliseconds_now();
    std::cout << "Pointer 8:" << end - start << "ms." << std::endl;

    return 0;
}

