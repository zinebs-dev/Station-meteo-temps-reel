#!/bin/bash
# Test systeme de l'issue #32.
# Usage : sudo -v && ./scripts/test_systeme.sh [duree_secondes] (defaut : 600)
REPO="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO" || exit 1
DUREE=${1:-600}
CSV=/tmp/test_systeme_$(date +%Y%m%d_%H%M).csv
make -s || exit 1
echo "Lancement : ./station --duree $DUREE --sortie $CSV"
sudo ./station --duree "$DUREE" --sortie "$CSV" \
 >/tmp/station_sortie.txt 2>/tmp/station_erreurs.txt &
PID_SUDO=$!
sleep 5
echo; echo "--- Threads (attendu : FF 60 / 50 / 40 / 30 + un thread TS pour le journal) ---"
ps -eLo pid,tid,cls,rtprio,comm | grep -E "CLS|station"
wait $PID_SUDO
CODE=$?
echo; echo "--- Code de retour : $CODE (attendu : 0) ---"
echo; echo "--- Lignes par tache dans le CSV ---"
cut -d, -f1 "$CSV" | sort | uniq -c
echo; echo "--- Messages de fin (stderr) ---"
tail -5 /tmp/station_erreurs.txt
echo; echo "--- Jitter par tache (analyse.py, images dans /tmp/analyse_systeme) ---"
mkdir -p /tmp/analyse_systeme
( cd /tmp/analyse_systeme && python3 "$REPO/scripts/analyse.py" "$CSV" )
echo; echo "CSV : $CSV"
exit $CODE
