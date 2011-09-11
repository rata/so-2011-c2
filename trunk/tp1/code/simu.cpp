#include <vector>
#include <pthread.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <queue>
#include "simu.h"

#include "basetask.h"
#include "basesched.h"

using namespace std;

// #define DBG_THREAD

#ifdef DBG_THREAD
#define _D(X) X
#else
#define _D(X)
#endif

#define forn(i,n) for(int i = 0; i < (int)(n); ++i)
#define DBG(X) _D(cerr << #X << " = " << X << endl;)

/* "Globales" */
static int cur_pid;
static unsigned int cur_time;
static pthread_mutex_t m_sched;

enum status_t {ST_EXIT, ST_IO, ST_CPU};

struct task_data {
	pthread_t tid;
	int pid, running;
	status_t blk;
	int blkms;
	TaskBase* tsk;
	vector<int>* prms;
	pthread_mutex_t mutex;
};
static task_data* tsks;

void* task_thread(task_data* tsk) {
	pthread_mutex_lock(&tsk->mutex);
	//fprintf(stderr, "Thread %d\n", tsk->pid);
	
	tsk->tsk(*tsk->prms); // Run!
	// 
	tsk->blk = ST_EXIT;
	_D(cerr << "TSK unlock(sched) exit" << endl;) pthread_mutex_unlock(&m_sched);
	return NULL;
}


/* Funciones llamadas por el scheduler */
int current_pid(void) { return cur_pid; }
unsigned int current_time(void) { return cur_time; }

/* Funciones llamadas por las tareas */
static void uso_X(task_data* tsk, enum status_t tp, unsigned int ms) {
	if (ms == 0) return;
	tsk->blk = tp;
	tsk->blkms = ms;
	_D(cerr << "TSK unlock(sched)" << endl;) pthread_mutex_unlock(&m_sched);
	pthread_mutex_lock(&tsk->mutex); _D(cerr << "TSK lock(mutex["<<tsk-tsks<<")" << endl;)
}

void uso_CPU(unsigned int ms) {
	uso_X(&(tsks[cur_pid]), ST_CPU, ms);
}

void uso_IO(unsigned int ms) {
	uso_X(&(tsks[cur_pid]), ST_IO, ms);
}


