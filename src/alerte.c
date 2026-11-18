/* src/alerte.c : tache sporadique d'alerte (repere 8.5 du guide) */
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>

#include "alerte.h"
#include "journal.h"

static pthread_mutex_t verrou;                               /* initialise dans initialiser() */
static pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
static pthread_once_t une_fois = PTHREAD_ONCE_INIT;
static int alerte_demandee = 0;                              /* le "drapeau" */
static double derniere_temperature = 0.0;

/* Cree le mutex avec heritage de priorite (une seule fois, quel que soit le thread appelant). */
static void initialiser(void)
{
    pthread_mutexattr_t ma;
    pthread_mutexattr_init(&ma);
    pthread_mutexattr_setprotocol(&ma, PTHREAD_PRIO_INHERIT);
    pthread_mutex_init(&verrou, &ma);
    pthread_mutexattr_destroy(&ma);
}
/* Appelee automatiquement si le thread est annule pendant qu'il tient le mutex. */
static void liberer(void *m)
{
    pthread_mutex_unlock((pthread_mutex_t *)m);
}



void *alerte_tache(void *arg) {
    (void)arg;
    pthread_once(&une_fois, initialiser);

    for (;;) {
        double t;

        pthread_mutex_lock(&verrou);
        pthread_cleanup_push(liberer, &verrou);
        while (!alerte_demandee)                       /* while, jamais if */
            pthread_cond_wait(&condition, &verrou);
        alerte_demandee = 0;
        t = derniere_temperature;
        pthread_cleanup_pop(1);                        /* 1 = executer liberer() : deverrouille */

        /* Action de l'alerte : hors du mutex, donc sans bloquer alerte_declencher(). */
        fprintf(stderr, "*** ALERTE : temperature %.1f C > %.1f C ***\n", t, ALERTE_SEUIL_C);
        journal_ecrire("ALERTE : temperature %.1f C depasse le seuil de %.1f C",
                       t, ALERTE_SEUIL_C);
    }
    return NULL;
}

void alerte_declencher(double temperature) {
    pthread_once(&une_fois, initialiser);

    pthread_mutex_lock(&verrou);
    derniere_temperature = temperature;
    alerte_demandee = 1;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&verrou);
}