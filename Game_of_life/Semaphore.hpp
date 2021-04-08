#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H
#include "Headers.hpp"

// Synchronization Warm up 
class Semaphore {
public:
	Semaphore(): Semaphore(0) {}; // Constructs a new semaphore with a counter of 0

	explicit Semaphore(unsigned val): m_value(val) {// Constructs a new semaphore with a counter of val
        pthread_mutexattr_init(&mutex_attr);
        pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
        pthread_cond_init(&m_waitcond, NULL);
        pthread_mutex_init(&m_mux, &mutex_attr);
        //printf("Semaphore Constructs SUCCESS\n");

	}

	~Semaphore(){
		int error;
		error = pthread_cond_destroy(&m_waitcond);
		if (error) printf("Semaphore m_waitcond destroy error\n");
		error = pthread_mutexattr_destroy(&mutex_attr);
		if (error) printf("mutex_attr destroy error\n");
        error = pthread_mutex_destroy(&m_mux);
        if (error) printf("Semaphore m_mux destroy error\n");
        //printf("Semaphore Destructs SUCCESS\n");
	}

	/* Semaphore up operation. */
	void up(){// Mark: 1 Thread has left the critical section2
		// Start a waiting thread if required.
		pthread_mutex_lock(&m_mux);
		m_value++;
		pthread_cond_signal(&m_waitcond);
        //printf("Semaphore up() SUCCESS\n");
        pthread_mutex_unlock(&m_mux);

	}

	/* Semaphore down operation. */
	void down() { // Block untill counter >0, and mark - One thread has entered the critical section.

	    pthread_mutex_lock(&m_mux);
		// Check the mutex value, and wait if need be.
		// Make us wait.  When we wait, the mutex is unlocked until thew ait ends.
		while (m_value <= 0) {
            pthread_cond_wait(&m_waitcond, &m_mux);
		}
		--m_value;

        //printf("Semaphore down() SUCCESS\n");
        pthread_mutex_unlock(&m_mux);
	}
	uint get_value()const {
		return m_value;
	}
    void set_value(int new_m_value){
		pthread_mutex_lock(&m_mux);
         m_value = new_m_value;
		pthread_mutex_unlock(&m_mux);
    }

private:
	int m_value;                    		// Value of semaphore.
	pthread_mutexattr_t mutex_attr;			// locking aux field.
	pthread_mutex_t  m_mux;        			// Controls access.
    pthread_cond_t m_waitcond;		        // Controls waiting and restart
};

#endif
