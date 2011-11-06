#ifndef RWLock_h
#define RWLock_h
#include <queue>	// queue
#include <utility>	// pair
#include <pthread.h>	// mutex
#include <semaphore.h>	// sem

enum RWLock_queue_t {
	RWL_READER,
	RWL_WRITER,
};

class RWLock {
    public:
        RWLock();
	~RWLock();
        void rlock();
        void wlock();
        void runlock();
        void wunlock();

    private:
	// This lock protects variables: q, q_in_use, readers
	pthread_mutex_t q_lock;
	std::queue<std::pair<sem_t *, RWLock_queue_t> > q;
	bool q_in_use;
	unsigned int readers;

	// Inicializa y destruye un semaforo para usar en la cola q
	sem_t *init_q_sem();
	void destroy_q_sem(sem_t *sem);

	// funciones "common" para manejar la cola
	// XXX: Todas asumen que q_lock esta tomado
	void encolar_reader(sem_t *sem);
	void encolar_writer(sem_t *sem);
	void despertar_prox(void);
	void despertar_si_reader(void);
	void desencolar_tope(void);
};

#endif
