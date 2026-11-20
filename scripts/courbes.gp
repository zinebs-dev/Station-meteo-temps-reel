if (!exists("f")) f = "mesures/meteo.csv"
set terminal png size 900,400
set datafile separator ","
set xlabel "temps (s)"

set output "mesures/temperature.png"
set ylabel "temperature (C)"
plot f using 1:2 with lines title "temperature"

set output "mesures/humidite.png"
set ylabel "humidite (%)"
plot f using 1:3 with lines title "humidite"