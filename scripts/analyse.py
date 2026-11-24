import csv, os, re, sys

# Echeance : une activation rate son echeance quand son retard de reveil depasse
# la periode de la tache (le travail de chaque tache dure quelques microsecondes).
PERIODES_US = {"acquisition": 100000, "filtrage": 200000, "affichage": 1000000}
NOM = re.compile(r"^jitter_(.+)_r(\d+)_\d{8}_\d{4}\.csv$")
SORTIE = os.environ.get("SORTIE_SYNTHESE", "mesures")
ENTETE = ["n", "p50_ms", "p99_ms", "p999_ms", "max_ms", "echeance_ms", "ratees",
          "ratees_pct", "seuil_strict_ms", "ratees_strict", "ratees_strict_pct"]

def percentile(v, p):
    v = sorted(v)
    k = min(len(v) - 1, int(round(p / 100 * (len(v) - 1))))
    return v[k]

def lire(fichier):
    ecarts = {}
    with open(fichier) as f:
        for ligne in csv.DictReader(f):
            ecarts.setdefault(ligne["tache"], []).append(float(ligne["ecart_us"]))
    return ecarts

def stats(tache, v):
    d1 = PERIODES_US[tache]
    d2 = d1 // 10
    n = len(v)
    r1 = sum(1 for x in v if x > d1)
    r2 = sum(1 for x in v if x > d2)
    return [n] + ["%.3f" % (percentile(v, p) / 1000) for p in (50, 99, 99.9)] + [
        "%.3f" % (max(v) / 1000), "%g" % (d1 / 1000), r1, "%.2f" % (100.0 * r1 / n),
        "%g" % (d2 / 1000), r2, "%.2f" % (100.0 * r2 / n)]

def analyser_un(fichier):
    import matplotlib
    matplotlib.use("Agg")  # pas d'ecran : on enregistre en fichier
    import matplotlib.pyplot as plt
    base = os.path.splitext(fichier)[0]   # ex : mesures/jitter_normale_r1_20261121_1030
    for tache, v in lire(fichier).items():
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

def synthese(fichiers):
    runs = {}
    for f in fichiers:
        m = NOM.match(os.path.basename(f))
        if not m:
            print("ignore (nom non conforme) :", f)
            continue
        cle = (m.group(1), int(m.group(2)))
        if cle in runs:
            print("ERREUR : deux fichiers pour", cle)
            return 1
        runs[cle] = lire(f)
    if not runs:
        print("ERREUR : aucun fichier jitter_<config>_r<n>_<date>_<heure>.csv")
        return 1
    configs = sorted({c for c, _ in runs})
    par_config, par_run, vide = [], [], False
    for c in configs:
        nums = sorted(r for cc, r in runs if cc == c)
        for t in PERIODES_US:
            groupe = [runs[(c, r)].get(t, []) for r in nums]
            for r, v in zip(nums, groupe):
                if v:
                    par_run.append([c, r, t] + stats(t, v))
            tout = [x for v in groupe for x in v]
            if not tout:
                print("ERREUR : aucune mesure pour", c, t)
                vide = True
                continue
            par_config.append([c, len(nums), t] + stats(t, tout))
    os.makedirs(SORTIE, exist_ok=True)
    with open(SORTIE + "/synthese_configurations.csv", "w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["config", "runs", "tache"] + ENTETE)
        w.writerows(par_config)
    with open(SORTIE + "/synthese_par_run.csv", "w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["config", "run", "tache"] + ENTETE)
        w.writerows(par_run)
    with open(SORTIE + "/synthese_configurations.md", "w") as f:
        f.write("| config | runs | tache | " + " | ".join(ENTETE) + " |\n")
        f.write("|" + "---|" * (len(ENTETE) + 3) + "\n")
        for l in par_config:
            f.write("| " + " | ".join(str(x) for x in l) + " |\n")
    print("ecrit : %d lignes par configuration, %d par run" % (len(par_config), len(par_run)))
    return 1 if vide else 0

if len(sys.argv) < 2:
    print("Usage : python3 scripts/analyse.py mesures/fichier.csv")
    print("        python3 scripts/analyse.py --synthese mesures/jitter_*.csv")
    sys.exit(1)
if sys.argv[1] == "--synthese":
    sys.exit(synthese(sys.argv[2:]))
analyser_un(sys.argv[1])
