#!/usr/bin/env python3
"""Histogrammes de jitter : une figure par tache, repetitions superposees
sur les memes axes (vue complete en echelle log + zoom).
Usage : python3 scripts/figures.py mesures/jitter_normale_*.csv
Pour comparer des configurations, passer des fichiers de configs differentes."""
import csv, os, sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

TACHES = [("acquisition", "100 ms"), ("filtrage", "200 ms"), ("affichage", "1 s")]
SORTIE = "mesures/figures"

def lire(fichier):
    d = {}
    with open(fichier) as f:
        for l in csv.DictReader(f):
            d.setdefault(l["tache"], []).append(float(l["ecart_us"]) / 1000.0)  # ms
    return d

def pct(v, p):
    v = sorted(v)
    return v[min(len(v) - 1, int(round(p / 100 * (len(v) - 1))))]

def etiquette(fichier):
    nom = os.path.basename(fichier)
    return nom.replace("jitter_", "").replace(".csv", "")

if len(sys.argv) < 2:
    sys.exit("Usage : python3 scripts/figures.py fichier1.csv [fichier2.csv ...]")

os.makedirs(SORTIE, exist_ok=True)
runs = [(etiquette(f), lire(f)) for f in sys.argv[1:]]

for tache, periode in TACHES:
    series = [(nom, d.get(tache, [])) for nom, d in runs]
    series = [(n, v) for n, v in series if v]
    if not series:
        continue
    tous = [x for _, v in series for x in v]
    bas, haut = min(tous), max(tous)
    zoom_haut = max(pct(v, 99) for _, v in series) * 1.2

    fig, (a1, a2) = plt.subplots(1, 2, figsize=(13, 4.8))
    for nom, v in series:
        lab = "%s  (P50 %.2f ms, P99 %.1f ms)" % (nom, pct(v, 50), pct(v, 99))
        a1.hist(v, bins=60, range=(bas, haut), histtype="step", linewidth=1.6, label=lab)
        a2.hist(v, bins=80, range=(max(bas, 0), zoom_haut), histtype="step", linewidth=1.6)
    a1.set_yscale("log")
    a1.set_title("Etendue complete (echelle log)")
    a1.set_xlabel("ecart reel - prevu (ms)")
    a1.set_ylabel("nombre d'executions")
    a1.legend(fontsize=7)
    a2.set_yscale("log")
    a2.set_title("Zoom jusqu'au P99 maximal (echelle log)")
    a2.set_xlabel("ecart reel - prevu (ms)")
    a2.set_ylabel("nombre d'executions")
    for a in (a1, a2):
        a.grid(True, alpha=0.3)
    fig.suptitle("Jitter de la tache %s (periode %s)" % (tache, periode))
    fig.tight_layout()
    sortie = "%s/jitter_%s.png" % (SORTIE, tache)
    fig.savefig(sortie, dpi=120)
    plt.close(fig)
    print("ecrit", sortie)