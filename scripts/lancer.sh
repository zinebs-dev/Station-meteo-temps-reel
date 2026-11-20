#!/bin/bash
# Usage : ./scripts/lancer.sh [nom_de_la_config] [duree_en_secondes]
set -e
NOM=${1:-normale}
DUREE=${2:-1800}
make
mkdir -p mesures
HORO=$(date +%Y%m%d_%H%M)
sudo ./station --duree "$DUREE" \
     --sortie "mesures/jitter_${NOM}_${HORO}.csv" \
     --meteo  "mesures/meteo_${NOM}_${HORO}.csv"