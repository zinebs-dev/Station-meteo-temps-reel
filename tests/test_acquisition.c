#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include "acquisition.h"
#include "buffer.h"

static buffer_t buf;

int main(int argc, char **argv) {
    int duree = (argc > 1) ? atoi(argv[1]) : 60;

    pthread_t th;
    pthread_attr_t attr;
    struct sched_param sp = { .sched_priority = 60 };

    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setschedparam(&attr, &sp);

    buffer_init(&buf);

    if (pthread_create(&th, &attr, acquisition_tache, &buf) != 0) {
        fprintf(stderr, "SCHED_FIFO refuse (lancer avec sudo), priorite normale\n");
        pthread_create(&th, NULL, acquisition_tache, &buf);
    }
    pthread_attr_destroy(&attr);

    sleep(duree);
    mesure_t m; int n = 0;
    while (buffer_pop(&buf, &m) == 0) n++;
    printf("elements dans le buffer : %d (attendu : min(10 x duree, 63))\n", n);
    return 0;
}
