#ifndef __SCHED_MFQ__
#define __SCHED_MFQ__

#include <vector>
#include <queue>
#include "basesched.h"

class SchedMFQ : public SchedBase {
	public:
		SchedMFQ(std::vector<int> argn);
		virtual void load(int pid);
		virtual void unblock(int pid);
		virtual int tick(const enum Motivo m);
	
	private:
		std::vector<std::queue<int> > vq;
		std::vector<int> def_quantum;
		std::vector<int> unblock_to;
		int quantum, n, cur_pri;
		
		int next(void);
};

#endif
