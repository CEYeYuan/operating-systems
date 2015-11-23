#include "request.h"
#include "server_thread.h"
#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include <string.h>
#include <ctype.h>
pthread_mutex_t cache_mutex;
int current_size=0;    
int max_size; 
int buckets;
struct table** map; 
struct list *list;


struct server {
	int nr_threads;
	int max_requests;
	int max_cache_size;
	/* add any other parameters you need */
};
struct DATA{                                    
        struct file_data *FILE_DATA;
        struct DATA *next; 
};

struct table{
	char *word;
	struct file_data *data;
	struct table *next;
};

struct list{
	struct listnode *head;
};

struct listnode{
	char * word;
	struct file_data *data;
	struct listnode *next;
	struct listnode *prev;
	long size;
} ;






long hashCode(char *key){

    long hashVal = 0;
    int i = 0;
    while( i < strlen(key)) {
      hashVal = (127 * hashVal + *(key+i)) % buckets;
      i++;
    }
    if(hashVal<0)	return -1*hashVal;
    else
    	return hashVal;
}

struct table*
lookup(char *file_name){
	long index=hashCode(file_name);
	if(map[index]==NULL){
		return NULL;
	}else{
		struct table *node=map[index];
		while(node!=NULL){
		//look up while reorder the list
			if(strcmp(node->word,file_name)==0){
				/*struct listnode *tmp=node->copy;
				tmp->prev->next=tmp->next;
				tmp->next->prev=tmp->prev;
				node->copy=list_insert(tmp->word);
				free(tmp->word);
				free(tmp);*/
				return node;
			}
			node=node->next;
		}
		return NULL;
	}
}

void cache_insert(struct file_data *file_data){
	struct table *result=lookup(file_data->file_name);
	if(result!=NULL){
		//existed in cache
		current_size-=result->data->file_size;
		result->data=file_data;
		current_size+=file_data->file_size;
	}
	else{
		long index=hashCode(file_data->file_name);
		if(map[index]==NULL){

				//this is the first word that hashed to that bucket
				//init the bucket and linked list
				struct table* node= malloc(sizeof(struct table));
				node->word=malloc(strlen(file_data->file_name)+1);
				strcpy(node->word,file_data->file_name);
				node->next=NULL;
				node->data->file_buf=malloc((strlen(file_data->file_buf)+1) * sizeof(char));
				node->data->file_size=file_data->file_size;
				strcpy(node->data->file_buf, file_data->file_buf);
				map[index]=node;
				current_size+=file_data->file_size;
		}
		else{
				struct table *node=map[index];
				while(node->next!=NULL){
						node=node->next;
				}
				struct table* newword = malloc(sizeof(struct listnode));
				newword->word=malloc(strlen(file_data->file_name)*sizeof(char)+1);
				strcpy(node->word,file_data->file_name);
				newword->data->file_buf=malloc((strlen(file_data->file_buf)+1) * sizeof(char));
				strcpy(newword->data->file_buf,file_data->file_buf );
				newword->data->file_size=file_data->file_size;
				newword->next=NULL;
				node->next=newword;
				current_size+=file_data->file_size;
			}		
	}

}



struct listnode*
list_insert(struct file_data *data){
//insert new cache to the list(as well as hash map, use the last line of function call) 
	struct listnode* node= malloc(sizeof(struct listnode));
	node->word=malloc(strlen(data->file_name)+1);
	strcpy(node->word,data->file_name);
	struct listnode *head=list->head;
	struct listnode *tmp=head->next;
	head->next=node;
	node->prev=head;
	node->next=tmp;
	tmp->prev=node;
	cache_insert(data);
	return node;
}



int clear_hash(char *file_name){
	int size;
	long index=hashCode(file_name);
	struct table* node=map[index];
		if(strcmp(node->word,file_name)==0){
			//the first node in the bucket
			map[index]=node->next;
			size=node->data->file_size;
			current_size+=node->data->file_size;
			free(node->data->file_name);
			free(node->data->file_buf);
			free(node->data);
			free(node);
			return size;
		}else{
			struct table *fast=node->next;
			while(strcmp(fast->word,file_name)!=0){
				fast=fast->next;
				node=node->next;
			}
			size=fast->data->file_size;
			node->next=fast->next;
			current_size+=fast->data->file_size;
			free(fast->data->file_name);
			free(fast->data->file_buf);
			free(fast->data);
			free(fast);
			return size;
			
		}
}


int remove_least_used(){
		int size=0;
		struct listnode *remove=list->head->prev;
		if(remove==list->head)
			return 0;	
		else{
			remove->prev->next=remove->next;
			remove->next->prev=remove->prev;
			size = clear_hash(remove->word);
			free(remove->word);
			free(remove);
			return size;
		}
}

void cache_evict(int amount){
        int size=0;
        while(amount > size){ 
               size += remove_least_used(); 
        }  
        return;   
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
do_server_request(struct server *sv, int connfd)               //still not move it out!!
{
        
	int ret;
	struct request *rq;
//        struct request *REQUEST;              //the one you copy rq
 	struct file_data *data;
        struct table *current;
        unsigned int num;
        
//        int LENGTH_NAME , LENGTH_BUF;
        int size; 
	data = file_data_init();


	/* fills data->file_name with name of the file being requested */
	rq = request_init(connfd, data);
	if (!rq) {
		file_data_free(data);
		return;
	}
        pthread_mutex_lock(&cache_mutex);
        if(lookup(data->file_name)==NULL){             //not find in cache
                pthread_mutex_unlock(&cache_mutex);
                ret = request_readfile(rq);
                if (!ret)
		       goto out;
                pthread_mutex_lock(&cache_mutex);
                size = data->file_size;
                if(size < max_size){
                       // if(cache_lookup(data)==0){
                                if((max_size-current_size) > size)        //have enough space
                                      if(lookup(data->file_name)==0) 
                                        list_insert(data);
                                      else;
                                else{
                                       cache_evict(size - (max_size-current_size)); 
                                       if(lookup(data->file_name)==NULL)
                                          list_insert(data);
                                       else;
                                }
                      //  }         
                }  
              pthread_mutex_unlock(&cache_mutex); 
        }
        else{                                           
                 num = hashCode(data->file_name);
                 current = map[num];
      
                 while(strcmp(current->data->file_name,data->file_name) != 0)        //find the one in cache
                        current = current->next;
                           
                 data->file_buf = (char *)malloc( (strlen(current->data->file_buf)+1) * sizeof(char) );
                 strcpy(data->file_buf , current->data->file_buf);
                 data->file_size = current->data->file_size;
                 pthread_mutex_unlock(&cache_mutex); 
        }

     
        
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
	buckets=max_cache_size/31+1;
	if(buckets<=332)
		buckets=333;
    list=malloc(sizeof(struct list));
    list->head=malloc(sizeof(struct listnode));
    list->head->prev=list->head;
    list->head->next=list->head;
    map=malloc(sizeof(struct table*)*buckets);
    int i;        
    for(i=0;i<buckets;i++){
       map[i]=NULL;
     }
             
	if (nr_threads > 0 || max_requests > 0 || max_cache_size > 0) {
		
		if(nr_threads>=1){
			data=malloc(sizeof(int)*(max_requests+1));
			in=0;
			out=0;
			buffer_size=max_requests+1;
			pthread_mutex_init(&lock, 0);
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
