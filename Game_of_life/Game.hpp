#ifndef __GAMERUN_H
#define __GAMERUN_H

#include "Headers.hpp"
#include "Thread.hpp"
#include "PCQueue.hpp"


#define DEAD  0
#define LIVE  1
class Game;
class Task;
class gameOfLife_Thread;
/*--------------------------------------------------------------------------------
								  Class Task
--------------------------------------------------------------------------------*/
class Task{

    int beginnig_row;
    int end_row;
    int current_generation;
    friend class gameOfLife_Thread;
public:
    Task(int __beginnig_row = 0 , int __end_row = 0,int _current_generation = 0);
    int get_beginnig_row()const;
    int get_end_row()const;
    friend ostream& operator<<(ostream& os, const Task& task);
};

/*--------------------------------------------------------------------------------
								  Class gameOfLife_Thread
--------------------------------------------------------------------------------*/
class gameOfLife_Thread : public Thread {
    Game& game;
public:
    gameOfLife_Thread(uint thread_id,Game& _game);
    void thread_workload() override;
};

/*--------------------------------------------------------------------------------
								  Auxiliary Structures
--------------------------------------------------------------------------------*/
struct game_params {
	// All here are derived from ARGV, the program's input parameters. 
	uint n_gen;
	uint n_thread;
	string filename;
	bool interactive_on; 
	bool print_on; 
};
/*--------------------------------------------------------------------------------
									Class Declaration
--------------------------------------------------------------------------------*/
class Game {
public:

	Game(game_params);
	~Game();
	void run(); // Runs the game
	const vector<float> gen_hist() const; // Returns the generation timing histogram  
	const vector<float> tile_hist() const; // Returns the tile timing histogram
	uint thread_num() const; //Returns the effective number of running threads = min(thread_num, field_height)

	friend class Task;
    friend class gameOfLife_Thread;

protected: // All members here are protected, instead of private for testing purposes

	// See Game.cpp for details on these three functions
	void _init_game(); 
	void _step(uint curr_gen); 
	void _destroy_game();


	uint m_gen_num; 			 // The number of generations to run
	uint m_thread_num; 			 // Effective number of threads = min(thread_num, field_height)
	vector<float> m_tile_hist; 	 // Shared Timing history for tiles: First m_thread_num cells are the calculation durations for tiles in generation 1 and so on.
							   	 // Note: In your implementation, all m_thread_num threads must write to this structure. 
	vector<float> m_gen_hist;  	 // Timing history for generations: x=m_gen_hist[t] iff generation t was calculated in x microseconds
	vector<Thread*> m_threadpool; // A storage container for your threads. This acts as the threadpool.

	bool interactive_on; // Controls interactive mode - that means, prints the board as an animation instead of a simple dump to STDOUT 
	bool print_on; // Allows the printing of the board. Turn this off when you are checking performance (Dry 3, last question)

	inline void print_board(const char* header);


	// variables and synchronization primitives
    uint rows;
    uint cols;
    vector<vector<int>> curr;
    vector<vector<int>> next;
    PCQueue<Task> Tasks_Queue;

    uint num_tasks_Done;
    pthread_mutex_t  m_mux;        			// Controls access.
    pthread_cond_t m_done_cond;		        // Controls run one step done , all threads do

};


#endif
