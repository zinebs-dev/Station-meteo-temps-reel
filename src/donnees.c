/* src/donnees.c */
#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "donnees.h"
static pthread_mutex_t verrou;     /* protege derniere_mesure */
static mesure_t derniere_mesure;   /* derniere valeur filtree */
static int protocole_applique = -1; /* -1 tant que donnees_init() n'a pas ete appelee */

/* Arret immediat si un appel pthread echoue : sans mutex valide, le systeme
   n'a aucun sens (on prefere echouer au demarrage plutot que corrompre des donnees). */
static void verifier(int code, const char *appel)
{
    if (code != 0) {
        fprintf(stderr, "donnees : %s a echoue : %s\n", appel, strerror(code));
        exit(EXIT_FAILURE);
    }
}

void donnees_init(void) {
    
    pthread_mutexattr_t ma;
    int protocole = -1;

    memset(&derniere_mesure, 0, sizeof derniere_mesure);

    verifier(pthread_mutexattr_init(&ma), "pthread_mutexattr_init");
    verifier(pthread_mutexattr_setprotocol(&ma, PTHREAD_PRIO_INHERIT),
             "pthread_mutexattr_setprotocol(PTHREAD_PRIO_INHERIT)");

    /* On relit le protocole de l'attribut : c'est la preuve qu'il a bien ete pris en compte. */
    verifier(pthread_mutexattr_getprotocol(&ma, &protocole),
             "pthread_mutexattr_getprotocol");

    verifier(pthread_mutex_init(&verrou, &ma), "pthread_mutex_init");
    pthread_mutexattr_destroy(&ma);

    protocole_applique = protocole;
}

void donnees_ecrire(const mesure_t *m) {
    if (m == NULL)
        return;
    pthread_mutex_lock(&verrou);
    derniere_mesure = *m;
    pthread_mutex_unlock(&verrou);
}

void donnees_lire(mesure_t *m) {
   if (m == NULL)
        return;
    pthread_mutex_lock(&verrou);
    *m = derniere_mesure;
    pthread_mutex_unlock(&verrou);
}
int donnees_protocole(void)
{
    return protocole_applique;
}