#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "affichage.h"
#include "donnees.h"
#include "mesure.h"

int main(void) {
    donnees_init();
    mesure_t m = { .temperature = 21.5, .humidite = 48.0, .pression = 1013.2 };
    clock_gettime(CLOCK_REALTIME, &m.horodatage);
    donnees_ecrire(&m);

    pthread_t th;
    pthread_create(&th, NULL, affichage_tache, NULL);
    sleep(61);
    return 0;
}
