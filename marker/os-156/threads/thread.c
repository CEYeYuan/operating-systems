#include <assert.h>
#include <stdlib.h>
#include <ucontext.h>
#include "thread.h"
#include "interrupt.h"

/* This is the thread control block */

enum state{ 
	running=1,
	ready=2,
	blocked=3,
	exited=4
};



//information about a particular thread
typedef struct thread {
	/* a linked list of thread */
	ucontext_t *mycontext;
	enum state status;
	void *sp_base;
	Tid id;
	struct thread *next;
} thread;

struct list {
	/* you can define this struct to have whatever fields you want. */
	struct thread *head;
	int size;
};

/* thread starts by calling thread_stub. The arguments to thread_stub are the
 * thread_main() function, and one argument to the thread_main() function. */
void
thread_stub(void (*thread_main)(void *), void *arg)
{
	Tid ret;

	thread_main(arg); // call thread_main() function with arg
	ret = thread_exit(THREAD_SELF);
	// we should only get here if we are the last thread. 
	assert(ret == THREAD_NONE);
	// all threads are done, so process should exit
	exit(0);
}


void printlist(struct list *sp,struct thread *cur){
		printf("%s %d        ","running thread ",cur->id );
		if(sp->size==0)
			printf("%s","ready queue is empty list\n" );
		else{
			printf("ready queue: [");
			struct thread *tmp;
			tmp=sp->head;
			int i=0;
			while(i<sp->size){
				printf("%d,",tmp->id);
				tmp=tmp->next;
				i++;
			}
			printf("]   size=%d \n",sp->size);
		}
}
int 
thread_compare(struct thread *a,struct thread *b){
	//compare the id of two thread; no duplicate id are allowed
	if(a->id>b->id)
		return 1;
	else
		return -1;
}
struct thread *
removeFirst(struct list *queue){
	if(queue->size==0){
		return NULL;
	}
	else{
		struct thread *pt=queue->head;
		queue->head=queue->head->next;
		queue->size-=1;
		return pt;
	}

}
struct thread *
removeById(struct list *queue,Tid tid){
	struct thread * pt=NULL;
	if(queue->size==0)
		return pt;
	else{
		struct thread * tmp=queue->head->next;
		struct thread *cur=queue->head;
		if(cur->id==tid){
			queue->head=tmp;
			queue->size-=1;
			return cur;
		}
		while(tmp!=NULL){
			if(tmp->id!=tid){
				cur=cur->next;
				tmp=tmp->next;
			}
			else
				break;
		}
		if(tmp==NULL)
			return pt;
		else{
			pt=tmp;
			cur->next=tmp->next;
			queue->size-=1;
			return pt;
		}

		
	}
}

void
add_thread(struct list *queue, struct thread *thread)
{	thread->next=NULL;
	if(queue->size==0){
		queue->head=thread;
	}
	else{
		struct 	thread *head;
		head=queue->head;
		while(head->next!=NULL){
			head=head->next;
		}
		head->next=thread;
	}
	queue->size+=1;
}



struct thread *current;
struct list *readyQueue;
struct list *exitedQueue;
int *arr;//mark the # of threads, 0 for not used, 1 for used
void
thread_init(void)
{
	/* your optional code here */
	current=malloc(sizeof(struct thread));
	readyQueue=malloc(sizeof(struct list));
	exitedQueue=malloc(sizeof(struct list));
	exitedQueue->size=0;
	current->mycontext=malloc(sizeof(ucontext_t));
	getcontext(current->mycontext);
	current->id=0;
	current->sp_base=NULL;
	current->status=running;
	current->next=NULL;//this field should never get used.
	readyQueue->size=0;
	arr=malloc(sizeof(int)*THREAD_MAX_THREADS);
	int i=0;
	while(i<THREAD_MAX_THREADS){
		arr[i]=0;
		i++;
	}
	arr[0]=1;
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
	//printlist(readyQueue,current);
/* create a thread that starts running the function fn(arg). Upon success,
 * return the thread identifier. On failure, return the following:
 *
 * THREAD_NOMORE: no more threads can be created.
 * THREAD_NOMEMORY: no more memory available to create a thread stack. 
 

 to create a new thread, you will use getcontext to create a valid context,
 but you will leave the current thread running; you (the current thread, actually) 
 will then change a few registers in this valid context to initialize it as a new thread, 
 and put this new thread into the ready queue; finally, at some point, the new thread will
 be chosen by the scheduler, and it will run when setcontext is called on this new 
 thread's context.

when creating a thread, you can't just make a copy of the current thread's context (using getcontext).
You need to make a copy and then change four things:
You need to change the program counter to point to the first function that the thread should run.
You need to allocate a new stack.
You need to change the stack pointer to point to the top of the new stack.
You need to setup the parameters to the first function.
 */
	if(readyQueue->size+1>=THREAD_MAX_THREADS)
		return THREAD_NOMORE;
	else{
		struct thread *another=malloc(sizeof(thread));
		another->mycontext=malloc(sizeof(ucontext_t));
		getcontext(another->mycontext);
		void *stack=malloc(THREAD_MIN_STACK*sizeof(char));
		if(stack==NULL){
			free(another->mycontext);
			free(another);
			return THREAD_NOMEMORY;
		}
		else{
			(*another->mycontext).uc_mcontext.gregs[REG_RIP]=(unsigned long)&thread_stub;
			(*another->mycontext).uc_mcontext.gregs[REG_RSP]=(unsigned long)(stack+THREAD_MIN_STACK);
			//should store the highest address; but both malloc and free should use the lowest address
			another->sp_base=stack;
			//why do we need another member in the thread? becasue the register value for RSP will
			//change as code executing, it's not necessary the base address when we exit the thread
			(*another->mycontext).uc_mcontext.gregs[REG_RDI]=(unsigned long)fn;//first argument
			(*another->mycontext).uc_mcontext.gregs[REG_RSI]=(unsigned long)parg;//second argument
			int id=0;
			while(arr[id]==1&&id<THREAD_MAX_THREADS-1){
				id++;
			}
			arr[id]=1;
			another->id=id;
			add_thread(readyQueue,another);
			return id;
		}
	}
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
 //printlist(readyQueue,current);
 	if(want_tid==THREAD_SELF||want_tid==current->id)
 		return current->id;
 	else if(want_tid==THREAD_ANY){
 		struct thread *pt=removeFirst(readyQueue);
 		if(pt==NULL)
 			return THREAD_NONE;
 		else {
 			int mark=0;//DO NOT WANT INFINITE LOOP BETWEEN SAVE/RECOVERY!!!!!!!!! 
 			struct thread *old=current;
 			if(old->status==running){// if it's exited, we do not want to add it to readyqueue
 				old->status=ready;
 				old->next=NULL;
 				add_thread(readyQueue,old);
 			}
 			current=pt;
 			current->status=running;
 			
 			int ret=current->id;//necessary,otherwise,when switch back,it will return the 
 			//original id
 			getcontext(old->mycontext);
 			mark++;
 			if(mark>=2) 
 				return ret;
 			else{
 				setcontext(current->mycontext);
 				return ret;
 			}
 			//when switch back, current is still the running thread, so it won't 
 			//return the new thread id;	
 		}
 	}
 	else{
 		struct thread *target=(void*)removeById(readyQueue,want_tid);
 		if(target==NULL)
			return THREAD_INVALID;
		else{
			int mark=0;//DO NOT WANT INFINITE LOOP BETWEEN SAVE/RECOVERY!!!!!!!!! 
 			struct thread *old=current;
 			if(old->status==running){// if it's exited, we do not want to add it to readyqueue
 				old->status=ready;
 				old->next=NULL;
 				add_thread(readyQueue,old);
 			}
 			current=target;
 			current->status=running;
 			current->next=NULL;
 			int ret=current->id;//necessary,otherwise,when switch back,it will return the 
 			//original id
 			getcontext(old->mycontext);
 			mark++;
 			if(mark>=2) 
 				return ret;
 			else{
 				setcontext(current->mycontext);
 				return ret;
 			}
		}

 	}
}

