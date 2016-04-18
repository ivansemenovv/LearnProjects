// LZF.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "lzf.h"
#include <stdlib.h> 

#define width       1920
#define hight       1080
#define BUFFER_SIZE width * hight
int main()
{
    //unsigned int width = 1920;
    //unsigned int hight = 1080;
    char* in = new char[BUFFER_SIZE];

    unsigned int len = BUFFER_SIZE;
    while (len--)
    {
        in[len] = (char)((double)rand() / (RAND_MAX + 1) * 255);
    }

    //char in[] = "Test string for compression\0";
    char* compressed = new char[BUFFER_SIZE - 1];
    char* decompressed = new char[BUFFER_SIZE];
    ZeroMemory(compressed, BUFFER_SIZE);
    ZeroMemory(decompressed, BUFFER_SIZE);

    unsigned int compressedLen = lzf_compress(in, BUFFER_SIZE, compressed, BUFFER_SIZE - 1);

    if (compressedLen == 0) // if no compression happened then write uncompressed data
    {
        compressedLen = BUFFER_SIZE;
        compressed = in;
    }

    unsigned int decompressedLen = lzf_decompress(compressed, compressedLen, decompressed, BUFFER_SIZE);


    return 0;
}

