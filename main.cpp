#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>



class ThreadTarget
{
  public:
    ThreadTarget(pthread_mutex_t* mut);
    ~ThreadTarget();

    int testdata;

    /// Start the thread
    bool Start(void);

    /// POSIX id (handle) for the thread
    pthread_t threadId_;

    /// Function seen by POSIX as thread function
    static void* ThreadWrapper(void* data);

  private:
    /// Mutex reference
    pthread_mutex_t *mut_;

    int thread_loop_cnt;

    /// This function does the thread work
    void Thread(void);

};
ThreadTarget::~ThreadTarget()
{
    printf("ThreadTarget is deleted\n");
};

ThreadTarget::ThreadTarget(pthread_mutex_t* mut)
     :mut_(mut)

{
    testdata = 0,
    thread_loop_cnt = 0;
};

/// Function seen by POSIX as thread function
void* ThreadTarget::ThreadWrapper(void* data)
{
    printf("Hej\n");
    // data is a pointer to a Thread, so we can call the
    // real thread function
    (static_cast<ThreadTarget*>(data))->Thread();
    return 0;
}

/// This function does the thread work
void ThreadTarget::Thread(void)
{
    while(1)
    {


      pthread_mutex_lock(mut_);
      //... Do the update of shared memory here ....

      printf("Mutex is lock inside thread and now Update shared memory..\n");
      testdata++;
      printf("shared testdata increased to %d inside thread\n", testdata);
      pthread_mutex_unlock(mut_);
      thread_loop_cnt++;
      printf("thread_loop_cnt = %d\n", thread_loop_cnt);
      //printf("Thread set to sleep 200ms\n");
      usleep(200000);//Sleep inside this thread.
    }
}
bool ThreadTarget::Start(void)
{
   // Start the thread, send it the this pointer (points to this class
  // instance)

  //pthread_create() takes 4 arguments.
  //The first argument is a pointer to thread_id which is set by this function.
  //The second argument specifies attributes. If the value is NULL, then default attributes shall be used.
  //The third argument is name of function to be executed for the thread to be created.
  //The fourth argument is used to pass arguments to the function.

  return (pthread_create(&threadId_,    // Pointer to the thread handle
                         NULL,          // Optional ptr to thread settings
                         ThreadWrapper, // Thread function
                         this) == 0);   // Argument passed to thread func
}

int main(void)
{
    ThreadTarget *TargThread;
    pthread_mutex_t *mut;
    mut = new pthread_mutex_t;
    pthread_mutex_init(mut, NULL);
    TargThread = new ThreadTarget(mut);

    // Start the thread, send it the this pointer (points to this class
    // instance)
    TargThread->Start();
    int main_loop_cnt = 0;
    while(main_loop_cnt < 100)
    {
        if(pthread_mutex_trylock(mut) == 0)
        {
            TargThread->testdata--;
            pthread_mutex_unlock(mut);

            printf("shared testdata was decreased to %d at main\n", TargThread->testdata);
        }
        else
        {
            printf("Main program was locked out from decrease testdata by the increase thread\n");
        }

        main_loop_cnt++;
        printf("main_loop_cnt = %d\n", main_loop_cnt);

        printf("Main sleep 200ms\n");
        usleep(200000);//Sleep main thread.
        //printf("\033c");//Flushing the screen
    }

    delete TargThread;
    delete mut;
    return 1;
}


