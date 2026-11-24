# Première analyse des résultats (issue #38)

**Conditions.** Campagne comparative entrelacée sur la VM de Zineb (3 vCPU, noyau lowlatency) : 4 configurations × 3 runs de 10 min. Échéance d'une tâche = sa période (acquisition 100 ms, filtrage 200 ms, affichage 1 s).

**Échéances (acquisition).** En configuration normale (SCHED_FIFO + mlockall), le P50 du retard est de 0,77 à 12,8 ms selon le run, mais le P99 atteint 252 ms au run 1, soit plus de deux fois la période. Les échéances ne sont donc pas toutes respectées, et l'écart entre runs (P99 de 77 à 252 ms) dépasse l'écart entre configurations.

**SCHED_FIFO et mlockall.** Sans SCHED_FIFO, les P99 sont de 262 / 58 / 58 ms ; sans mlockall, de 522 / 81 / 141 ms. Aucune de ces valeurs ne se distingue clairement de la configuration normale (252 / 93 / 77 ms). Sans charge, la politique d'ordonnancement et le verrouillage mémoire ne montrent pas d'apport mesurable sur cette VM.

**Charge.** Sous stress-ng --cpu 2 libre, les P99 de l'acquisition vont de 1,39 s à 12,4 s selon le run, avec un P50 de 3,6 à 526 ms. Un retard médian de plusieurs centaines de ms est incompatible avec un ordonnanceur qui préempte une tâche SCHED_FIFO : la cause est probablement hors de la VM invitée (hôte/VirtualBox). La configuration « charge » n'est pas interprétable pour juger SCHED_FIFO.

**Part de la VM.** Le run 1 est le plus mauvais des trois configurations sans charge (P50 acquisition : 12,8 / 5,5 / 19,7 ms contre 0,8 à 1,0 ms aux runs 2 et 3), ce qui désigne l'environnement plutôt que la configuration. Les tests épinglés (`taskset -c 0`, 4 programmes de charge) montrent un P99 acquisition plus élevé sans FIFO que avec FIFO dans les 5 paires (facteur 1,5 à 6 sur le P99, pas sur le P50) : c'est un indice, pas une preuve (5 paires de 2 min).

**Stabilité.** Les trois runs d'une même configuration ne donnent pas les mêmes P99 (normale : 252 / 93 / 77 ms ; ×3,3). La variabilité entre runs est le fait dominant.

**Limites.** Mesures en VM (valeurs absolues plus élevées et moins régulières qu'en physique), 3 runs par configuration, une seule machine pour la campagne. La configuration normale tient ses échéances pour l'affichage et presque pour le filtrage, pas pour l'acquisition. Nos mesures ne permettent pas d'attribuer d'effet stable à SCHED_FIFO ni à mlockall sans charge.