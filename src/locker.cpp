#include "../include/locker.h"
#include "../include/easylogging++.h"

namespace miniserver {

Semaphore::Semaphore() : _sem() {};
Semaphore::~Semaphore() = default;

bool Semaphore::semInit() {
    if (::sem_init(&_sem, 0, 0) == -1) {
        LOG(WARNING) << "[Semaphore::semInit]: sem_init failed";
        return false;
    }
    return true;
}

bool Semaphore::semDestroy() {
    if (::sem_destory(&_sem) == -1) {
        LOG(WARNING) << "[Semaphore::SemDestroy]: sem_init failed";
        return false;
    }
    return true;
}

bool Semaphore::wait() {
    if (::sem_wait(&_sem) == -1) {
        LOG(WARNING) << "[Semaphore::wait]: sem_wait failed";
        return false;
    }
    return true;
}

bool Semaphore::post() {
    if (::sem_post(&_sem) == -1) {
        LOG(WARNING) << "[Semaphore::post]: sem_post failed";
        return false;
    }
    return true;
}

Locker::Locker() : _mutex() {};
Locker ::~Locker() = default;

bool Locker::lockerInit() {
    if (pthread_mutex_init(&_mutex, nullptr) != 0) {
        LOG(WARNING) << "[Locker::lockerInit]: pthread_mutex_init failed";
        return false;
    }
    return true;
}

bool Locker::lockerDestroy() {
    if (pthread_mutex_destroy(&_mutex) != 0) {
        LOG(WARNING) << "[Locker::lockerDestroy]: pthread_mutex_destroy failed";
        return false;
    }
    return true;
}

bool Locker::lock() {
    if (pthread_mutex_lock(&_mutex) != 0) {
        LOG(WARNING) << "[Locker::locke]: pthread_mutex_lock failed";
        return false;
    }
    return true;
}

bool Locker::unlock() {
    if (pthread_mutex_unlock(&_mutex) != 0) {
        LOG(WARNING) << "[Locker::unlock]: pthread_mutex_unlock failed";
        return false;
    }
    return true;
}


Cond::Cond() : _mutex(), _cond() {};
Cond::~Cond() = default;

bool Cond::condInit() {
    if (pthread_mutex_init(&_mutex, nullptr) != 0) {
        LOG(WARNING) << "[Locker::conInit]: pthread_mutex_init failed";
        return false;
    }
    return true;
}

bool Cond::condDestroy() {
    if (pthread_mutex_destroy(&_mutex) != 0) {
        LOG(WARNING) << "[Locker::condDestroy]: pthread_mutex_destroy failed";
        return false;
    }
    if (pthread_cond_destroy(&_cond) != 0) {
        LOG(WARNING) << "[Locker::condDestroy]: pthread_mutex_destroy failed";
        return false;
    }
    return true;
}

bool Cond::wait() {
    if (pthread_mutex_lock(&_mutex)) {
        LOG(WARNING) << "[Locker::wait]: pthread_mutex_unlock failed";
        return false;
    }
    if (pthread_cond_wait(&_cond, &_mutex) != 0) {
        LOG(WARNING) << "[Locker::wait]: pthread_cond_wait failed";
        return false;
    }
    if (pthread_mutex_unlock( &_mutex) != 0) {
        LOG(WARNING) << "[Locker::wait]: pthread_mutex_unlock failed";
        return false;
    }
    return true;
}

bool Cond::signal() { 
    if (pthread_cond_signal(&_cond) != 0) {
        LOG(WARNING) << "[Locker::signal]: pthread_cond_signal failed";
        return false;
    }
    return true;
}


} // namespace miniserver

