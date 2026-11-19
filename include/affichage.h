#ifndef AFFICHAGE_H
#define AFFICHAGE_H

void *affichage_tache(void *arg);

/* Active l'enregistrement meteo (CSV temps_s,temperature,humidite).
 * A appeler avant la creation de la tache. Si l'ouverture echoue, un message
 * est affiche et l'affichage continue sans CSV. */
void affichage_enregistrer_meteo(const char *fichier);

/* Ferme le fichier meteo. A appeler apres le pthread_join. */
void affichage_fermer_meteo(void);

#endif