#include "RWLock.h"
#include <assert.h>


using namespace std;

// NOTA: se ignoran los return code de muchas de las llamadas porque la API no
// permite devolver error y en la implmentancion con RWLocks posix se los
// ignoraba tambien

RWLock :: RWLock()
{
	pthread_mutex_init(&q_lock, NULL);
	q_in_use = false;
	readers = 0;
}

RWLock :: ~RWLock()
{
	// XXX: cuando se destruye el lock, no deberia haber ningun usuario
	// Eso es: nadie encolado, ni ningun reader ni la cola deberia estar en
	// uso (pues implica que un writer tiene el lock)
	assert(q.empty());
	assert(readers == 0);
	assert(!q_in_use);
	pthread_mutex_destroy(&q_lock);
}

void RWLock :: rlock()
{
	pthread_mutex_lock(&q_lock);

	// Si esta en uso la cola, encolamos el read
	if (q_in_use) {
		sem_t *sem = init_q_sem();
		encolar_reader(sem);
		pthread_mutex_unlock(&q_lock);
		sem_wait(sem);

		// Es nuestro turno, me saco de la cola y me fijo si queda
		// vacia. Si queda vacia, no se usa mas la cola. Y si no esta
		// vacia, y el que viene es un reader, lo despierto tambien
		pthread_mutex_lock(&q_lock);
		desencolar_tope();

		destroy_q_sem(sem);

		if (q.empty())
			q_in_use = false;
		else
			despertar_si_reader();
	}

	readers++;
	pthread_mutex_unlock(&q_lock);
}

void RWLock :: wlock()
{
	pthread_mutex_lock(&q_lock);

	// Si ya se esta usando la cola o si hay algun lector, el wlock se debe
	// encolar
	if (q_in_use || readers > 0) {

		// Cualquier pedido que llegue mientras hacemos el wait, debe
		// encolarse (para evitar inanicion). Asique marcamos la cola
		// como en uso antes de hacer el wait
		q_in_use = true;
		sem_t *sem = init_q_sem();
		encolar_writer(sem);
		pthread_mutex_unlock(&q_lock);
		sem_wait(sem);
		pthread_mutex_lock(&q_lock);
		desencolar_tope();
		destroy_q_sem(sem);
	}

	// Cuando un writer tiene el lock, todo lo que venga se debe encolar.
	// Por lo que la cola esta en uso
	q_in_use = true;
	pthread_mutex_unlock(&q_lock);
}

void RWLock :: runlock()
{
	pthread_mutex_lock(&q_lock);

	readers--;

	if (readers == 0)
		despertar_prox();

	pthread_mutex_unlock(&q_lock);
}

void RWLock :: wunlock()
{
	pthread_mutex_lock(&q_lock);

	if (q.empty())
		q_in_use = false;
	else
		despertar_prox();

	pthread_mutex_unlock(&q_lock);
}


sem_t *RWLock :: init_q_sem()
{
	sem_t *sem = new sem_t;

	// Inicializamos el semaforo para que se comparta entre threads (el
	// primer 0) y el valor del semaforo es 0 (esta esperando a que alguien
	// lo "despierte")
	sem_init(sem, 0, 0);

	return sem;
}


void RWLock :: destroy_q_sem(sem_t *sem)
{
	sem_destroy(sem);
	delete sem;
}


// Todas los helpers de la cola asumen que el q_lock esta tomado
void RWLock :: encolar_reader(sem_t *sem)
{
	pair<sem_t *, RWLock_queue_t>  p(sem, RWL_READER);
	q.push(p);
}

void RWLock :: encolar_writer(sem_t *sem)
{
	pair<sem_t *, RWLock_queue_t>  p(sem, RWL_WRITER);
	q.push(p);
}

void RWLock :: despertar_prox(void)
{
	if (q.empty())
		return;

	pair<sem_t *, RWLock_queue_t>  p = q.front();

	// Despierto al proximo de la cola
	sem_post(p.first);
}

void RWLock :: despertar_si_reader(void)
{
	if (q.empty())
		return;

	pair<sem_t *, RWLock_queue_t>  p = q.front();

	if (p.second == RWL_READER) {
		// Despierto al proximo de la cola
		sem_post(p.first);
	}
}

void RWLock :: desencolar_tope(void)
{
	if (q.empty())
		return;

	q.pop();
}