void simulate(SchedBase& sch, std::vector<ptsk>& lote, const Settings& settings) {
	int n = lote.size();
	
	cout.flush();
	if (settings.output_log != "-") {
		/* Oh yeah! */
		freopen(settings.output_log.c_str(), "wt", stdout);
	}
	
	tsks = (task_data*)malloc(sizeof(task_data)*n);
	
	if (!tsks) { perror("malloc(task_data)"); return; }
	forn(i, n) {
		tsks[i].pid = i;
		pthread_mutex_init(&(tsks[i].mutex), NULL);
		pthread_mutex_lock(&(tsks[i].mutex));
		tsks[i].tsk = lote[i].tsk;
		tsks[i].prms = &(lote[i].prms);
		tsks[i].running = 0;
		tsks[i].blk = ST_CPU;
		tsks[i].blkms = 0;
		if (pthread_create(&(tsks[i].tid), NULL, (void*(*)(void*))task_thread, (void*)(&(tsks[i]))) < 0) {
			perror("Lanzando la tarea (no use muchas tareas, < 500)"); return;
		}
	}
	pthread_mutex_init(&m_sched, NULL);
	pthread_mutex_lock(&m_sched);
	int finished = 0;
	cur_pid = IDLE_TASK;
	cur_time = 0;
	
	priority_queue<pair<int, int> > load;
	forn(i, n) load.push(make_pair(-lote[i].start, -i));
	
	priority_queue<pair<int, int> > unblock;
	
	int context_remain = 0; /* Remaining context_switch ticks */
	
	while (finished < n || context_remain) {
		if (settings.verbose) {
			cerr << "--- sched, tm=" << cur_time << " pid=" << cur_pid;
			if (cur_pid != IDLE_TASK) { cerr << " [" << cur_pid << " ST:"<< tsks[cur_pid].blk << " ms:" << tsks[cur_pid].blkms << "]"; }
			cerr << endl;
		}

		// Load de las tareas
		while (!load.empty() && load.top().first >= -(int)cur_time) {
			int pid = -load.top().second; load.pop();
			tsks[pid].running = 1;
			sch.load(pid);
			cout << "LOAD " << cur_time << " " << pid << endl; 
		}

		vector<int> to_unblock;
		while (!unblock.empty() && unblock.top().first >= -(int)cur_time) {
			int pid = -unblock.top().second; unblock.pop();
			_D(cerr << "SCH unblock(" << pid << ")" << endl;)
			sch.unblock(pid); // pid
			to_unblock.push_back(pid);
			if (cur_pid == pid) {
				tsks[pid].blkms = -2;
			} else {
				tsks[pid].blk = ST_CPU;
				tsks[pid].blkms = 0;
			}
		}

		if (!context_remain) {
		int npid;
		if (cur_pid == IDLE_TASK) {
			npid = sch.tick(TICK);
			_D(cerr << "SCH tick(TICK) -> " << npid << endl;)
		} else {
			if (tsks[cur_pid].blk == ST_CPU && !tsks[cur_pid].blkms) {
				_D(cerr << "SCH unlock(" << cur_pid << ") tick" << endl;) pthread_mutex_unlock(&(tsks[cur_pid].mutex));
				pthread_mutex_lock(&m_sched); _D(cerr << "SCH lock(sched)" << endl;)
			}
			switch (tsks[cur_pid].blk) {
				case ST_EXIT:
					finished++;
					npid = sch.tick(EXIT);
					cout << "EXIT " << cur_time << " " << cur_pid << endl; 
					_D(cerr << "SCH tick(EXIT) -> " << npid << endl;)
					tsks[cur_pid].running = 0;
					break;
				case ST_IO:
					if (tsks[cur_pid].blkms >= 0) {
						unblock.push(make_pair(-(cur_time+tsks[cur_pid].blkms), -cur_pid));
						tsks[cur_pid].blkms = -1;
					}
					if (tsks[cur_pid].blkms == -2) {
						tsks[cur_pid].blk = ST_CPU;
						tsks[cur_pid].blkms = 0;
					}
					cout << "BLOCK " << cur_time << " " << cur_pid << endl; 
					npid = sch.tick(BLOCK);
					_D(cerr << "SCH tick(BLOCK) -> " << npid << endl;)
					break;
				case ST_CPU:
					if (tsks[cur_pid].blkms) {
						tsks[cur_pid].blkms--;
					} else {
						cerr << "FATAL ERROR, this should not happend" << endl;
					}
					npid = sch.tick(TICK);
					_D(cerr << "SCH tick(TICK) -> " << npid << endl;)
					break;
			}
		}
		if (npid == IDLE_TASK) {
			// cerr << "SCH unlock(sched)" << endl;pthread_mutex_unlock(&m_sched);
		} else {
			if (npid < 0 || npid >= n) { cerr << "Error!, scheduler sent an invalid pid="<<npid<< endl; return; }
			if (!tsks[npid].running) { cerr << "Error!, scheduler sent pid="<<npid << " but that process has exited." << endl; return; }
			if (!tsks[npid].blk == ST_IO) { cerr << "Error!, scheduler sent pid="<<npid << " but that process is still blocked." << endl; return; }
		}
		if (settings.switch_cost > 0) {
			if (cur_pid != npid) {
				context_remain += settings.switch_cost;
			}
		}
		cur_pid = npid;
		} else {
			context_remain--;
		}
		/* Unblock tasks at the end of the tick */
		for(int j=0; j<(int)to_unblock.size(); j++) cout << "UNBLOCK " << cur_time << " " << to_unblock[j] << endl;
		
		if (context_remain) {
			cout << "# CONTEXT " << cur_time << endl;
		} else {
			cout << "CPU " << cur_time << " " << cur_pid << endl;
		}
		cur_time++;
	}
	
	free(tsks);
}
