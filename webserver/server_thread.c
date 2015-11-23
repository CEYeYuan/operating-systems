#include "request.h"
#include "server_thread.h"
#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include <string.h>
#include <ctype.h>

struct server {
	int nr_threads;
	int max_requests;
	int max_cache_size;
	/* add any other parameters you need */
};
long buckets =0;
struct map *map;

struct map {
	/* you can define this struct to have whatever fields you want. */
	struct listnode **dict;
};

struct listnode{
	char * word;
	struct file_data *data;
	struct listnode *next;
} ;

long hashCode(char *key, long size){

    long hashVal = 0;
    int i = 0;
    while( i < strlen(key)) {
      hashVal = (127 * hashVal + *(key+i)) % size;
      i++;
    }
    if(hashVal<0)	return -1*hashVal;
    else
    	return hashVal;
}


void
map_init(long size)
{
//size means the size of cache
/* Initialize wc data structure, returning pointer to it. The input to this
 * function is an array of characters. The length of this array is size.
 * The array contains a sequence of words, separated by spaces. You need to
 * parse this array for words, and initialize your data structure with the words
 * in the array. You can use the isspace() function to look for spaces between
 * words. Note that the array is read only and cannot be modified. */

	
	map=malloc(sizeof(map));
	if(size<=0)	return;
	buckets=size/351+1;
	long j=0;
	map->dict=malloc((buckets)*sizeof(struct listnode*));
	while(j<buckets){
		map->dict[j]=NULL;
		j++;		
	}
}
//cache_lookup(file), cache_insert(file), and cache_evict(amount_to_evict)
void cache_insert(char *file_name,struct file_data *file_data){
	long index=hashCode(file_name,buckets);
	if(!map->dict[index]){

			//this is the first word that hashed to that bucket
			//init the bucket and linked list
			struct listnode* node= malloc(sizeof(struct listnode));
			node->word=malloc(strlen(file_name)+1);
			node->next=NULL;
			node->data=file_data;
			map->dict[index]=node;
	}
	else{
			struct listnode *node=map->dict[index];
			int added=0;
			while(node->next){
				/*if(strcmp(node->word,file_name)==0){
					node->count+=1;
					added=1;	//printf(" %s:%ld\n",node->word,node->count);
					break;
				}
				else*/
				//assume no two files have the same name
					node=node->next;
			}

			if(added==0){
				/*if(strcmp(node->word,str)==0){
					node->count+=1;
				}
				else{
				*/
					struct listnode* newword = malloc(sizeof(struct listnode));
					newword->word=malloc(strlen(file_name)*sizeof(char)+1);
					newword->data=file_data;
					newword->next=NULL;
					node->next=newword;
				}
				
			
		}	
}


		

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
pthread_t **threads;
int *data;
int in,out;
pthread_mutex_t lock;
pthread_cond_t full;
pthread_cond_t empty;
int buffer_size;


void *
stub(void *server){
	while(1){
		pthread_mutex_lock(&lock);
		while(in==out)
			pthread_cond_wait(&empty,&lock);
		if((in-out+buffer_size)%buffer_size==buffer_size-1)
			pthread_cond_signal(&full);
		int conn=data[out];
		out=(out+1)%buffer_size;
		pthread_mutex_unlock(&lock);
		do_server_request(server,conn);
	}
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
			data=malloc(sizeof(int)*(max_requests+1));
			in=0;
			out=0;
			buffer_size=max_requests+1;
			pthread_mutex_init(&lock, 0);
			//pthread_mutex_lock(&lock);
			threads=malloc(sizeof(pthread_t)*(nr_threads));
			pthread_cond_init(&empty, NULL);
			pthread_cond_init(&full, NULL);
			int i;
			for(i=0;i<nr_threads;i++){
            	threads[i]=(pthread_t *) malloc (sizeof(pthread_t));
			    int rc;
				rc=pthread_create(threads[i],NULL,&stub,sv);
				if (rc){
					 printf("ERROR; return code from pthread_create() is %d\n", rc);
					 exit(-1);
				 }
			}		
	}
	
	/* Lab 4: create queue of max_request size when max_requests > 0 */

	/* Lab 5: init server cache and limit its size to max_cache_size */

	/* Lab 4: create worker threads when nr_threads > 0 */

	}
	//pthread_mutex_unlock(&lock);
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
		data[in]=connfd;
		if(in==out)//if previously, the buffer is empty, some consumer(thread) might be blocked because of no request
			pthread_cond_signal(&empty);
		in=(in+1)%buffer_size;
		pthread_mutex_unlock(&lock);
		
	}
}
