#include "tasks.h"
#include <cstdlib>
#include <time.h>
#include <assert.h>
#include <algorithm>
#include <iostream> //debug

using namespace std;

void TaskCPU(vector<int> params) { // params: n
	uso_CPU(params[0]); // Uso el CPU n milisegundos.
}

void TaskIO(vector<int> params) { // params: ms_cpu, ms_io,
	uso_CPU(params[0]); // Uso el CPU ms_cpu milisegundos.
	uso_IO(params[1]); // Uso IO ms_io milisegundos.
}

void TaskAlterno(vector<int> params) { // params: ms_cpu, ms_io, ms_cpu, ...
	for(int i = 0; i < (int)params.size(); i++) {
		if (i % 2 == 0) uso_CPU(params[i]);
		else uso_IO(params[i]);
	}
}

void TaskCon(vector<int> params) {
	int n = params[0];
	int bmin = params[1];
	int bmax = params[2];
	int tiempo_bloqueado;

	// Que explote si el rango no esta bien definido
	assert(bmin > 0);
	assert(bmax >= bmin);

	srand(time(NULL));
	for (int i = 0; i < n; i++) {
		tiempo_bloqueado = (rand() % (bmax - bmin)) + bmin;
		uso_IO(tiempo_bloqueado);
	}
}


void TaskBatch(vector<int> params)
{
	int tot = params[0] - 1;
	int blocks = params[1];

	//Sino es imposible cumplirlo.
	assert((tot - blocks) >= 0);

	// Este vector define en que ciclo de reloj voy a interrumpir al CPU
	// para hacer la llamada bloqueante
	std::vector<int> numbers;
	std::vector<int> IO_times;
	for( int i = 0; i < tot; i++) {
		numbers.push_back(0);
	}

	//Generamos un vector con los numeros al azar
	srand(time(NULL));	
	int i = blocks;
	while(i > 0) {
		int v;
		v = (rand() % (tot));
		
		if(numbers[v] == 0) {
			IO_times.push_back(v);
			i--;
			numbers[v] = 1;
		}
	}

	//cout << "tot es: " << tot << endl;
	//int tick_actual = 0;
	for (int i = 0; i < tot; i++) {
		if (numbers[i] == 1) {
				uso_IO(1);
		} else {	
				uso_CPU(1);
		}

	}
//	for (int i = 0; i < blocks; i++) {
//		cout << "\tcall_to_block[" << i << "]: " << call_to_block[i] << endl;
//		cout << "\ttick_actual es: " << tick_actual << endl;
//		
//		int cpu_time = call_to_block[i] - tick_actual;
//		if (cpu_time > 0) {
//			cout << "\t\tuso_CPU" << endl;
//			uso_CPU(cpu_time);
//		} else {
//			cout << "\t\t NO uso_CPU" << endl;
//			cpu_time = 0;
//		}
//
//		uso_IO(1);
//		tick_actual += cpu_time + 1;
//	}
//
//	if (tot > tick_actual)
//		uso_CPU(tot - tick_actual + 1);
	
	return;
}


/*
void TaskBatch(vector<int> params) { // params: ms_cpu, ms_io,
	std::vector<int> numbers;
	std::vector<int> IO_times;
	for( int i = 0; i < params[0]; i++) {
		numbers.push_back(0);
	}

	//Generamos un vector con los numeros al azar
	srand(time(NULL));	
	int i = params[1];
	while(i > 0) {
		int v;
		v = (rand() % params[0]);
		
		if(numbers[v] == 0) {
			IO_times.push_back(v);
			i--;
			numbers[v] = 1;
		}
	}
	
	//Ordenamos el vector
	sort(IO_times.begin(), IO_times.end());

	//Ejecutamos los usos de la CPU.
	uso_CPU(IO_times[0]);
	uso_IO(1);
	for (int i = 1; i < params[1]-1; i++) {
		uso_CPU(IO_times[i] - IO_times[i-1]);
		uso_IO(1);	
	}
	uso_CPU(params[0] - IO_times[params[1]-1]);
	

}
*/


void TaskAlternoRand(vector<int> params) { // params: ms_cpu, ms_io, ms_cpu, ...
	int n = params[0];
	int bmin = params[1];
	int bmax = params[2];
	int tiempo_bloqueado;

	// Que explote si el rango no esta bien definido
	assert(bmin > 0);
	assert(bmax >= bmin);

	srand(time(NULL));

	for(int i = 0; i < n; i++) {
		tiempo_bloqueado = (rand() % (bmax - bmin)) + bmin;
		if (i % 2 == 0)
			uso_CPU(tiempo_bloqueado);
		else
			uso_IO(tiempo_bloqueado);
	}
}


void tasks_init(void) {
	/* Todos los tipos de tareas se deben registrar acá para poder ser usadas.
	 * El segundo parámetro indica la cantidad de parámetros que recibe la tarea
	 * como un vector de enteros, o -1 para una cantidad de parámetros variable. */
	register_task(TaskCPU, 1);
	register_task(TaskIO, 2);
	register_task(TaskAlterno, -1);
	register_task(TaskCon, 3);
	register_task(TaskBatch, 2);
	register_task(TaskAlternoRand, 3);
	register_task(TaskBatch, 2);
}
