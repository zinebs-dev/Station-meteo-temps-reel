#!/bin/bash
# Campagne de reference (issue #34) : repetitions de la configuration "normale".
# Usage (apres "make", sans VS Code, PC branche) :
#   sudo -v
#   sudo nohup ./scripts/campagne_normale.sh > mesures/campagne.log 2>&1 &
#   tail -f mesures/campagne.log      # Ctrl+C quitte tail, la campagne continue
# Variables : REPETITIONS (defaut 3), PAUSE entre repetitions en secondes (defaut 60)
cd "$(dirname "$0")/.." || exit 1
REP=${REPETITIONS:-3}
PAUSE=${PAUSE:-60}
PROPRIO=${SUDO_USER:-$USER}
mkdir -p mesures
make -q station 2>/dev/null || { echo "make necessaire (SANS sudo) avant la campagne"; exit 1; }
CONF=mesures/config_vm_$(date +%Y%m%d_%H%M).txt
{
    echo "Date : $(date)"
    echo "--- Systeme ---";  lsb_release -d; uname -r
    echo "--- CPU ---";      nproc; lscpu | grep -E "Model name|Hypervisor vendor"
    echo "--- Memoire ---";  free -h | head -2
    echo "--- Capacite jitter ---"; grep -n "define JITTER_CAPACITE" src/jitter.c
    echo "--- Commit ---";   git -c safe.directory="$PWD" log --oneline -1 2>/dev/null
} > "$CONF"
echo "Configuration notee dans $CONF"
for i in $(seq 1 "$REP"); do
    DEB=$(date +%Y%m%d_%H%M)
    AFF=mesures/affichage_normale_rep${i}_${DEB}.txt
    ERR=mesures/stderr_normale_rep${i}_${DEB}.txt
    echo "=== Repetition $i/$REP : debut $(date +%T) ==="
    ./scripts/lancer.sh normale > "$AFF" 2> "$ERR"
    CODE=$?
    echo "Repetition $i terminee a $(date +%T), code de retour $CODE"
    [ "$CODE" -ne 0 ]              && echo "ATTENTION : code de retour non nul"
    grep -q "mlockall"      "$ERR" && echo "ATTENTION : mlockall a echoue (config non conforme)"
    grep -q "tableau plein" "$ERR" && echo "ATTENTION : tableau jitter plein (CSV tronque)"
    [ "$i" -lt "$REP" ] && sleep "$PAUSE"
done
chown -R "$PROPRIO" mesures 2>/dev/null
chown "$PROPRIO" journal.log 2>/dev/null
echo "Campagne terminee : $(date)"
ls -l mesures
