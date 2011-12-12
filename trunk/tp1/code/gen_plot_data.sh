#!/bin/bash

function ej8_solo_sorteos()
{
	# Archivo con la salida del simusched
	f=$1

	# Archivo usado para cosas temporales. Muchas veces para evitar grabar un
	# archivo vacio al hacer <cmd> < $tmpf > $tmpf
	tmp=$(mktemp)
	
	# Dejamos en $tmpf solo la salida hasta que termina la primer tarea
	first_exit=$(grep -m1 -n EXIT $f | cut -d ':' -f1)
	head -n $first_exit $f > $tmp
	mv $tmp $f
	
	# Sacamos las lineas que no son "CPU", porque no nos interesan para los sorteos.
	grep ^CPU $f > $tmp
	mv $tmp $f
	
	# Dejamos las lineas solo en las que el CPU cambio a ejecutar otra tarea. Eso
	# implica un sorteo del schedLottery. Es decir, sacamos los repetidos ignorando
	# los primeros dos campos (CPU y ciclo de clock)
	uniq -f2 < $f > $tmp
	mv $tmp $f
	
	# Sacamos lineas en las que no se ejecuta ninguna tarea (es decir, se ejecuta
	# la tarea -1)
	grep -v '\-1' $f > $tmp
	mv $tmp $f

	rm -f $tmp
}



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



# Ej 8
# grafico 1


F="out/ej8-1.out"
ITERS="$(seq 1 100)"
TSK="ej8-binomial.tsk"
TSK_LINES=$(cat $TSK | grep -v '^#' | grep -P -v "^( |\t)*$" | wc -l)
Q=10
PROC=0

rm -f $F
rm -f $F-2

tmpf=$(mktemp)

for i in $ITERS; do

	# Corremos el simulador para este quantum/SEED
	SEED=$RANDOM
	#echo "Corriendo ./simusched $TSK 0 SchedLottery $Q $SEED  > $tmpf"
	./simusched $TSK 0 SchedLottery $Q $SEED  > $tmpf
	
	# "Limpiamos" $tmpf para dejar solo las lineas en las que el CPU cambio de tarea a ejecutar
	ej8_solo_sorteos $tmpf

	# Nos fijamos la cantidad de sorteos y las veces que gano el proceso $PROC
	tot=$(wc -l $tmpf | cut -d ' ' -f1)
	won=$(cat $tmpf | cut -d ' ' -f3 | grep -c $PROC)

	# Como tiene una distribucion binomial, la esperanza es n*p. Es decir,
	# n=cantidad de sorteos y p=1/cantidad de procesos

	# Lista con todos los numeros de procesos que se usaron
	proc_list=$(cat $tmpf | cut -d ' ' -f3 | sort -u)

	# proc_list es un string, no un array. La forma facil de saber el largo es
	# recorrerlo, lamentablemente
	proc_size=0
	for x in $proc_list; do
		proc_size=$((proc_size+1))
	done

	# Con precision
	n=$tot
	esperanza=$(bc -l <<< "$n * (1/$proc_size)")
	error=$(bc -l <<< "$esperanza - $won")
	error=${error#-}

	#echo $i $error >> $F

	# Redondeando al numero entero menor
	# Si la distancia es 0.5 se puede decir que no le erro, porque la
	# cantidad de sorteos que gano es entera. Entonces, usar numeros
	# enteros para estas cuentas tiene cierto sentido
	#echo -n "El error es: $error"
	#error=$(awk "BEGIN { printf int($error) }")
	#echo " redondenado a $error"
	#echo $i $error >> $F

	# Usando redondeo
	#echo -n "El error es: $error"
	error=$(awk "BEGIN { printf \"%.0f\", $error }")
	#echo " redondenado a $error"
	echo $i $error >> $F
done

rm -f $tmpf


	
#	for proc in $proc_list; do
#		won=$(cat $tmpf | cut -d ' ' -f3 | grep -c $proc)
#		tot=$(wc -l $tmpf | cut -d ' ' -f1)
#		echo "proceso $proc gano $won veces en $tot sorteos"
#	done

