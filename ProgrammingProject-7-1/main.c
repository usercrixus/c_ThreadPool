#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"

struct data
{
    int a;
    int b;
};

void add(void *param){
    struct data *temp;
    temp = (struct data*)param;

    printf("I add two values %d and %d result = %d\n",temp->a, temp->b, temp->a + temp->b);
}

int main(void){
    // create some work to do
    struct data work;
    work.a = 5;
    work.b = 10;


    // initialize the thread pool
    ThreadPool* threadPool = pool_init(3);
    sleep(1); // test with & without

    // submit the work to the queue
    pool_submit(&add, &work, threadPool);
    pool_submit(&add, &work, threadPool);
    pool_submit(&add, &work, threadPool);
    sleep(1); // test with & without
    pool_submit(&add, &work, threadPool);
    pool_submit(&add, &work, threadPool);
    pool_submit(&add, &work, threadPool);
    sleep(1); // test with & without

    // close the pool
    pool_shutdown(threadPool);

    return 0;
}
