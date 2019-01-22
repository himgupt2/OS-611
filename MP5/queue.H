/*
 File: queue.H
 
 Author: Himanshu Gupta
 Date  : October 24 2018
 
 */


#ifndef QUEUE_H
#define QUEUE_H


#include "thread.H"

class Queue {
    private:
        Thread*     thread;
        Queue*      next;

    public:
        Queue() {
            thread  = NULL;
            next    = NULL;
        }

        Queue(Thread * new_thread ){
            thread = new_thread;
            next   = NULL;
        }

    // Enter the given Thread in the end
    void enqueue (Thread * new_thread) {
        if (thread == NULL) {
            thread = new_thread;
        } else  {
            if (next == NULL) {
                next = new Queue(new_thread);
            } else {
                next->enqueue(new_thread);
            }
        }
    }

    //Return the refernce for the first available queue
    Thread *dequeue() {
        if (thread == NULL)
            return NULL;

        Thread * top_thread = thread;

        if (next != NULL) {
            //Update thread and next pointer
            thread              =  next->thread;
            Queue * stale_next  =  next;
            next                =  next->next;

            delete stale_next;
        } else {
            thread = NULL;
        }

        return top_thread;
    }
};


#endif

