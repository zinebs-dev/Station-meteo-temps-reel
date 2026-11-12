set terminal png size 900,400
set output "mesures/temperature.png"
set datafile separator ","
set xlabel "temps (s)"
set ylabel "temperature (C)"
plot "mesures/meteo.csv" using 1:2 with lines title "temperature"
