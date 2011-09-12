#include "sched_lottery.h"
#include "basesched.h"
#include <stdio.h>
#include <errno.h>
#include <algorithm>
#include <iostream> //debug
using namespace std;

SchedLottery::SchedLottery(vector<int> argn) {
	quantum = argn[0];
	semilla = argn[1];
	contador = quantum;
	tot_tickets = 0;
	srand(semilla);
}

void SchedLottery::load(int pid) {
	// Llega una tarea nueva entonces la agrego en la lista de procesos disponibles con 1 ticket.
	tickets.push_back(make_pair(pid, 1));
	tot_tickets++;
}

void SchedLottery::unblock(int pid) {
	//Busco la tarea en la lista de bloqueadas.
	for (unsigned int i = 0; i < blocked_tasks.size(); i++) {
		if(blocked_tasks[i].first == pid) {
			//La agrego en la lista de disponibles y la elimino de la lista de bloqueadas.
			tickets.push_back(blocked_tasks[i]);
			blocked_tasks.erase(blocked_tasks.begin() + i);
			
			//Sumo la cantidad de tickets que participan del sorteo.
			tot_tickets += blocked_tasks[i].second;
			return;
		}
	}
}

int SchedLottery::tick(const enum Motivo m) {
	ticket_actual.second = 1;
// 	cout << "tickets: " << endl;
// 	for (unsigned int i = 0; i < tickets.size(); i++) {
// 		cout << "[" << tickets[i].second << "]";
// 	}
// 	cout << endl;
// 	cout << "#TICKETS: " << tot_tickets << endl;
// 	for (unsigned int i = 0; i < blocked_tasks.size(); i++) {
// 		cout << "FUERA!!![" << blocked_tasks[i].first << "]" << endl;
// 		sleep(1);
// 	}
// 	
// 	for (unsigned int i = 0; i < tickets.size(); i++) {
// 		cout << "[" << tickets[i].first << "]";
// 	}

	if (m == EXIT) {
		// Si el pid actual terminó, sigue el próximo.
		tot_tickets -= ticket_actual.second;
		if (tickets.empty()) {
			return IDLE_TASK;
		} else {			
			//
		}
	} else if (m == BLOCK) {
// 		cout << "contador: " << contador << endl;
		contador--;
		int ciclos_usados = quantum - contador ;
// 		cout << "usados: " << ciclos_usados << endl;
		
		int tickets_recompenza;
		if(ciclos_usados > 0) {
			tickets_recompenza = quantum / ciclos_usados;
		}
		//sleep(1);

		//Busco la tarea en la lista de disponibles.
		ticket_actual.second = tickets_recompenza;
		blocked_tasks.push_back(ticket_actual);

		//Resto la cantidad de tickets que tenía la tarea que se bloqueó.
		//Su valor siempre es uno ya que la tarea se estaba ejecutando.
		tot_tickets -= ticket_actual.second;

	// Como se llama a tick cuando YA PASO el ciclo, cuando llega a 1 la
	// variable contador es que ya se le acabo el quantum
	} else if (contador == 1 && current_pid() != IDLE_TASK) {
		tickets.push_back(make_pair(current_pid(), 1));
		
	} else {
		if(current_pid() == IDLE_TASK && !tickets.empty()) {
			
		} else {
			// Siempre sigue el pid actual mientras no termine.
			if(contador > 0) 
				contador--;
			return current_pid();
		}
	}

	if(tickets.size() == 0) {
		return IDLE_TASK;
	}
	
	//Realizo el sorteo.
	//Genero el número del ticket ganador al azar.
	int ticket_ganador = 0;
	if (tot_tickets > 1) 
		ticket_ganador = (rand() % (tot_tickets - 1));
	int suma_parcial = 0;

	for (unsigned int i = 0; i < tickets.size(); i++) {
		suma_parcial += tickets[i].second;
		if(suma_parcial >= ticket_ganador) {
			ticket_actual = tickets[i];
			int sig = tickets[i].first;
			tickets.erase(tickets.begin() + i);
			contador = quantum;
			
			return sig;
		}
	}
	cout << "ERROR" << "ganador: " << ticket_ganador << endl << "total Tickets" << tot_tickets << endl;

}