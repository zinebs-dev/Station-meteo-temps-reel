#!/bin/bash
# Usage : ./scripts/lancer.sh [nom_de_la_config] [duree_en_secondes] [repetition]
#   config : normale | sans-fifo | sans-mlockall | charge

set -e
NOM=${1:-normale}
DUREE=${2:-1800}
REP=${3:-1}

case "$NOM" in
    normale|charge) OPTIONS="" ;;
    sans-fifo)      OPTIONS="--sans-fifo" ;;
    sans-mlockall)  OPTIONS="--sans-mlockall" ;;
    *) echo "Configuration inconnue : $NOM (normale | sans-fifo | sans-mlockall | charge)" >&2
       exit 2 ;;
esac

make
mkdir -p mesures
HORO=$(date +%Y%m%d_%H%M)
BASE="${NOM}_r${REP}_${HORO}"

if [ "$NOM" = "charge" ]; then
    stress-ng --cpu 2 --timeout "$((DUREE + 10))s" >/dev/null 2>&1 &
    STRESS=$!
    trap 'kill "$STRESS" 2>/dev/null || true' EXIT
    sleep 2   # laisse la charge s'installer avant de mesurer
    kill -0 "$STRESS" 2>/dev/null || { echo "stress-ng ne tourne pas" >&2; exit 1; }
    echo "charge : stress-ng actif (pid $STRESS)"
fi

sudo ./station --duree "$DUREE" $OPTIONS\
     --sortie "mesures/jitter_${BASE}.csv" \
     --meteo  "mesures/meteo_${BASE}.csv"

sudo chown "$(id -un):$(id -gn)" "mesures/jitter_${BASE}.csv" "mesures/meteo_${BASE}.csv"