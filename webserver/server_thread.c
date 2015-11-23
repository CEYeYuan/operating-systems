#include "request.h"
#include "server_thread.h"
#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include <string.h>
#include <ctype.h>
#define HASH_TABLE_SIZE 3571
pthread_mutex_t cache_mutex;
int CURRENT_SIZE=0;    
int MAX_SIZE; 
int buckets;
struct DATA** hashTable;
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
;





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





void hashTable_insert(struct file_data *data){           //insert new file data into hashtable
        int num;
        struct DATA *current,*new_point;
        int LENGTH_NAME , LENGTH_BUF;
        
        LENGTH_NAME = strlen(data->file_name)+1;
        LENGTH_BUF = strlen(data->file_buf)+1;
        
        new_point = malloc(sizeof (struct DATA));
        new_point->FILE_DATA = malloc(sizeof (struct file_data));
        new_point->FILE_DATA->file_name = (char*)malloc(LENGTH_NAME * sizeof(char));
        new_point->FILE_DATA->file_buf = (char*)malloc(LENGTH_BUF * sizeof(char));

        strcpy(new_point->FILE_DATA->file_name , data->file_name);  
        strcpy(new_point->FILE_DATA->file_buf , data->file_buf );
        new_point->FILE_DATA->file_size = data->file_size;
                       
        new_point->next = NULL;
          
        CURRENT_SIZE += data->file_size;
        num = hashCode(data->file_name);
        current = hashTable[num];
        if(current ==NULL){
                   hashTable[num] = new_point;
        }
        else{
                   while(current->next){                 //add to tail of list
                         current = current->next;
                   }
                   current->next = new_point;
        }
        return;
}



struct listnode*
cache_insert(struct file_data *data){
	struct listnode* node= malloc(sizeof(struct listnode));
	node->word=malloc(strlen(data->file_name)+1);
	strcpy(node->word,data->file_name);
	struct listnode *head=list->head;
	struct listnode *tmp=head->next;
	head->next=node;
	node->prev=head;
	node->next=tmp;
	tmp->prev=node;
	hashTable_insert(data);
	return node;
}


int cache_lookup(struct file_data *data){          //return 0: not find         return 1: find in cache
        struct DATA *current = NULL;
        unsigned int num;                          
        num = hashCode(data->file_name);
        current = hashTable[num];
        if(current == NULL)  return 0;
        while(current){                 //current is not NULL, therefore run this one
                if(strcmp(current->FILE_DATA->file_name,data->file_name) == 0){        
                        return 1;
		}
		else{
			current = current->next;
		}
	}
        return 0; 
}


int clear_hashtable(char *file_name){
        struct DATA *previous,*current;
        int size;
        int num;  
//        fprintf(stderr, "\n file_name is = %s \n",file_name);   
        num = hashCode(file_name);
        current = hashTable[num];
    //    if(current == NULL) return 1000;
     //   fprintf(stderr, "\n current one is = %s : %d\n",current->FILE_DATA->file_name,current->FILE_DATA->file_size);
        if(strcmp(current->FILE_DATA->file_name,file_name) == 0){      //is the first one
               hashTable[num] = current->next;
               size = current->FILE_DATA->file_size;
               CURRENT_SIZE -= size;
               
               free(current->FILE_DATA->file_name);
               current->FILE_DATA->file_name = NULL;
               free(current->FILE_DATA->file_buf);
               current->FILE_DATA->file_buf = NULL; 
               free(current->FILE_DATA);
               current->FILE_DATA = NULL;                   
               free(current); 

               return size;
        }
        else{
               previous = current;
               current = current->next;
               while(strcmp(current->FILE_DATA->file_name,file_name) != 0){
                     previous = current;
                     current = current->next;
               }
               previous->next = current->next;               //must have one, so no need to check
               size = current->FILE_DATA->file_size;
               CURRENT_SIZE -= size;    
                           
               free(current->FILE_DATA->file_name);
               current->FILE_DATA->file_name = NULL;
               free(current->FILE_DATA->file_buf);
               current->FILE_DATA->file_buf = NULL;
               free(current->FILE_DATA);
               current->FILE_DATA = NULL;
               free(current);

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
			size = clear_hashtable(remove->word);
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
        struct DATA *current;
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
        if(cache_lookup(data)==0){             //not find in cache
                pthread_mutex_unlock(&cache_mutex);
                ret = request_readfile(rq);
                if (!ret)
		       goto out;
                pthread_mutex_lock(&cache_mutex);
                size = data->file_size;
                if(size < MAX_SIZE){
                       // if(cache_lookup(data)==0){
                                if((MAX_SIZE-CURRENT_SIZE) > size)        //have enough space
                                      if(cache_lookup(data)==0) 
                                        cache_insert(data);
                                      else;
                                else{
                                       cache_evict(size - (MAX_SIZE-CURRENT_SIZE)); 
                                       if(cache_lookup(data)==0)
                                          cache_insert(data);
                                       else;
                                }
                      //  }         
                }  
              pthread_mutex_unlock(&cache_mutex); 
        }
        else{                                           
                 num = hashCode(data->file_name);
                 current = hashTable[num];
      
                 while(strcmp(current->FILE_DATA->file_name,data->file_name) != 0)        //find the one in cache
                        current = current->next;
                           
                 data->file_buf = (char *)malloc( (strlen(current->FILE_DATA->file_buf)+1) * sizeof(char) );
                 strcpy(data->file_buf , current->FILE_DATA->file_buf);
                 data->file_size = current->FILE_DATA->file_size;
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
	buckets=max_cache_size/351+1;
	if(buckets<=666)
		buckets=667;
        list=malloc(sizeof(struct list));
        list->head=malloc(sizeof(struct listnode));
        list->head->prev=list->head;
        list->head->next=list->head;
		hashTable=malloc(sizeof(struct DATA*)*buckets);
        int i;
        for(i=0;i<buckets;i++)
               hashTable[i] = NULL;

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
