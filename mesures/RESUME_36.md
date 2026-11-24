# Issue #36 - Latences et echeances ratees

## Methode
- Commande : `python3 scripts/analyse.py --synthese mesures/jitter_*.csv`. Seuls les fichiers `jitter_<config>_r<n>_<date>_<heure>.csv` sont lus ; les autres (ex. les 3 runs de 30 min de #34) sont ignores.
- Percentile : rang le plus proche (`round(p/100 x (n-1))` sur les valeurs triees), comme `scripts/figures.py` et `scripts/comparer_repetitions.py`. D'autres conventions donnent des valeurs legerement differentes.
- Echeance de chaque tache = sa periode, lue dans `src/*.c` : acquisition 100 ms, filtrage 200 ms, affichage 1 s. Seuil strict = periode / 10 (10, 20, 100 ms).
- Une activation rate son echeance si son retard de reveil depasse strictement l'echeance. On mesure le reveil et pas la fin du travail ; c'est acceptable car le travail de chaque tache dure quelques microsecondes.
- Les taches visent des heures absolues : apres un blocage, les activations de rattrapage sont elles aussi en retard. Un incident produit donc plusieurs echeances ratees. Les taux comptent des activations, pas des incidents.

## Fichiers
- `synthese_configurations.csv` et `.md` : 4 configurations x 3 taches (les 3 runs de chaque configuration regroupes), sans case vide.
- `synthese_par_run.csv` : 36 lignes, une par run et par tache.
- Colonnes (ms) : n, p50_ms, p99_ms, p999_ms, max_ms, echeance_ms, ratees, ratees_pct, seuil_strict_ms, ratees_strict, ratees_strict_pct.

## Controle
- `scripts/test_analyse.sh` : echantillon de 30 mesures (`tests/echantillon/`) dont les resultats sont calcules a la main (`calcul_manuel.txt`, `attendu.csv`) ; affiche OK.
- Controle independant sur les donnees reelles, avec awk, sort, sed et wc (sans Python), de la ligne normale / acquisition : n = 18007, P50 = 1727 us, P99 = 171815 us, P99.9 = 369432 us, max = 565055 us, 485 echeances ratees et 4670 au seuil strict, identiques au tableau.

## Lecture (tache acquisition, 10 Hz, echeance 100 ms)
| Configuration | Activations | Ratees | Seuil strict (10 ms) |
|---|---|---|---|
| normale | 18007 | 485 (2,69 %) | 25,93 % |
| sans-fifo | 18005 | 360 (2,00 %) | 17,95 % |
| sans-mlockall | 18002 | 1041 (5,78 %) | 34,69 % |
| charge | 18017 | 11439 (63,49 %) | 70,05 % |

- Sans charge, les taux d'echeances ratees restent faibles : au plus 5,78 % (acquisition), 0,60 % (filtrage), 0 (affichage). Sous charge (stress-ng libre), 63 % et 56 % des activations de l'acquisition et du filtrage ratent leur echeance.
- Le run 1 pese lourd : en configuration normale il concentre 406 des 485 ratees (84 %) et 3374 des 4670 ratees au seuil strict (72 %). Hors run 1 (runs 2 et 3, environ 12000 activations), les taux sont de 0,66 % (normale), 0,14 % (sans-fifo) et 1,34 % (sans-mlockall), et de 10,8 %, 9,3 % et 19,3 % au seuil strict.
- Ces ecarts entre configurations ne sont pas stables d'un run a l'autre (au seuil strict, run 2 : normale 12,95 % contre sans-mlockall 14,42 % ; run 3 : 8,65 % contre 24,24 %). Avec 3 runs par configuration, on ne conclut pas.

## Limites
- Memes limites que #35 : une seule machine (VM VirtualBox, 3 vCPU), noyau lowlatency, 3 runs par configuration. La configuration charge n'est pas interpretable (voir `RESUME_35.md`).
- L'echeance suppose que le travail de chaque tache est negligeable devant sa periode ; la fin d'execution n'est pas mesuree.
- Les pourcentages comptent des activations en retard, y compris celles de rattrapage apres un blocage.
