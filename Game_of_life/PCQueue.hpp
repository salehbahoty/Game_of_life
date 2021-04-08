#ifndef _QUEUEL_H
#define _QUEUEL_H
#include "Headers.hpp"
#include "Semaphore.hpp"

// Single Producer - Multiple Consumer queue
template <typename T>class PCQueue
{

public:

    explicit PCQueue() {

        //mutex
        pthread_mutexattr_init(&mutex_attr);
        pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
        pthread_mutex_init(&m_mux, &mutex_attr);

        readers_inside = 0;
        writers_inside = 0;
        writers_waiting = 0;

        //cond
        pthread_cond_init(&read_allowed, NULL);
        pthread_cond_init(&write_allowed, NULL);
    }

    ~PCQueue(){
        int error = pthread_mutex_destroy(&m_mux);
        if (error) printf("Semaphore m_waitcond destroy error\n");
        error = pthread_cond_destroy(&read_allowed);
        if (error) printf("Semaphore m_waitcond destroy error\n");
        error = pthread_cond_destroy(&write_allowed);
        if (error) printf("Semaphore m_waitcond destroy error\n");


    }


    // Blocks while queue is empty. When queue holds items, allows for a single
    // thread to enter and remove an item from the front of the queue and return it.
    // Assumes multiple consumers.
    T pop(){

        pthread_mutex_lock(&m_mux);
        T item;
        while ( pc_queue.empty()  || writers_inside + readers_inside > 0 || writers_waiting > 0) {
            pthread_cond_wait(&read_allowed, &m_mux);
        }
        readers_inside++;
        item = pc_queue.front();
        pc_queue.pop();
        readers_inside--;

        pthread_cond_signal(&read_allowed);
        pthread_cond_signal(&write_allowed);

        pthread_mutex_unlock(&m_mux);
        //unlock

        return item;
    }

    // Allows for producer to enter with *minimal delay* and push items to back of the queue.
    // Hint for *minimal delay* - Allow the consumers to delay the producer as little as possible.
    // Assumes single producer
    void push(const T& item){

        pthread_mutex_lock(&m_mux);


        writers_waiting++;
        while (writers_inside + readers_inside > 0) {
            pthread_cond_wait(&write_allowed, &m_mux);
        }
        writers_waiting--;

        writers_inside++;
        pc_queue.push(item);
        writers_inside--;

        pthread_cond_signal(&read_allowed);
        pthread_cond_signal(&write_allowed);

        pthread_mutex_unlock(&m_mux);
        //unlock

    }
    queue<T> pc_queue_get() const {
        return pc_queue;
    }


private:
    // Add your class memebers here
    pthread_mutexattr_t mutex_attr;			// locking aux field.
    pthread_mutex_t  m_mux;        			// Controls access.
    //pthread_cond_t m_waitcond;		        // Controls waiting and restart

    int readers_inside, writers_inside, writers_waiting;
    pthread_cond_t read_allowed;
    pthread_cond_t write_allowed;

    std::queue<T> pc_queue;
};
// Recommendation: Use the implementation of the std::queue for this exercise
#endif