#include "sched_lottery.h"
#include "basesched.h"
#include <stdio.h>
#include <errno.h>
#include <math.h>
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
			//Sumo la cantidad de tickets que participan del sorteo.
			tot_tickets += blocked_tasks[i].second;
			
			//La agrego en la lista de disponibles y la elimino de la lista de bloqueadas.
			tickets.push_back(blocked_tasks[i]);
			blocked_tasks.erase(blocked_tasks.begin() + i);
			
			return;
		}
	}
}

int SchedLottery::tick(const enum Motivo m) {
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
		
	} else if (m == BLOCK) {
// 		cout << "contador: " << contador << endl;
		contador--;
		int ciclos_usados = quantum - contador ;
// 		cout << "usados: " << ciclos_usados << endl;
		
		//Si se acabó el quantum no le doy recompenza
		//Si no le doy como recompenza la fracción de tiempo que usó.
		int tickets_recompenza = 1;
		if(ciclos_usados > 0) {
			tickets_recompenza = ceil((quantum * 1.0)/ciclos_usados);
		}
		//sleep(1);

		//Busco la tarea en la lista de disponibles.
		ticket_actual.second = tickets_recompenza;
		blocked_tasks.push_back(ticket_actual);

		//Resto la cantidad de tickets que tenía la tarea que se bloqueó.
		//Su valor siempre es uno ya que la tarea se estaba ejecutando.

	// Como se llama a tick cuando YA PASO el ciclo, cuando llega a 1 la
	// variable contador es que ya se le acabo el quantum
	} else if (contador == 1 && current_pid() != IDLE_TASK) {
		//Si se acabó el quantum del proceso lo guardo en la lista
		//de disponibles con 1 ticket ya que le voy a sacar el control
		tickets.push_back(make_pair(current_pid(), 1));
		tot_tickets++;
	} else {
		// Siempre sigue el pid actual mientras no termine.
		// O esté ejecutando el IDLE_TASK y haya algún proceso disponible.
		if(!(current_pid() == IDLE_TASK && !tickets.empty())) {
			contador--;
			return current_pid();
		}
	}
	

	//Elijo el proceso siguiente.
	if(tickets.empty()) {
		return IDLE_TASK;
	}
	
	//Realizo el sorteo.
	//Genero el número del ticket ganador al azar.
	int ticket_ganador = 0;
	if (tot_tickets > 1) 
		ticket_ganador = (rand() % (tot_tickets - 1));
	
	//Busco al ganador del sorteo
	int suma_parcial = 0;
	for (unsigned int i = 0; i < tickets.size(); i++) {
		suma_parcial += tickets[i].second;
		if(suma_parcial >= ticket_ganador) {
			tot_tickets -= ticket_actual.second;
			//Le asigno el control del procesador y lo saco de la lista
			ticket_actual = tickets[i];
			int sig = tickets[i].first;
			tickets.erase(tickets.begin() + i);
			contador = quantum;
			//Cuando cargo a un proceso sólo le dejamos un ticket.
			ticket_actual.second = 1;
			
			return sig;
		}
	}
	cout << "ERROR" << "ganador: " << ticket_ganador << endl << "total Tickets" << tot_tickets << endl;

	// Hasta aca nunca deberia llegar
	// Por algun motivo silencia el warning "control reaches end of non-void
	// function" poner el abort()
	abort();
}
