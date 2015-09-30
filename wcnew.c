#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "wc.h"
#include <string.h>
#include <ctype.h>


int counter =0;

struct wc {
	/* you can define this struct to have whatever fields you want. */
	struct list *bucket ;
};

struct list{
	struct listnode *head;
	int length;
};
struct listnode{
	char * word;
	int count;
	struct listnode *next;
} ;

long hashCode(char *key, long size){

    long hashVal = 0;
    int i = 0;
    while( i < strlen(key)) {
    	//printf("%c\n", *(key+i));
      hashVal = (127 * hashVal + *(key+i)) % size;
      i++;
    }
    return hashVal;
}


struct wc *
wc_init(char *word_array, long size)
{
/* Initialize wc data structure, returning pointer to it. The input to this
 * function is an array of characters. The length of this array is size.
 * The array contains a sequence of words, separated by spaces. You need to
 * parse this array for words, and initialize your data structure with the words
 * in the array. You can use the isspace() function to look for spaces between
 * words. Note that the array is read only and cannot be modified. */

	/*struct wc **wc=malloc(sizeof(*wc)*size/5);
	
	*/
	struct wc **dict;
	int j=0;
	dict=malloc(size/5*sizeof(struct wc*));
	while(j<size/5){
		*(dict+j)=NULL;
		j++;
	}
	counter=size/5;
	int i=0;//count the character
	int start=0;//start of each string
	int len=0;//length of each string

	while(i<size&&isspace(*(word_array+i))){
			i++;
		}
	while(i<size){
		len=0;
		start=i;
		while(i<size&&!isspace(*(word_array+i))){
			i++;
			len++;
		}
		char *str=malloc(len*sizeof(char)+1);
		strncpy(str,word_array+start,len);
		while(i<size&&isspace(*(word_array+i))){
			i++;
		}
		start=i;

		long index=hashCode(str,size/5);
		// printf("%s is hashed to index %ld\n",str,index );
		
		
		if(!dict[index]){
			//this is the first word that hashed to that bucket
			//init the bucket and linked list
			dict[index]=malloc(sizeof(struct wc));
			struct list* first = malloc(sizeof(struct list));
			first->head=malloc(sizeof(struct listnode));
			first->head->word=malloc(sizeof(str));
			strcpy(first->head->word,str);
			first->length=1;
			dict[index]->bucket=first;
			first->head->next=NULL;
			printf("%s \n",dict[index]->bucket->head->word);
		}
		
		else{
			//printf("%p not null?\n",wc[index]->bucket );
			//printf("%p\n", wc[index]);
			/*if(wc[index]->item){
				struct listnode *node=wc[index]->item;
				printf("%p\n", node);
			}
			
			*/
			/*
			int added=0;
			while(node->next){
				if(strcmp(node->word,str)==0){
					node->count+=1;
					added=1;
					break;
				}
				else
					node=node->next;
			}
			
			if(!added){
				struct listnode* newword = malloc(sizeof(listnode));
				node->word=malloc(sizeof(str));
				strcpy(newword->word,str);
				node->count=1;
			}
			*/
		}
		
		 free(str);
		i++;
	}
	
	assert(dict);
	return dict[0];
}

void
wc_output(struct wc *wc)
{
/* wc_output produces output, consisting of unique words that have been inserted
 * in wc (in wc_init), and a count of the number of times each word has been
 * seen.
 *
 * The output should be sent to standard output, i.e., using the standard printf
 * function.
 *
 * The output should be in the format shown below. The words do not have to be
  * sorted in any order. Do not add any extra whitespace.
word1:5
word2:10
word3:30
word4:1
 */
/*
printf("%s\n","ssssss" );
int i=0;
while(i<counter){
	struct wc *current=&wc[i];
	if(current){
		struct listnode *node=current->item;
		while(node){
			printf("%s:%d\n",node->word,node->count);
			node=node->next;
		}
	}
	i++;
}
	*/
}

void
wc_destroy(struct wc *wc)
{
	/*
	int i=0;
	while(i<counter){
		struct wc *current=&wc[i];
		if(current){
			struct listnode *node=current->item;
			while(node){
				struct listnode *tmp=node->next;
				free(node->word);
				free(node);
				node=tmp;
			}
		}
		free(current);
		i++;
	}
	free(wc);
	*/
}
