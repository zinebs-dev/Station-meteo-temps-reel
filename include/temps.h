#ifndef TEMPS_H
#define TEMPS_H

#include <stdint.h>
#include <time.h>

/* Ajoute ns nanosecondes à t et normalise les secondes/nanosecondes. */
void ajouter_ns(struct timespec *t, long ns);

/* Retourne b - a en microsecondes. Le résultat peut être négatif. */
int64_t diff_us(const struct timespec *a, const struct timespec *b);

#endif