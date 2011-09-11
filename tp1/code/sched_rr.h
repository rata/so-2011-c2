#ifndef __SCHED_RR__
#define __SCHED_RR__

#include <vector>
#include <queue>
#include <cstdio>
#include "basesched.h"

class SchedRR : public SchedBase {
  public:
    SchedRR(std::vector<int> argn);
    virtual void load(int pid);
    virtual void unblock(int pid);
    virtual int tick(const enum Motivo m);

   private:
     int quantum;
     int contador;
     std::queue<int> q;

};

#endif
