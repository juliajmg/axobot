#!/bin/bash

for ((j=0; j<1; j++));
do

./kilobots.sh
awk '/Agreggated/,/TIME/' history.json > DATA0a
awk 'ORS=NR%2?FS:RS' DATA0a > DATA1a
sort DATA1a | uniq --count > DATA2a
grep '"Agreggated": 0' DATA2a > DATA3a
sed -i "s/,//g" DATA3a
awk '{print $1,$4,$5}' DATA3a > DATA3.1a
sort -n -k 3 DATA3.1a >> DATOS_agregacion

awk '/TIME/,/status/' history.json > DATA0s
awk 'ORS=NR%2?FS:RS' DATA0s > DATA1s
sort DATA1s | uniq --count > DATA2s
grep '"status": 0' DATA2s > DATA3s
sed -i "s/,//g" DATA3s
awk '{print $1,$2,$3}' DATA3s > DATA3.1s
sort -n -k 3 DATA3.1s >> DATOS_status


awk '/x_position/,/y_position/' history.json > DATAcoords
awk 'ORS=NR%2?FS:RS' DATAcoords > DATAcoords1
sed -i "s/,//g" DATAcoords1
awk '{print $2,$4}' DATAcoords1 > DATAcoords2


done

awk '{print $1,$3}' DATOS_agregacion > A=750_S=350_N=145_a
awk '{print $1,$3}' DATOS_status > A=750_S=350_N=145_s
awk '{print $1,$2}' DATAcoords2 > A=750_S=350_N=145_coord

rm DATOS_agregacion
rm DATOS_status
