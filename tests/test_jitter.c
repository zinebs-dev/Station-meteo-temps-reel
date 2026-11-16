#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "acquisition.h"
#include "affichage.h"
#include "buffer.h"
#include "donnees.h"
#include "filtrage.h"
#include "jitter.h"

static buffer_t tampon;

int main(int argc, char **argv) {
    int duree = (argc > 1) ? atoi(argv[1]) : 60;

    donnees_init();
    buffer_init(&tampon);
    jitter_init("/tmp/jitter_test.csv");

    pthread_t t[3];
    pthread_create(&t[0], NULL, acquisition_tache, &tampon);
    pthread_create(&t[1], NULL, filtrage_tache, NULL);
    pthread_create(&t[2], NULL, affichage_tache, NULL);

    sleep((unsigned)duree);

    for (int i = 0; i < 3; i++) pthread_cancel(t[i]);
    for (int i = 0; i < 3; i++) pthread_join(t[i], NULL);

    jitter_fermer();
    return 0;
}