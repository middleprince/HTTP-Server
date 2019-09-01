#ifndef MINISERVER_LOCKER_H
#define MINISERVER_LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

namespace miniserver {

class Semaphore {
public:
    Semaphore();
    ~Semaphore();

    bool semInit();
    bool SemDestroy();

    bool wait(); // p
    bool post(); // V
    
private:
    sem_t _semaphore;

    Semaphore(const Semaphore &other);
};

class Locker {
public:
    Locker();
    ~Locker();

    bool lockerInit();
    bool lockerDestroy();

    bool lock();
    bool unlock();

private:
    pthread_mutex_t _mutex;
    
    Locker(const Locker &other);
    Locker& operator=(const Locker &rhs);
};

class Cond {
public:
    Cond();
    ~Cond();
    
    bool condInit();
    bool CondDestroy();

    bool wait();
    bool signal();

private:
    pthread_mutex_t _mutex; 
    pthread_cond_t _cond;

    Cond(const Cond& other);
    Cond& operator=(const Cond& rhs);
};

}
#endif
