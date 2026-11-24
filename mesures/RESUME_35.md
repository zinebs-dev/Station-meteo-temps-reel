# Issue #35 - Mesures comparatives (sans FIFO, sans mlockall, sous charge)

## Protocole
- Programme : station, 3 taches temps reel (acquisition 10 Hz, filtrage 5 Hz, affichage 1 Hz).
- 4 configurations x 3 repetitions x 10 min = 12 runs, lances par `./scripts/campagne.sh 600 3` le 21/11/2026 de 17:47 a 19:49, dans l'ordre entrelace : normale, sans-fifo, sans-mlockall, charge, puis on recommence.
- normale : SCHED_FIFO + mlockall. sans-fifo : option --sans-fifo. sans-mlockall : option --sans-mlockall. charge : normale + `stress-ng --cpu 2` libre.
- Fichiers : `mesures/jitter_<config>_r<n>_<horodatage>.csv` (environ 9 600 mesures chacun). Machine : voir `CONFIG_VM_zineb.txt`.
- Mesure : retard du reveil par rapport a l'heure prevue. P50 = valeur typique, P99 = valeur depassee par 1 % des activations.


## Resultats de la campagne (tache acquisition, 10 Hz)
| Config | Run | P50 | P99 | Max |
|---|---|---|---|---|
| normale | r1 | 12,8 ms | 252 ms | 565 ms |
| normale | r2 | 0,89 ms | 93 ms | 274 ms |
| normale | r3 | 0,77 ms | 77 ms | 232 ms |
| sans-fifo | r1 | 5,5 ms | 262 ms | 834 ms |
| sans-fifo | r2 | 0,99 ms | 58 ms | 155 ms |
| sans-fifo | r3 | 1,0 ms | 58 ms | 326 ms |
| sans-mlockall | r1 | 19,7 ms | 522 ms | 1019 ms |
| sans-mlockall | r2 | 0,93 ms | 81 ms | 279 ms |
| sans-mlockall | r3 | 3,2 ms | 141 ms | 365 ms |
| charge | r1 | 3,6 ms | 12,4 s | 18,4 s |
| charge | r2 | 273 ms | 1,39 s | 2,59 s |
| charge | r3 | 526 ms | 1,53 s | 3,88 s |

Toutes les taches : `resultats_analyse_campagne.txt`.


## Lecture
1. Sans charge (normale, sans-fifo, sans-mlockall), aucune difference mesurable : l'ecart entre deux runs d'une meme configuration (normale : P99 de 77 a 252 ms) depasse l'ecart entre configurations. Explication probable, non verifiee : machine peu chargee, noyau lowlatency, tableau de mesure pre-rempli en memoire.
2. Hors charge, le run 1 est le plus mauvais de chaque configuration (P50 de 5,5 a 19,7 ms contre 0,8 a 3,2 ms ensuite). Les runs 1 ont eu lieu dans la premiere heure de la campagne ; effet de stabilisation possible, non demontre.
3. La configuration charge (stress-ng libre) ne permet pas de conclure sur SCHED_FIFO : P50 de 3,6 a 526 ms et P99 de 1,4 a 12,4 s selon le run. Pendant stress-ng --cpu 2, la VM n'utilisait que 44 a 49 % de CPU au lieu des 66 % attendus : l'hote ne lui donnait pas toute sa puissance (hypothese, VirtualBox ne renseigne pas le temps vole).

## Tests complementaires (runs de 120 s, sorties brutes dans `resultats_complementaires.txt`)
- Charge libre, 1 programme contre 2 (2 tours) : plus la charge est forte, plus les retards sont grands (P50 acquisition 85-136 ms avec 1 programme, 431-497 ms avec 2).
- Charge epinglee sur le meme coeur que station (`taskset -c 0`), FIFO (F) contre --sans-fifo (S).
  - 1 programme de charge : pas de difference (P99 acquisition : F 10,6 et 8,0 ms ; S 10,3 et 8,6 ms).
  - 4 programmes de charge, P99 acquisition :

| Tour | F (FIFO) | S (sans FIFO) |
|---|---|---|
| 1 | 12,4 ms | 18,4 ms |
| 2 | 24,8 ms | 38,3 ms |
| 3 (S puis F) | 22,8 ms | 141,5 ms |
| 4 | 8,8 ms | 17,5 ms |
| 5 (S puis F) | 37,0 ms | 61,4 ms |

  S est plus mauvais que F dans les 5 paires (aussi pour le filtrage), d'un facteur 1,5 a 6. Pas de difference sur le P50 (environ 2 a 4 ms). C'est un indice, pas une preuve : 5 paires de 2 minutes seulement.
- Fichiers journal et meteo en RAM contre disque : un seul essai, non repete, non concluant.

## Limites
- Une seule machine : VM VirtualBox (3 vCPU, 4,9 Go) sur un portable i5 de 4 coeurs et 8 threads avec 8 Go ; plan Windows Equilibre, sur batterie lors de la verification, alimentation pendant la campagne non confirmee.
- Noyau lowlatency : valeurs absolues non comparables avec celles des collegues (noyau generic).
- 3 repetitions par configuration ; tests complementaires courts.
- Remarque de conception, non demontree comme cause : la tache affichage fait des entrees-sorties directes (printf, fichier) dans une tache SCHED_FIFO, contrairement au journal ; piste pour une issue separee.
