#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "libattopng.c"
#include "libattopng.h"

#define RGBA(r, g, b, a) ((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))
#define RGB(r, g, b) ((r) | ((g) << 8) | ((b) << 16))
#define gotoxy(x, y) printf("\033[%d;%dH", (y), (x))
#define clear() printf("\033[H\033[J")

struct encrypt_infos {
    libattopng_t* png;
    unsigned char* buffer;
    int img_size;
    long offset_start;
    long offset_end;
    int thread_id;
};

void* encrypt_zone(void* ptr) {
    struct encrypt_infos* args = (struct encrypt_infos*)ptr;

    unsigned int j = 0, x = 0, y = 0;
    unsigned char r, g, b;

    libattopng_t* png = args->png;

    printf("Starting job.\n");
    for (unsigned int i = args->offset_start; i < args->offset_end; i += 3) {
        j = i / 3.;
        x = j % args->img_size;
        y = j / args->img_size;

        r = args->buffer[i];
        g = i + 1 < args->offset_end ? args->buffer[i + 1] : 0;
        b = i + 2 < args->offset_end ? args->buffer[i + 2] : 0;

        libattopng_set_pixel(png, x, y, RGB(r, g, b));

        // int per = i / (args->offset_end - (double)args->offset_start) * 100;
    }

    printf("Job done.\n");
}

int encrypt(unsigned char* buffer, long len) {
    printf("Encrypting file...\n");

    int img_size = ceil(sqrt(ceil(len / 3.)));
    libattopng_t* png = libattopng_new(img_size, img_size, PNG_RGB);

    //  prepare arguments for thead
    struct encrypt_infos args1, args2;

    args2.png = args1.png = png;
    args2.buffer = args1.buffer = buffer;
    args2.img_size = args1.img_size = img_size;
    args1.offset_start = 0;
    args1.offset_end = len / 2;
    args2.thread_id = 0;

    args2.offset_start = len / 2;
    args2.offset_end = len;
    args2.thread_id = 1;

    clear();

    // start thread
    pthread_t thread1, thread2;
    if (pthread_create(&thread1, NULL, encrypt_zone, (void*)&args1)) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread2, NULL, encrypt_zone, (void*)&args2)) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }

    // for (unsigned int i = 0; i < len; i += 3) {
    // }

    if (pthread_join(thread1, NULL) && pthread_join(thread2, NULL)) {
        perror("pthread_join");
        return EXIT_FAILURE;
    }

    printf("Writing PNG to disk...\n");
    libattopng_save(png, "output.png");
    libattopng_destroy(png);

    return EXIT_SUCCESS;
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
            printf("Loading file %s...\n", argv[2]);

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