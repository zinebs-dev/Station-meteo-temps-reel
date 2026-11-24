#!/bin/bash
# Controle de analyse.py --synthese sur un echantillon de 30 mesures calculable a la main.
cd "$(dirname "$0")/.." || exit 1
ECH=tests/echantillon/jitter_test_r1_20260101_0000.csv
SORTIE=$(mktemp -d)
SORTIE_SYNTHESE="$SORTIE" python3 scripts/analyse.py --synthese "$ECH" > /dev/null \
    || { echo "ECHEC : analyse.py a echoue"; exit 1; }
if diff <(tail -n +2 "$SORTIE/synthese_configurations.csv") tests/echantillon/attendu.csv; then
    echo "OK : analyse.py correspond au calcul manuel"
else
    echo "ECHEC : differences ci-dessus (< obtenu, > attendu)"
    exit 1
fi
