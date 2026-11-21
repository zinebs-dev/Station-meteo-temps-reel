if (!exists("f")) f = "mesures/meteo.csv"
if (!exists("out")) out = "mesures/figures/meteo"
set terminal png size 1000,450
set datafile separator ","
set grid
set key off
set xrange [0:1850]
set xtics 300
set xlabel "temps (s)"

set output out."_temperature.png"
set title "Temperature sur 30 minutes (donnees simulees)"
set ylabel "temperature (degres C)"
plot f using 1:2 with lines lw 2

set output out."_humidite.png"
set title "Humidite sur 30 minutes (donnees simulees)"
set ylabel "humidite (%)"
plot f using 1:3 with lines lw 2