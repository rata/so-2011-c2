#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "RWLock.h"

using namespace std;

void *rlock(void *dummy);
void *wlock(void *dummy);


RWLock rw;

int main()
{
	pthread_t t1;
	pthread_t t2;
	pthread_t t3;
	pthread_t t4;
	pthread_t t5;

	pthread_create(&t1, NULL, rlock, NULL);
	pthread_create(&t2, NULL, rlock, NULL);

	sleep(1);
	pthread_create(&t3, NULL, wlock, NULL);
	sleep(1);
	pthread_create(&t4, NULL, rlock, NULL);
	pthread_create(&t5, NULL, rlock, NULL);


	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	pthread_join(t4, NULL);
	pthread_join(t5, NULL);

	return 0;
}


void *rlock(void *dummy)
{
	
	rw.rlock();
	cout << "tengo el read lock" << endl;
	sleep(5);

	rw.runlock();
	cout << "libere read lock" << endl;

	return NULL;
}

void *wlock(void *dummy)
{
	
	cout << "esperando wlock" << endl;
	rw.wlock();
	cout << "tengo el wlock" << endl;
	sleep(5);

	rw.wunlock();

	cout << "libere wlock" << endl;

	return NULL;
}
