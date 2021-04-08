#include "Game.hpp"
#include "utils.hpp"

/*--------------------------------------------------------------------------------
								  static auxiliary functions
--------------------------------------------------------------------------------*/
static void file_get_dims(string filename,uint dims[2]){
    vector<string> v,v1;
    v = utils::read_lines(filename);
    dims[0]=(uint)v.size();
    v1 = utils::split(v[0],' ');
    dims[1]=(uint)v1.size();
}

static int Cell_get_state(const vector<vector<int>>& curr,int x,int y ) {
    if ((uint)x >= curr.size() || x < 0 || (uint)y >= curr[0].size() || y < 0)
        return DEAD;
    return curr[x][y];
}


static void Cell_update(const vector<vector<int>>& curr,vector<vector<int>>& next,int x,int y ){
    int LiveCnt=0;

    LiveCnt+=Cell_get_state(curr,x-1,y-1);
    LiveCnt+=Cell_get_state(curr,x-1,y);
    LiveCnt+=Cell_get_state(curr,x-1,y+1);
    LiveCnt+=Cell_get_state(curr,x,y-1);
    LiveCnt+=Cell_get_state(curr,x,y+1);
    LiveCnt+=Cell_get_state(curr,x+1,y-1);
    LiveCnt+=Cell_get_state(curr,x+1,y);
    LiveCnt+=Cell_get_state(curr,x+1,y+1);

    //rule 1:   A dead cell with exactly three live neighbors becomes a live cell (birth).
    if (curr[x][y] == DEAD && LiveCnt == 3)
    {
        next[x][y]=LIVE;
        return;
    }
    //rule 2:	A live cell with two or three live neighbors stays alive (survival).
    if (curr[x][y]==LIVE && (LiveCnt==2 || LiveCnt==3))//rule 2
    {
        next[x][y]=LIVE;
        return;
    }
    //In all other cases, a cell dies or remains dead (overcrowding or loneliness).
    next[x][y]=DEAD;

}

/*--------------------------------------------------------------------------------
								  Class Task
--------------------------------------------------------------------------------*/
Task::Task(int __beginnig_row , int __end_row,int _current_generation)
        :beginnig_row(__beginnig_row),end_row(__end_row),current_generation(_current_generation){}

int Task::get_beginnig_row()const {
    return beginnig_row;
}
int Task::get_end_row()const {
    return end_row;
}
ostream& operator <<(ostream& os, const Task& task){
    os << "Task (" <<task.get_beginnig_row() << "," << task.get_end_row() << ")";
    return os;
}

/*--------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------
 *                          Class gameOfLife_Thread
 * -------------------------------------------------------------------------------
 * -------------------------------------------------------------------------------*/
gameOfLife_Thread::gameOfLife_Thread(uint thread_id,Game& _game):Thread(thread_id),game(_game){}


void gameOfLife_Thread::thread_workload(){
    while(1) {
        Task task_TODO = this->game.Tasks_Queue.pop();
        auto task_start_time = std::chrono::system_clock::now();
        int cols_number =  this->game.cols;
        for (int i = task_TODO.get_beginnig_row(); i < task_TODO.get_end_row(); ++i) {
            for (int j = 0; j < cols_number; ++j) {
                Cell_update( this->game.curr,  this->game.next, i, j);
            }
        }
        auto task_end_time = std::chrono::system_clock::now();

        pthread_mutex_lock(&game.m_mux);
            this->game.m_tile_hist.push_back((float) std::chrono::duration_cast<std::chrono::microseconds>(task_end_time - task_start_time).count());
            game.num_tasks_Done++;
            if(game.num_tasks_Done == game.thread_num()) {
                pthread_cond_signal(&game.m_done_cond);
            }
        pthread_mutex_unlock(&game.m_mux);

        if((uint)(task_TODO.current_generation) >= this->game.m_gen_num-1) {
            break;
        }
    }

}

