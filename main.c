#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libattopng.c"
#include "libattopng.h"

#define RGBA(r, g, b, a) ((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))
#define RGB(r, g, b) ((r) | ((g) << 8) | ((b) << 16))
#define gotoxy(x, y) printf("\033[%d;%dH", (y), (x))
#define clear() printf("\033[H\033[J")

void encrypt(unsigned char* buffer, long len) {
    int img_size = ceil(sqrt(ceil(len / 3.)));
    libattopng_t* png = libattopng_new(img_size, img_size, PNG_RGB);

    unsigned int j = 0, x = 0, y = 0;
    unsigned char r, g, b;
    for (unsigned int i = 0; i < len; i += 3) {
        j = i / 3.;
        x = j % img_size;
        y = j / img_size;

        r = buffer[i];
        g = i + 1 < len ? buffer[i + 1] : 0;
        b = i + 2 < len ? buffer[i + 2] : 0;

        libattopng_set_pixel(png, x, y, RGB(r, g, b));
        // clear();
        // gotoxy(0, 0);
        // printf("Progress: %.2lf%%\n", i / (double)len * 100);
    }

    libattopng_save(png, "output.png");
    libattopng_destroy(png);
}

int main(int argc, char* argv[]) {
    // verify arguments
    if (argc <= 1) {
        printf("Usage: imcrypt <encrypt|decrypt|help> <path/to/file>\n");
        return 0;
    }

    if (!strcmp(argv[1], "encrypt")) {
        if (argc <= 2) {
            printf("Please specify the file to encrypt.\n");
            return 0;
        }
    } else if (!strcmp(argv[1], "decrypt")) {
        if (argc <= 2) {
            printf("Please specify the file to decrypt.\n");
            return 0;
        }
    } else {
        printf("Usage: imcrypt <encrypt|decrypt|help> <path/to/file>\n");
        return 0;
    }

    // verify that file exists
    if (access(argv[2], F_OK) != -1) {
        if (!strcmp(argv[1], "encrypt")) {
            printf("Encrypting file %s...\n", argv[2]);

            FILE* fileptr;
            unsigned char* buffer;
            long filelen;

            // open file in binary mode
            fileptr = fopen(argv[2], "rb");
            // jump to end of file
            fseek(fileptr, 0, SEEK_END);
            // get offset (used for length)
            filelen = ftell(fileptr);
            // go back to start of file
            rewind(fileptr);

            // allocate memory for the buffer
            buffer = (char*)malloc(filelen * sizeof(char));
            // write file data to buffer
            fread(buffer, 1, filelen, fileptr);
            // close file
            fclose(fileptr);

            // convert file to png
            encrypt(buffer, filelen);
        }
    } else {
        printf("The file given donesn't exists.");
        return 0;
    }

    return 0;
}