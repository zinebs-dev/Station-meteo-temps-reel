#!/bin/bash
# Convertit la sortie texte de l'affichage en CSV meteo.
# Usage : ./scripts/meteo_depuis_affichage.sh affichage.txt meteo.csv
[ $# -eq 2 ] || { echo "Usage : $0 affichage.txt meteo.csv"; exit 2; }
awk 'BEGIN { print "temps_s,temperature,humidite,pression"; prec = -1; decal = 0 }
/^\[[0-9][0-9]:[0-9][0-9]:[0-9][0-9]\.[0-9]+\] T=/ {
    s = substr($1, 2, length($1) - 2); split(s, h, ":")
    t = h[1] * 3600 + h[2] * 60 + h[3] + decal
    if (prec >= 0 && t < prec) { decal += 86400; t += 86400 }   # passage de minuit
    prec = t
    if (debut == "") debut = t
    sub(/^T=/, "", $2); sub(/^H=/, "", $4); sub(/^P=/, "", $6)
    printf "%.3f,%s,%s,%s\n", t - debut, $2, $4, $6
}' "$1" > "$2"
echo "$2 : $(($(wc -l < "$2") - 1)) lignes"
