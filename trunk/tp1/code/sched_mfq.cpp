#include <vector>
#include <queue>
#include "sched_mfq.h"
#include "basesched.h"

using namespace std;

SchedMFQ::SchedMFQ(vector<int> argn) {
	// MFQ recibe los quantums por parámetro
	this->def_quantum = argn;
	this->n = argn.size();
	this->vq = vector<queue<int> >(n);
	this->quantum = 0;
	this->cur_pri = -1; // safe value
}

void SchedMFQ::load(int pid) {
	vq[0].push(pid); // llegó una tarea nueva
	unblock_to.push_back(-1);
}

void SchedMFQ::unblock(int pid) {
	vq[unblock_to[pid]].push(pid); // Se re-encola la tarea en cola unblock_to[pid]
}

int SchedMFQ::tick(const enum Motivo m) {
	if (m == EXIT) {
		// El proceso actual no se re-encola
		return next();
	} else if (m == BLOCK) {
		// El proceso se bloqueó --> sube de prioridad
		if (cur_pri > 0) cur_pri--;
		unblock_to[current_pid()] = cur_pri;
		return next();
	} else { // m == TICK
		if (quantum) {
			// caso no-switch
			quantum--;
			return current_pid();
		} else {
			// Caso switch, el proceso agotó el quantum --> baja de prioridad
			if (current_pid() != IDLE_TASK) {
				if (cur_pri < n-1) cur_pri++;
				vq[cur_pri].push(current_pid());
			}
			return next();
		}
	}
}

int SchedMFQ::next(void) {
	// Elijo el nuevo pid
	int nuevo = IDLE_TASK;
	quantum = 0; // El IDLE_TASK tiene un quantum nulo, así al próximo tick verifica otra vez si hay alguien esperando.
	for (cur_pri = 0; cur_pri < n; cur_pri++) {
		if (!vq[cur_pri].empty()) {
			nuevo = vq[cur_pri].front();
			vq[cur_pri].pop();
			quantum = def_quantum[cur_pri] - 1;
			return nuevo;
		}
	}
	return nuevo;
}
