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
	struct listnode *item;
};

struct listnode{
	char * word;
	int count;
	struct listnode *next;
} listnode;

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

	struct wc *wc[size/5];
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
		printf("%s is hashed to index %ld\n",str,index );
		
		
		if(!wc[index]){
			//this is the first word that hashed to that bucket
			//init the bucket and linked list
			struct listnode* node = malloc(sizeof(listnode));
			node->word=malloc(sizeof(str));
			strcpy(node->word,str);
			node->count=1;
			wc[index]=malloc(sizeof(struct wc));
			wc[index]->item=node;
		}
		
		else{
			//struct listnode *node=wc[index]->item;
			//int added=0;
			while(node->next){
				if(strcmp(node->word,str)==0){
					node->count+=1;
			//		added=1;
					break;
				}
				else
					node=node->next;
			}
			/*
			if(!added){
				struct listnode* newword = malloc(sizeof(listnode));
				node->word=malloc(sizeof(str));
				strcpy(newword->word,str);
				node->count=1;
			}*/
		}
		
		// free(str);
		i++;
	}
	
	assert(wc);
	return wc[0];
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
