/*
 File: scheduler.C
 
 Author: Himanshu Gupta
 Date  : October 24 2018
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
    queue_size = 0;
    Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
    if (queue_size == 0) {
        Console::puts("No new thread available, can not yield \n");
    } else {
        queue_size--;
        Thread* new_thread = ready_queue.dequeue();
        Thread::dispatch_to(new_thread);
    }
}

void Scheduler::resume(Thread * _thread) {
    ready_queue.enqueue(_thread);
    queue_size++;
}

void Scheduler::add(Thread * _thread) {
    ready_queue.enqueue(_thread);
    queue_size++;
}

void Scheduler::terminate(Thread * _thread) {
    for (int i = 0; i < queue_size; i++) {
        Thread * top_thread = ready_queue.dequeue();

        if (_thread->ThreadId() == top_thread->ThreadId()) {
            queue_size--;
        } else {
            ready_queue.enqueue(top_thread);
        }
    }
}
