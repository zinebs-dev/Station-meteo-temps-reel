#!/bin/bash
# Tests de l'issue #31 (options de ligne de commande).
# Usage : sudo -v && ./scripts/test_options.sh
cd "$(dirname "$0")/.." || exit 1
make -s || exit 1

ECHECS=0
verifier() {   # description, code attendu, code obtenu
    if [ "$2" = "$3" ]; then echo "OK        $1"
    else echo "ECHEC     $1 (attendu $2, obtenu $3)"; ECHECS=$((ECHECS + 1)); fi
}

./station --inconnue   >/dev/null 2>&1; verifier "option inconnue -> code 2"           2 $?
./station --duree abc  >/dev/null 2>&1; verifier "--duree abc -> code 2"               2 $?
./station --duree -5   >/dev/null 2>&1; verifier "--duree -5 -> code 2"                2 $?
./station --duree      >/dev/null 2>&1; verifier "--duree sans argument -> code 2"     2 $?
./station surprise     >/dev/null 2>&1; verifier "argument en trop -> code 2"          2 $?
./station --help       >/dev/null 2>&1; verifier "--help -> code 0"                    0 $?

debut=$(date +%s)
sudo ./station --duree 5 >/dev/null 2>&1; code=$?
fin=$(date +%s); d=$((fin - debut))
verifier "--duree 5 -> code 0" 0 $code
if [ "$d" -ge 5 ] && [ "$d" -le 7 ]; then echo "OK        --duree 5 dure ${d} s"
else echo "ECHEC     --duree 5 a dure ${d} s"; ECHECS=$((ECHECS + 1)); fi

./station >/dev/null 2>&1;             verifier "sans sudo -> code 1 (EPERM)"          1 $?

sudo rm -f /tmp/essai_options.csv
sudo ./station --duree 2 --sortie /tmp/essai_options.csv >/dev/null 2>&1
if [ -s /tmp/essai_options.csv ]; then echo "OK        --sortie cree /tmp/essai_options.csv"
else echo "ECHEC     --sortie : CSV absent ou vide"; ECHECS=$((ECHECS + 1)); fi

sudo ./station --duree 2 --force-chaud >/dev/null 2>&1
verifier "--force-chaud : le programme tourne et s'arrete (code 0)" 0 $?

out=$(sudo ./station --duree 10 --force-chaud 2>&1)
if echo "$out" | grep -q "ALERTE"; then
    echo "OK        --force-chaud -> alerte"
else
    echo "ECHEC     --force-chaud -> alerte"; ECHECS=$((ECHECS + 1))
fi

echo; [ "$ECHECS" -eq 0 ] && echo "Aucun echec." || echo "$ECHECS echec(s)."
exit $ECHECS