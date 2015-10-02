#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "wc.h"
#include <string.h>
#include <ctype.h>


long counter =0;

struct wc {
	/* you can define this struct to have whatever fields you want. */
	struct listnode **dict;
};

struct listnode{
	char * word;
	long count;
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
    if(hashVal<0)	return -1*hashVal;
    else
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

	
	struct wc *wc=malloc(sizeof(wc));
	if(size<=0)	return wc;
	counter=size/35+1;
	long  i=0;//count the character
	int len=0;//length of each string
	long j=0;
	wc->dict=malloc((1+size/35)*sizeof(struct listnode*));
	//printf("size = %ld\n",size/5);
	while(j<size/35+1){
		wc->dict[j]=NULL;
		j++;		
	}

	while(i<size&&isspace(*(word_array+i))){
			i++;
		}
	while(i<size){
		len=0;
		char *str=malloc(sizeof(char)*100);
		while(i<size&& !isspace(*(word_array+i))){
			*(str+len)=*(word_array+i);
			i++;
			len++;			
		}
		*(str+len)='\0';
		
		while(i<size&&isspace(*(word_array+i))){
			i++;
		}
		long index=hashCode(str,size/35+1);
		
		if(!wc->dict[index]){

			//this is the first word that hashed to that bucket
			//init the bucket and linked list
			struct listnode* node= malloc(sizeof(struct listnode));
			node->word=malloc(strlen(str));
			node->next=NULL;
			strcpy(node->word,str);
			node->count=1;
			wc->dict[index]=node;
		}
		
		else{
			
			
			struct listnode *node=wc->dict[index];
			int added=0;
			while(node->next){
				if(strcmp(node->word,str)==0){
					node->count+=1;
					added=1;	//printf(" %s:%ld\n",node->word,node->count);
					break;
				}
				else
					node=node->next;
			}

			if(added==0){
				if(strcmp(node->word,str)==0){
					node->count+=1;
				}
				else{
					struct listnode* newword = malloc(sizeof(struct listnode));
					newword->word=malloc(strlen(str));
					strcpy(newword->word,str);
					newword->count=1;
					newword->next=NULL;
					node->next=newword;
				}
				
			}
		}
		
	    free(str);
	}
	
	assert(wc);
	return wc;
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

int i=0;
struct listnode *node;
while(i<counter){
	node=wc->dict[i];	
	while(node){
		printf("%s:%ld\n",node->word,node->count);
		node=node->next;	
	}	
	i++;
	}
	
}

void
wc_destroy(struct wc *wc)
{
	
	int i=0;
	while(i<counter){
		
			struct listnode *node=wc->dict[i];
			while(node){
				struct listnode *tmp=node->next;
				free(node->word);
				free(node);
				node=tmp;
			}
		i++;
	}
	free(wc->dict);
	free(wc);	
}
