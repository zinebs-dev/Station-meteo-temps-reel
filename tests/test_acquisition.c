#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include "acquisition.h"

int main(int argc, char **argv) {
    int duree = (argc > 1) ? atoi(argv[1]) : 60;

    pthread_t th;
    pthread_attr_t attr;
    struct sched_param sp = { .sched_priority = 60 };

    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    pthread_attr_setschedparam(&attr, &sp);

    if (pthread_create(&th, &attr, acquisition_tache, NULL) != 0) {
        fprintf(stderr, "SCHED_FIFO refuse (lancer avec sudo), priorite normale\n");
        pthread_create(&th, NULL, acquisition_tache, NULL);
    }
    pthread_attr_destroy(&attr);

    sleep(duree);
    return 0;   /* la fin du processus arrete le thread */
}
