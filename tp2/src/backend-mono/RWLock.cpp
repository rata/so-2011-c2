#include "RWLock.h"

RWLock :: RWLock() {
    pthread_rwlock_init(&(this->rwlock),NULL);
}

// XXX: Falta el destroy! es un leak!

void RWLock :: rlock() {
    pthread_rwlock_rdlock(&(this->rwlock));
}
void RWLock :: wlock() {
    pthread_rwlock_wrlock(&(this->rwlock));
}
void RWLock :: runlock() {
    pthread_rwlock_unlock(&(this->rwlock));
}
void RWLock :: wunlock() {
    pthread_rwlock_unlock(&(this->rwlock));
}
