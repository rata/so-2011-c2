#include <vector>
#include "sched_rr.h"
#include "basesched.h"
#include <iostream>

using namespace std;

SchedRR::SchedRR(vector<int> argn)
{
	// Round robin recibe el quantum por parámetro
	quantum = argn[0];
	contador = quantum;
}

void SchedRR::load(int pid)
{
	// llegó una tarea nueva
	q.push(pid);
}

void SchedRR::unblock(int pid)
{
	// La tarea pid se desbloqueó. La agregamos al final para asegurarnos
	// no tener inanicion
	q.push(pid);
}

int SchedRR::tick(const enum Motivo m) {

	if (m == EXIT) {
		// Si el pid actual terminó, sigue el próximo.
		if (q.empty()) {
			return IDLE_TASK;
		} else {
			int sig = q.front();
			q.pop();
			contador = quantum;
			return sig;
		}
	} else if (m == BLOCK) {
		if (q.empty()) {
			return IDLE_TASK;
		} else {
			contador = quantum;
			int sig = q.front(); q.pop();
			return sig;
		}

	// Como se llama a tick cuando YA PASO el ciclo, cuando llega a 1 la
	// variable contador es que ya se le acabo el quantum

	} else if (contador == 1) {
		contador = quantum;
		q.push(current_pid());
		int sig = q.front(); q.pop();
		return sig;
	} else {
		// Siempre sigue el pid actual mientras no termine.
		if (current_pid() == IDLE_TASK && !q.empty()) {
			contador = quantum;
			int sig = q.front(); q.pop();
			return sig;
		} else {
			contador--;
			return current_pid();
		}
	}
}
