#!/bin/bash
# Usage : ./scripts/lancer.sh nom_de_la_config
set -e
NOM=${1:-normale}
make
mkdir -p mesures
sudo ./station --duree 1800 --sortie mesures/jitter_${NOM}_$(date +%Y%m%d_%H%M).csv
