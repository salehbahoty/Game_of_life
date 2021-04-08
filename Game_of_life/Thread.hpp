#ifndef __THREAD_H
#define __THREAD_H
#include "Headers.hpp"
class Thread
{
public:
	Thread(uint thread_id) :m_thread_id(thread_id)
	{
		// Only places thread_id 
	} 
	virtual ~Thread() {} // Does nothing 

	/** Returns true if the thread was successfully started, false if there was an error starting the thread */
	bool start()
	{
		int res=pthread_create(&(this->m_thread), NULL, entry_func, this);
		assert(res==0);
		return res==0;//pthread_create is successed
	}

	/** Will not return until the internal thread has exited. */
	void join()
	{
        pthread_join((this->m_thread),NULL);

	}

	/** Returns the thread_id **/
	uint thread_id()
	{
		return m_thread_id;
	}
protected:
	/** Implement this method in your subclass with the code you want your thread to run. */
	virtual void thread_workload() = 0;
    /*virtual void thread_workload(){
        cout<<"ERROR: The Thread running the wrong thread_workload() function";
    }*/
	uint m_thread_id; // A number from 0 -> Number of threads initialized, providing a simple numbering for you to use

private:
	//static void * entry_func(void * thread) { ((Thread *)thread)->thread_workload(); return NULL; }
    //static void * entry_func2(void * thread) { /*cout<<"entry_func2";*/ return NULL; }

    static void * entry_func(void * thread) {
        Thread* th=(Thread*)thread;
        th->thread_workload();
        return NULL;
    }
	pthread_t m_thread;
};


#endif
