#!/bin/bash

# Grafico 1

F="out/ej7-1.out"
ITERS="$(seq 1 30) 40 60 80 100 150 200 500 1000"
TSK="ej7-reentrega-rendimiento.tsk"
TSK_LINES=$(cat $TSK | grep -v '^#' | grep -P -v "^( |\t)*$" | wc -l)

rm -f $F

for q in $ITERS; do
	ciclos=$(./simusched $TSK 1 SchedRR $q | tail -n 1 | cut -d ' ' -f2)
	proc_per_t=$(expr $ciclos / $TSK_LINES)
	echo $q $proc_per_t >> $F
done


# Grafico 2

F="out/ej7-2.out"
ITERS="$(seq 1 30) 40 60 80 100 150 200 500 1000"
TSK="ej7-reentrega-rendimiento.tsk"
TSK_LINES=$(cat $TSK | grep -v '^#' | grep -P -v "^( |\t)*$" | wc -l)

rm -f $F

tmpf=$(mktemp)
for q in $ITERS; do

	# Corremos el simulador para este quantum
	./simusched $TSK 1 SchedRR $q  > $tmpf

	# Una "lista" con los ciclos en los que termino cada proceso de este lote
	# Es decir, una lista tipo: 10, 11, 12 que significa que en el ciclo 10
	# termino el primer proceso, en el 11 el segundo y en el 12 el tercero
	ciclos=$(grep EXIT $tmpf | cut -d ' ' -f2)

	# La cantidad de veces que iteramos dentro del segundo for. Para saber
	# cuantos procesos terminaron
	i=1
	for c in $ciclos; do
		# Escribimos: <quantum> <ciclo> <cantidad de procesos que
		# terminaron hasta este ciclo>
		echo $q $c $i >> $F
		i=$((i+1))
	done
done

rm $tmpf