/*--------------------------------------------------------------------------------
								  Class Game
--------------------------------------------------------------------------------*/
Game::Game(game_params gameParams){


    pthread_cond_init(&m_done_cond, NULL);
    pthread_mutex_init(&m_mux, NULL);
    num_tasks_Done=0;
    m_gen_num=gameParams.n_gen;
    m_thread_num=gameParams.n_thread;
    interactive_on=gameParams.interactive_on;
    print_on=gameParams.print_on;

    uint dims[2];
    file_get_dims(gameParams.filename,dims);
    rows=dims[0];
    cols=dims[1];


    curr.resize(rows);
    for (uint k = 0; k < rows ; ++k) {
        curr[k].resize(cols);
    }
    next.resize(rows);
    for (uint k = 0; k < rows ; ++k) {
        next[k].resize(cols);
    }

    int i=0,j=0;
    vector<string> v,v1;
    v = utils::read_lines(gameParams.filename);
    for(auto line : v){
        v1 = utils::split(line,' ');
        for(auto str : v1){
            int x=std::stoi( str );
            (curr.at(i)).at(j)=x;
            (next.at(i)).at(j)=x;
            j++;
        }
        i++;j=0;
    }
}

Game::~Game(){}//DO NOTHING
const vector<float> Game::gen_hist() const{
    return m_gen_hist;
}
const vector<float> Game::tile_hist() const{
    return m_tile_hist;
}
uint Game::thread_num() const{//Returns the effective number of running threads = min(thread_num, field_height)
    return m_thread_num < rows ? m_thread_num : rows;
}



/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/
void Game::run() {

    _init_game(); // Starts the threads and all other variables you need
    print_board("Initial Board");
    for (uint i = 0; i < m_gen_num; ++i) {
        auto gen_start = std::chrono::system_clock::now();
        _step(i); // Iterates a single generation
        auto gen_end = std::chrono::system_clock::now();
        m_gen_hist.push_back((float) std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
        print_board(NULL);
    } // generation loop
    print_board("Final Board");
    _destroy_game();
}

void Game::_init_game() {

    // Create threads
    for (uint k = 0; k < thread_num() ; ++k) {
        m_threadpool.push_back(new gameOfLife_Thread(k,*this));
    }
    // Start the threads
    for (uint k = 0; k < thread_num() ; ++k) {
        gameOfLife_Thread* thread= static_cast<gameOfLife_Thread*>(m_threadpool[k]);
        bool res=thread->start();
        assert(res);
    }

}

static void curr_and_next_Swap (vector<vector<int>> *curr, vector<vector<int>> *next){
    vector<vector<int>>* temp = curr;
    *curr = *next;
    *next = *temp;
}

void Game::_step(uint curr_gen) {
    num_tasks_Done=0;
    uint effective_threads=thread_num();
    int task_rows=rows/effective_threads;
    // Push jobs to queue
    for (uint i = 0; i < rows; i+=task_rows) {
        bool last_partion=(i/task_rows >= effective_threads-1);
        int end=(last_partion) ? rows : i+task_rows ;
        Task task=Task(i,end,curr_gen);
        Tasks_Queue.push(task);
        if (i/task_rows >= effective_threads - 1 )
            break;
    }
    // Wait for the workers to finish calculating
    pthread_mutex_lock(&m_mux);
    while (num_tasks_Done != effective_threads){
        pthread_cond_wait(&m_done_cond, &m_mux);
    }
    pthread_mutex_unlock(&m_mux);
    // Swap pointers between current and next field
    curr_and_next_Swap(&curr,&next);
}

void Game::_destroy_game(){
    // Destroys board and frees all threads and resources
    // Testing of your implementation will presume all threads are joined here
    for (uint k = 0; k < thread_num() ; ++k) {
        m_threadpool[k]->join();
    }
    for (auto thread : m_threadpool) {
        delete thread;
    }
}

/*--------------------------------------------------------------------------------

--------------------------------------------------------------------------------*/
inline void Game::print_board(const char* header){
    if(print_on){

        /* Clear the screen, to create a running animation*/
        if(interactive_on)
            system("clear");

        // Print small header if needed
        if (header != NULL)
            cout << "<------------" << header << "------------>" << endl;

        // TODO: Print the board
        cout << u8"╔" << string(u8"═") * cols << u8"╗" << endl;
        for (uint i = 0; i < rows ; ++i) {
            cout << u8"║";
            for (uint j = 0; j < cols; ++j) {
                cout << (curr[i][j] ? u8"█" : u8"░");
            }
            cout << u8"║" << endl;
        }
        cout << u8"╚" << string(u8"═") * cols << u8"╝" << endl;
        // Display for GEN_SLEEP_USEC micro-seconds on screen
        if(interactive_on)
            usleep(GEN_SLEEP_USEC);
    }

}




