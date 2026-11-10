#ifndef ACQUISITION_H
#define ACQUISITION_H

/* Tache periodique 100 ms (10 Hz). arg : pointeur vers le buffer_t partage. */
void *acquisition_tache(void *arg);

#endif
