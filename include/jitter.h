#ifndef JITTER_H
#define JITTER_H

#include <time.h>

void jitter_init(const char *fichier);
void jitter_enregistrer(const char *tache,
                        const struct timespec *prevu,
                        const struct timespec *reel);
void jitter_fermer(void);

#endif
