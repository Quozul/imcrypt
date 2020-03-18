#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct thread_args {
    char a;
    int b;
    int *c;
};

void *thread_1(void *arg) {
    struct thread_args *args = (struct thread_args *)arg;
    printf("Nous sommes dans le thread. '%c' '%d'\n", args->a, args->b);
    int *ptr = args->c;
    printf("Adresse de a: %p\n", ptr);

    *ptr = 6;

    /* Pour enlever le warning */
    pthread_exit(NULL);
}

int main(void) {
    pthread_t thread1;

    printf("Avant la creation du thread.\n");

    int a = 5;
    int *ptr = &a;

    struct thread_args args;
    args.a = 'a';
    args.b = 5;
    args.c = ptr;

    printf("%d\n", a);

    if (pthread_create(&thread1, NULL, thread_1, (void *)&args)) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }

    if (pthread_join(thread1, NULL)) {
        perror("pthread_join");
        return EXIT_FAILURE;
    }

    printf("Apres la creation du thread.\n");
    printf("%d\n", a);

    return EXIT_SUCCESS;
}
