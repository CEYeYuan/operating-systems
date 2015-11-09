#include "request.h"
#include "server_thread.h"
#include "common.h"

struct server {
	int nr_threads;
	int max_requests;
	int max_cache_size;
	/* add any other parameters you need */
};

struct thread_data{
	struct server *server;
	int num;
};

/* static functions */

/* initialize file data */
static struct file_data *
file_data_init(void)
{
	struct file_data *data;

	data = Malloc(sizeof(struct file_data));
	data->file_name = NULL;
	data->file_buf = NULL;
	data->file_size = 0;
	return data;
}

/* free all file data */
static void
file_data_free(struct file_data *data)
{
	free(data->file_name);
	free(data->file_buf);
	free(data);
}

static void
do_server_request(struct server *sv, int connfd)
{
	int ret;
	struct request *rq;
	struct file_data *data;

	data = file_data_init();

	/* fills data->file_name with name of the file being requested */
	rq = request_init(connfd, data);
	if (!rq) {
		file_data_free(data);
		return;
	}
	/* reads file, 
	 * fills data->file_buf with the file contents,
	 * data->file_size with file size. */
	ret = request_readfile(rq);
	if (!ret)
		goto out;
	/* sends file to client */
	request_sendfile(rq);
out:
	request_destroy(rq);
	file_data_free(data);
}

/*GLOBAL VARIABLES*/
pthread_t *threads;
struct thread_data *data;
int in,out;
pthread_mutex_t lock;
pthread_cond_t full;
pthread_cond_t empty;
int buffer_size;


void *
stub(void *arg){
	pthread_mutex_lock(&lock);
	while(in==out)
		pthread_cond_wait(&empty,&lock);
	struct thread_data *data=(struct thread_data *)arg;
	do_server_request(data->server,data->num);
	if((in-out+buffer_size)%buffer_size==buffer_size-1)
		pthread_cond_signal(&full);
	out=(out+1)%buffer_size;
	pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
}
/*pthread_create, pthread_mutex_init, pthread_mutex_lock, pthread_mutex_unlock, pthread_cond_init, pthread_cond_wait, and pthread_cond_signal*/


/* entry point functions */
struct server *
server_init(int nr_threads, int max_requests, int max_cache_size)
{
	struct server *sv;

	sv = Malloc(sizeof(struct server));
	sv->nr_threads = nr_threads;
	sv->max_requests = max_requests;
	sv->max_cache_size = max_cache_size;

	if (nr_threads > 0 || max_requests > 0 || max_cache_size > 0) {
		
		if(nr_threads>=1){
			pthread_mutex_init(&lock, NULL);
			pthread_mutex_lock(&lock);
			threads=malloc(sizeof(pthread_t)*(nr_threads+1));
			pthread_cond_init(&empty, NULL);
			pthread_cond_init(&full, NULL);
			int i;
			for(i=0;i<nr_threads;i++){
				/*
				pthread_create (thread,attr,start_routine,arg);
				thread: An opaque, unique identifier for the new thread returned by the subroutine.
				attr: An opaque attribute object that may be used to set thread attributes. You can specify a thread attributes object, or NULL for the default values.
				start_routine: the C routine that the thread will execute once it is created.
				arg: A single argument that may be passed to start_routine. It must be passed by reference as a pointer cast of type void. NULL may be used if no argument is to be passed.
				*/	
			   int rc;
				rc=pthread_create(&threads[i],NULL,stub,&data[i]);
				if (rc){
					 printf("ERROR; return code from pthread_create() is %d\n", rc);
					 exit(-1);
				 }
			}		
	}
	if(max_requests>=1){
		data=malloc(sizeof(struct thread_data)*(max_requests+1));
		in=0;
		out=0;
		buffer_size=max_requests+1;
	}
	/* Lab 4: create queue of max_request size when max_requests > 0 */

	/* Lab 5: init server cache and limit its size to max_cache_size */

	/* Lab 4: create worker threads when nr_threads > 0 */

	}
	pthread_mutex_unlock(&lock);
	return sv;
}
void
server_request(struct server *sv, int connfd)
{
	if (sv->nr_threads == 0) { /* no worker threads */
		do_server_request(sv, connfd);
	} else {
		/*  Save the relevant info in a buffer and have one of the
		 *  worker threads do the work. */
		pthread_mutex_lock(&lock);
		while((in-out+buffer_size)%buffer_size==buffer_size-1){
			pthread_cond_wait(&full, &lock);//buffer is full; wait
		}
		(data+in)->server=sv;
		(data+in)->num=connfd;
		if(in==out)//if previously, the buffer is empty, some consumer(thread) might be blocked because of no request
			pthread_cond_signal(&empty);
		in=(in+1)%buffer_size;
		pthread_mutex_unlock(&lock);
		
	}
}
