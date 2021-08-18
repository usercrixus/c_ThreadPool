#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "threadpool.h"



int enqueue(Task* t, ThreadPool* threadPool)
{
    pthread_mutex_lock(&threadPool->dequeueMutex); // protected area

    if(threadPool->head == NULL){
        threadPool->head = t; // assign task to the pool
        pthread_cond_signal(&threadPool->cond); // the pool was empty so we have to relaunch (try to relaunch) it
    }else{
        Task* buff = threadPool->head;
        while(buff->next != NULL){
            buff = buff->next; // get pointer at the end of the pool
        }
        buff->next = t; // put task at the end of the pool
    }

    pthread_mutex_unlock(&threadPool->dequeueMutex); // end of protected area
    return 0;
}

Task* dequeue(ThreadPool* threadPool){
    pthread_mutex_lock(&threadPool->dequeueMutex); // protected area

    if(threadPool->head == NULL && threadPool->active == true){
        pthread_cond_wait(&threadPool->cond, &threadPool->dequeueMutex); // if pool is null we cant dequeue so we wait
    }
    if(threadPool->head == NULL && !threadPool->active == false){
        pthread_mutex_unlock(&threadPool->dequeueMutex); // if pool is close and null we return null as we broke the thread while loop condition
        return NULL;
    }

    Task* worktodo = threadPool->head;
    if(threadPool->head != NULL){
        threadPool->head = threadPool->head->next; // if all is right we dequeue
    }

    pthread_mutex_unlock(&threadPool->dequeueMutex); // end of protected area

    return worktodo; // return the head of queue.
}


void *worker(void *param){

    ThreadPool* threadPool = (ThreadPool*)param; // convert param to ThreadPool
    Task* worktodo; // this is the object who will be dequeue
    while((worktodo = dequeue(threadPool)) != NULL){ // if dequeue return null, pool_shutdown have been call and the pool is empty so, we leave the thread
        execute(worktodo->function, worktodo->data); // execute function with params
        free(worktodo); // avoid memory leaks
    }

    pthread_exit(0); // close thread
}
void execute(void (*somefunction)(void *p), void *p){
    (*somefunction)(p); // call ptr function with params
}

int pool_submit(void (*somefunction)(void *p), void *p, ThreadPool* threadPool){
    if(threadPool->active){ // we cant submit task after shutdown
        Task* t = malloc(sizeof (Task)); // create the task
        t->function = somefunction; // function ptr
        t->data = p; // data for the function
        t->next = NULL; // it will be push at the end of queue
        enqueue(t, threadPool); // push to the queue
    }

    return 0;
}

ThreadPool* pool_init(int size)
{
    ThreadPool* threadPool = malloc(sizeof(ThreadPool)); // create the pool
    threadPool->head = NULL; // init head to null
    threadPool->active = true; // make it active
    pthread_mutex_init(&threadPool->dequeueMutex, NULL); // init the queue mutex
    pthread_cond_init(&threadPool->cond, NULL); // init the stop and go condition

    pthread_attr_t attr; // attr to the threads
    pthread_attr_init(&attr);

    pthread_t* pool = malloc(sizeof(pthread_t)*size); // create pool of thread of desired size
    threadPool->pool = pool; // save the pool
    threadPool->size = size; // save the size

    for(int i = 0 ; i < size ; i++){
        pthread_create(&pool[i], &attr, worker, threadPool); // launch all thread
    }

    return threadPool; // return the pool
}

void pool_shutdown(ThreadPool* threadPool)
{
    threadPool->active = false; // close the pool
    pthread_cond_broadcast(&threadPool->cond); // deblock all threads
    for(int i = 0 ; i < threadPool->size ; i++){
        pthread_join(threadPool->pool[i], NULL); // join all threads
    }
    free(threadPool->pool); // prevent memory leaks
    free(threadPool); // prevent memory leaks

}
