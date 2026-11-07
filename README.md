# Station-meteo-temps-reel

## Dépendances

```
sudo apt install -y build-essential gdb git make gnuplot python3 python3-matplotlib
```

## Compilation

```
make
```

## Exécution

```
sudo ./station
```

`sudo` est nécessaire pour activer SCHED_FIFO.

## Structure

- `src/` : fichiers .c
- `include/` : interfaces .h
- `tests/` : tests unitaires
- `scripts/` : lancer.sh, analyse.py, courbes.gp
- `mesures/` : CSV et graphiques
- `docs/` : fiche de cadrage, rapport, schémas
