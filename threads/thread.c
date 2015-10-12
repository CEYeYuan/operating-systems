#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include "thread.h"
#include "interrupt.h"

/* This is the thread control block */

enum state{ 
	running=1,
	ready=2,
	blocked=3
};



//information about a particular thread
typedef struct thread {
	/* a linked list of thread */
	ucontext_t *mycontext;
	enum state status;
	Tid id;
	struct thread *next;
} thread;

struct list {
	/* you can define this struct to have whatever fields you want. */
	struct thread *head;
	int size;
};

int 
thread_compare(struct thread *a,struct thread *b){
	//compare the id of two thread; no duplicate id are allowed
	if(a->id>b->id)
		return 1;
	else
		return -1;
}

void
add_thread(struct list *readyqueue, struct thread *thread)
{
	if(readyqueue->size==0){
		readyqueue->head=malloc(sizeof(struct thread));
		readyqueue->head=thread;
	}
	else{
		struct 	thread *head,*tmp;
		head=readyqueue->head;
		while(head->next!=NULL&&thread_compare(head,thread)<0){
			head=head->next;
		}
		if(head->next==NULL)
			head->next=thread;
		else{
			tmp=head->next;
			head->next=thread;
			thread->next=tmp;
		}
	}
	readyqueue->size+=1;
}


struct thread *current;
struct list *readyQueue;
int *arr;//mark the # of threads, 0 for not used, 1 for used
void
thread_init(void)
{
	/* your optional code here */
	current=malloc(sizeof(struct thread));
	readyQueue=malloc(sizeof(struct list));
	current->mycontext=malloc(sizeof(ucontext_t));
	getcontext(current->mycontext);
	current->id=0;
	current->status=running;
	current->next=NULL;//this field should never get used.
	readyQueue=malloc(sizeof (struct list));
	readyQueue->size=0;
	arr=malloc(sizeof(int)*THREAD_MAX_THREADS);
	int i=0;
	while(i<THREAD_MAX_THREADS){
		arr[i]=0;
		i++;
	}
}

Tid
thread_id()
{
/* return the thread identifier of the currently running thread 
The first thread in the system (before the first call to thread_create) should have a 
thread identifier of 0. Your threads system should support the creation of a maximum of 
THREAD_MAX_THREADS concurrent threads by a program (including the initial main thread). 
Thus, the maximum value of the thread identifier should thus be THREAD_MAX_THREADS - 1 
(since thread identifiers start from 0). Note that when a thread exits, its thread 
identifier can be reused by another thread created later.*/

	if(current!=NULL)
		return current->id;
	return THREAD_INVALID;
}

Tid
thread_create(void (*fn) (void *), void *parg)
{
	TBD();
	return THREAD_FAILED;
}

Tid
thread_yield(Tid want_tid)
{
/* suspend calling thread and run the thread with identifier tid. The calling
 * thread is put in the ready queue. tid can be identifier of any available
 * thread or the following constants:
 *
 * THREAD_ANY:	   run any thread in the ready queue.
 * THREAD_SELF:    continue executing calling thread, for debugging purposes.
 *
 * Upon success, return the identifier of the thread that ran. The calling
 * thread does not see this result until it runs later. Upon failure, the
 * calling thread continues running, and returns the following:
 *
 * THREAD_INVALID: identifier tid does not correspond to a valid thread.
 * THREAD_NONE:    no more threads, other than the caller, are available to
 *		   run. this can happen is response to a call with tid set to
 *		   THREAD_ANY. */
	//TBD();
	return THREAD_FAILED;
}

Tid
thread_exit(Tid tid)
{
	TBD();
	return THREAD_FAILED;
}

/*******************************************************************
 * Important: The rest of the code should be implemented in Lab 3. *
 *******************************************************************/

/* This is the wait queue structure */
struct wait_queue {
	/* ... Fill this in ... */
};

struct wait_queue *
wait_queue_create()
{
	struct wait_queue *wq;

	wq = malloc(sizeof(struct wait_queue));
	assert(wq);

	TBD();

	return wq;
}

void
wait_queue_destroy(struct wait_queue *wq)
{
	TBD();
	free(wq);
}

Tid
thread_sleep(struct wait_queue *queue)
{
	TBD();
	return THREAD_FAILED;
}

/* when the 'all' parameter is 1, wakeup all threads waiting in the queue.
 * returns whether a thread was woken up on not. */
int
thread_wakeup(struct wait_queue *queue, int all)
{
	TBD();
	return 0;
}

struct lock {
	/* ... Fill this in ... */
};

struct lock *
lock_create()
{
	struct lock *lock;

	lock = malloc(sizeof(struct lock));
	assert(lock);

	TBD();

	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	TBD();

	free(lock);
}

void
lock_acquire(struct lock *lock)
{
	assert(lock != NULL);

	TBD();
}

void
lock_release(struct lock *lock)
{
	assert(lock != NULL);

	TBD();
}

struct cv {
	/* ... Fill this in ... */
};

struct cv *
cv_create()
{
	struct cv *cv;

	cv = malloc(sizeof(struct cv));
	assert(cv);

	TBD();

	return cv;
}

void
cv_destroy(struct cv *cv)
{
	assert(cv != NULL);

	TBD();

	free(cv);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	assert(cv != NULL);
	assert(lock != NULL);

	TBD();
}