Tid
thread_exit(Tid tid)
{ 

/* destroy the thread whose identifier is tid. The calling thread continues to
 * execute and receives the result of the call. tid can be identifier of any
 * available thread or the following constants:
 *
 * THREAD_ANY:     destroy any thread except the caller.
 * THREAD_SELF:    destroy the calling thread and reclaim its resources. in this
 *		   case, the calling thread obviously doesn't run any
 *		   longer. some other ready thread is run.
 *
 * Upon success, return the identifier of the destroyed thread. A new thread
 * should be able to reuse this identifier. Upon failure, the calling thread
 * continues running, and returns the following:
 *
 * THREAD_INVALID: identifier tid does not correspond to a valid thread.
 * THREAD_NONE:	   no more threads, other than the caller, are available to
 *		   destroy, i.e., this is the last thread in the system. This
 *		   can happen in response to a call with tid set to THREAD_ANY
 *		   or THREAD_SELF. */
 	//printlist(readyQueue,current);
 	//printf("%s   %d\n","destroying",tid );
 	struct thread *temp;
 	while(exitedQueue->size!=0){
 		temp=exitedQueue->head;
 		exitedQueue->size-=1;
 		exitedQueue->head=temp->next;
 		void* stack=(void*)temp->sp_base;
			//for the first time of thread 0,since we do not use malloc
				//for it's stack, we have nothing to free. For other cases, we can safely
				//free it
		if(stack!=NULL)
			free(stack);
		free(temp->mycontext);
		free(temp);
 	}
	if(tid==THREAD_ANY){
		struct thread *pt=removeFirst(readyQueue);
		if(pt==NULL)
			return THREAD_NONE;
		else{
			arr[pt->id]=0;
			int ret=pt->id;
			void* stack=(void*)pt->sp_base;
			//for the first time of thread 0,since we do not use malloc
				//for it's stack, we have nothing to free. For other cases, we can safely
				//free it
			if(stack!=NULL)
				free(stack);
			free(pt->mycontext);
			free(pt);
			return ret;
		}

	}
	else if(tid==THREAD_SELF||tid==current->id){
		if(readyQueue->size==0)
			return THREAD_NONE;
		else{
			/*####buggy code : pt will always be null no matter where to put next line of code
			struct thread *pt=(void*)removeById(readyQueue,tid);
			thread_yield(THREAD_ANY);
			*/
			current->status=exited;
			arr[current->id]=0;
			add_thread(exitedQueue,current);
			/*##### buggy code, thread yield will add this thread to ready queue,which is not what 
			we'd like
			thread_yield(THREAD_ANY);
			*///
			thread_yield(THREAD_ANY);//modify thread yield
			return tid;
		}
	}

	else{
		struct thread *pt=removeById(readyQueue,tid);
		if(pt==NULL)
			return THREAD_INVALID;
		else{
			arr[tid]=0;
			void* stack=(void*)(pt->sp_base);
			if(stack!=NULL)
				free(stack);
			free(pt->mycontext);
			free(pt);
			return tid;
		}

		}
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
