import csv, os ,sys
import matplotlib
matplotlib.use("Agg")  # pas d'ecran : on enregistre en fichier
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
    print("Usage : python3 scripts/analyse.py mesures/fichier.csv")
    sys.exit(1)

fichier = sys.argv[1]
base = os.path.splitext(fichier)[0]   # ex : mesures/jitter_normale_r1_20261121_1030
ecarts = {}
with open(fichier) as f:
    for ligne in csv.DictReader(f):
        ecarts.setdefault(ligne["tache"], []).append(float(ligne["ecart_us"]))

def percentile(v, p):
    v = sorted(v)
    k = min(len(v) - 1, int(round(p / 100 * (len(v) - 1))))
    return v[k]

for tache, v in ecarts.items():
    print(tache, "n=%d" % len(v),
          "P50=%.1f P99=%.1f P99.9=%.1f max=%.1f (us)" % (
              percentile(v, 50), percentile(v, 99), percentile(v, 99.9), max(v)))
    plt.figure()
    plt.hist(v, bins=50)
    plt.xlabel("ecart (us)")
    plt.ylabel("occurrences")
    plt.title("Jitter - %s (%s)" % (tache, os.path.basename(base)))
    plt.savefig("%s_%s.png" % (base, tache))
    plt.close()
