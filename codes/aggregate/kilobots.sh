#!/bin/bash

a=80.0
b=`expr "$a*2-50" | bc -l`

sed -i "s/#define NIDO AQUI/#define NIDO $a/g" aggregate.h
sed -i "s/int nido = AQUI1;/int nido = $a;/g" startjson.c
sed -i "s/double nido_r = AQUI2;/double nido_r = $b;/g" startjson.c

gcc startjson.c -o cscript0 -lm -Wall -g
./cscript0 > start.json
sed -i 's/#/"/g' start.json
sed -i "s/int nido = $a;/int nido = AQUI1;/g" startjson.c
sed -i "s/double nido_r = $b;/double nido_r = AQUI2;/g" startjson.c

rm cscript
rm agreggate.o
make
./cscript -b start.json


sed -i "s/#define NIDO $a/#define NIDO AQUI/g" agreggate.h
echo $b
