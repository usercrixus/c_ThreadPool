#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <semaphore.h>
#include <stdbool.h>

// Task represent a function to call
typedef struct task{
    void (*function)(void *p);
    void *data;
    struct task* next;
}Task;

// ThreadPool represent a pool
typedef struct list{
    Task* head; // linked list of task
    pthread_mutex_t dequeueMutex; // mutex to handle the queue
    pthread_t* pool; // array of threads
    int size; // this of the array of threads
    pthread_cond_t cond; // stop and go condition
    bool active; // status of the pool (usable/unusable)
}ThreadPool ;

/**
 * @brief execute : call function to execute with params
 * @param p : params to the function
 */
void execute(void (*somefunction)(void *p), void *p);
/**
 * @brief pool_submit : add a task to the pool
 * @param p : data
 * @param threadPool : your threadPool
 * @return 0 if success
 */
int pool_submit(void (*somefunction)(void *p), void *p, ThreadPool* threadPool);
/**
 * @brief worker : thread(s) of the pool
 * @param param : ThreadPool structure
 * @return pthread_exit();
 */
void *worker(void *param);
/**
 * @brief pool_init : initialize ThreadPool
 * @param size : number of usable thread in the pool
 * @return the threadPool
 */
ThreadPool* pool_init(int size);
/**
 * @brief pool_shutdown : close the pool
 * @param threadPool : pool to close
 */
void pool_shutdown(ThreadPool* threadPool);
/**
 * @brief enqueue : push the pool queue with a task
 * @param t : the submited task
 * @param threadPool : pool to push
 * @return 0 if success
 */
int enqueue(Task* t, ThreadPool* threadPool);
/**
 * @brief dequeue : pop the list
 * @param threadPool : pool to pop
 * @return task to execute
 */
Task* dequeue(ThreadPool* threadPool);

#endif // THREADPOOL_H
