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
struct list *list;
pthread_mutex_t cache_lock;

struct map {
	/* you can define this struct to have whatever fields you want. */
	struct listnode **dict;
	int cache;
	int max;
};

struct list{
	struct listnode *head;
};

struct listnode{
	char * word;
	struct file_data *data;
	struct listnode *next;
	struct listnode *prev;
	struct listnode *copy;
	long size;
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
	map->cache=0;
	map->max=size;
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
void 
cache_evict(int amount_to_evict){
	int current=0;
	while(current<amount_to_evict&&map->cache!=0){
		struct listnode *remove=list->head->prev;
		remove->prev->next=remove->next;
		remove->next->prev=remove->prev;
		char *file_name=remove->word;
		long index=hashCode(remove->word,buckets);
		struct listnode *node=map->dict[index];
		if(strcmp(node->word,file_name)==0){
			//the first node in the bucket
			map->dict[index]=node->next;
			free(node->data);
			current+=node->size;
			map->cache-=node->size;
			free(node);
			free(remove->word);
			free(remove);
		}else{
			struct listnode *fast=node->next;
			char *file_name=remove->word;
			while(strcmp(node->word,file_name)!=0){
				fast=fast->next;
				node=node->next;
			}
			node->next=fast->next;
			free(fast->data);
			current+=fast->size;
			map->cache-=fast->size;
			free(fast);
			free(remove->word);
			free(remove);
			
		}
	}
}
struct listnode*
list_insert(char *file_name){
	struct listnode* node= malloc(sizeof(struct listnode));
	node->word=malloc(strlen(file_name)+1);
	struct listnode *head=list->head;
	//lru 
	struct listnode *tmp=head->next;
	head->next=node;
	node->prev=head;
	node->next=tmp;
	tmp->prev=node;
	return node;
}

struct listnode*
lookup(char *file_name){
	long index=hashCode(file_name,buckets);
	if(!map->dict[index]){
		return NULL;
	}else{
		struct listnode *node=map->dict[index];
		while(node->next){
		//look up while reorder the list
			if(strcmp(node->word,file_name)==0){
				struct listnode *tmp=node->copy;
				tmp->prev->next=tmp->next;
				tmp->next->prev=tmp->prev;
				node->copy=list_insert(tmp->word);
				free(tmp->word);
				free(tmp);
				return node;
			}
			node=node->next;
		}
		return NULL;
	}
}





void cache_insert(char *file_name,struct file_data *file_data){
	struct listnode *result=lookup(file_name);
	if(result!=NULL){
	//existed in cache
		map->cache-=file_data->file_size;
		result->data=file_data;
		map->cache+=file_data->file_size;
	}
	else{
		long index=hashCode(file_name,buckets);
		if(!map->dict[index]){

				//this is the first word that hashed to that bucket
				//init the bucket and linked list
				struct listnode* node= malloc(sizeof(struct listnode));
				node->word=malloc(strlen(file_name)+1);
				node->size=file_data->file_size;
				node->next=NULL;
				node->data=file_data;
				node->copy=list_insert(file_name);
				map->dict[index]=node;
				map->cache+=file_data->file_size;
		}
		else{
				struct listnode *node=map->dict[index];
				while(node->next){
						node=node->next;
				}
				struct listnode* newword = malloc(sizeof(struct listnode));
				newword->word=malloc(strlen(file_name)*sizeof(char)+1);
				newword->data=file_data;
				newword->size=file_data->file_size;
				newword->next=NULL;
				node->copy=list_insert(file_name);
				node->next=newword;
				map->cache+=file_data->file_size;
			
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
	pthread_mutex_lock(&cache_lock);
	if(lookup(data->file_name)==NULL){
		pthread_mutex_unlock(&cache_lock);
		ret = request_readfile(rq);
		if (!ret)
			goto out;
		//got the file after this line
		pthread_mutex_lock(&cache_lock);
		//check buffer size
		if((map->max-map->cache)>=data->file_size)
		//enough cache size
			cache_insert(data->file_name,data);
		else{
			cache_evict(data->file_size-(map->max-map->cache));
			cache_insert(data->file_name,data);
		}
		pthread_mutex_unlock(&cache_lock);
		
	}
	else{
		pthread_mutex_lock(&cache_lock);
		struct listnode *node=lookup(data->file_name);
		data->file_buf=(char *)malloc(sizeof(char)*(strlen(node->data->file_buf)+1));
		strcpy(data->file_buf,node->data->file_buf);
		data->file_size=node->size;
		pthread_mutex_unlock(&cache_lock);
	}
	
	if (!ret)
		goto out;
	/* sends file to client */
	request_sendfile(rq);
	//free(data->file_buf);
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
	map_init(max_cache_size);
	list=malloc(sizeof(struct list));
	list->head=malloc(sizeof(struct listnode));
	list->head->prev=list->head;
	list->head->next=list->head;
	

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
