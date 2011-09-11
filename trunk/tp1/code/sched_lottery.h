#ifndef __SCHED_LOTTERY__
#define __SCHED_LOTTERY__

#include <vector>
#include "basesched.h"

class SchedLottery : public SchedBase {
  public:
    SchedLottery(std::vector<int> argn);
    virtual void load(int pid);
    virtual void unblock(int pid);
    virtual int tick(const enum Motivo m);

  private:
};

#endif

