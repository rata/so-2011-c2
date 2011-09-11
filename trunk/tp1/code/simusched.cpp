#include <vector>
#include "basetask.h"
#include "basesched.h"
#include "tasks.h"

using namespace std;

//void run_


int main(int argc, char* argv[]) {
	tasks_init();
	
	//vector<class TaskBase> ts();
	vector<ptskvi> ts = tasks_load("lote.tsk");
	// TaskCPU t1();
	
	
	
	return 0;
}
