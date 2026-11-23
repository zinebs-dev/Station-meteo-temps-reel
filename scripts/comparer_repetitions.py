#!/usr/bin/env python3
"""Compare les repetitions de la configuration normale (issue #34).
Usage : python3 scripts/comparer_repetitions.py mesures/jitter_normale_*.csv"""
import csv
import sys
SEUIL_P50 = 2.0     # rapport max/min tolere entre repetitions pour le P50
SEUIL_P99 = 3.0     # idem pour le P99
def percentile(v, p):
    v = sorted(v)
    return v[min(len(v) - 1, int(round(p / 100 * (len(v) - 1))))]
def charger(fichier):
    ecarts = {}
    with open(fichier) as f:
        for ligne in csv.DictReader(f):
            ecarts.setdefault(ligne["tache"], []).append(float(ligne["ecart_us"]))
    return ecarts
def ratio(valeurs):
    return max(max(valeurs), 1.0) / max(min(valeurs), 1.0)
fichiers = sys.argv[1:]
if len(fichiers) < 2:
    sys.exit("Usage : python3 scripts/comparer_repetitions.py rep1.csv rep2.csv rep3.csv")
donnees = [charger(f) for f in fichiers]
taches = sorted(set().union(*donnees))
tout_ok = True
print("%-12s %-4s %8s %9s %9s %9s %9s" % ("tache", "rep", "n", "P50 ms", "P99 ms",
                                         "P99.9 ms", "max ms"))
for t in taches:
    p50, p99 = [], []
    for i, d in enumerate(donnees, 1):
        v = d.get(t, [])
        if not v:
            print("%-12s %-4d (aucune mesure)" % (t, i))
            tout_ok = False
            continue
        p50.append(percentile(v, 50))
        p99.append(percentile(v, 99))
        print("%-12s %-4d %8d %9.2f %9.2f %9.2f %9.2f" % (t, i, len(v), p50[-1] / 1000,
              p99[-1] / 1000, percentile(v, 99.9) / 1000, max(v) / 1000))
    if len(p50) >= 2:
        r50, r99 = ratio(p50), ratio(p99)
        ok = r50 <= SEUIL_P50 and r99 <= SEUIL_P99
        tout_ok = tout_ok and ok
        print("%-12s rapport max/min : P50 x%.2f (<= %.0f)  P99 x%.2f (<= %.0f)  -> %s\n" % (
              t, r50, SEUIL_P50, r99, SEUIL_P99, "COHERENT" if ok else "A EXAMINER"))
print("Verdict :", "repetitions coherentes." if tout_ok else "au moins une tache a examiner.")
