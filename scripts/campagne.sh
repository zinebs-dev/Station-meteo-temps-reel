#!/bin/bash
# Usage : ./scripts/campagne.sh [duree_en_secondes] [repetitions]
# Enchaine les 4 configurations, en alternance, pour chaque repetition.
set -e
cd "$(dirname "$0")/.."
DUREE=${1:-600}
REPS=${2:-3}

sudo -v
( while kill -0 $$ 2>/dev/null; do sudo -n true; sleep 50; done ) &

VMSTAT="$HOME/campagne_vmstat_$(date +%Y%m%d_%H%M).txt"
vmstat -t 1 > "$VMSTAT" &
VPID=$!
trap 'kill $VPID 2>/dev/null || true' EXIT

echo "noyau : $(uname -r) | coeurs : $(nproc) | debut : $(date)"
for rep in $(seq 1 "$REPS"); do
    for cfg in normale sans-fifo sans-mlockall charge; do
        echo "=== run : $cfg, repetition $rep ($(date +%H:%M:%S)) ==="
        ./scripts/lancer.sh "$cfg" "$DUREE" "$rep"
        sleep 5
    done
done
echo "campagne terminee : $(date)"
echo "noyau : $(uname -r)"