#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "point.h"
#include "sorted_points.h"

struct sorted_points {
	/* you can define this struct to have whatever fields you want. */
	struct point *pt;
	struct sorted_points *next;
};
struct sorted_points *
sp_init()
{
	struct sorted_points *sp;

	sp = (struct sorted_points *)malloc(sizeof(struct sorted_points));
	assert(sp);
	sp->pt=NULL;
	sp->next=NULL;
	//TBD();

	return sp;
}

void
sp_destroy(struct sorted_points *sp)
{
	//TBD();
	while(sp!=NULL){
		struct sorted_points *tmp;
		tmp=sp;
		sp=sp->next;
		free(tmp->pt);
		free(tmp->next);
	}
	free(sp);
}

int
sp_add_point(struct sorted_points *sp, double x, double y)
{
	TBD();
	/*struct point *p;
	p=(struct point *)malloc(sizeof(struct point));
	p->x=x;
	p->y=y;
	if(sp->pt==NULL)
		sp->pt=p;
	else{
		while(sp->next!=NULL&&point_compare(sp->pt,p)<0)
			sp++;
		//if(sp->next=NULL)


	
		}*/
	return 0;
}

int
sp_remove_first(struct sorted_points *sp, struct point *ret)
{
/* Remove the first point from the sorted list.  Caller provides a pointer to a
 * Point where this procedure stores the values of that point. Returns 1 on
 * success and 0 on failure (empty list). */
	if(sp==NULL){
		ret=NULL;
		return 	0;
	}
	else{
		if(sp->pt!=NULL){
			ret=malloc(sizeof(struct point));
			point_set(ret,sp->pt->x,sp->pt->y);
		}
		
		free(sp->pt);
		free(sp->next);
		free(sp);
		return 1;
	}

	
}

int
sp_remove_last(struct sorted_points *sp, struct point *ret)
{
	TBD();
	return 0;
}

int
sp_remove_by_index(struct sorted_points *sp, int index, struct point *ret)
{
 /* Remove the point that appears in position <index> on the sorted list, storing
 * its value in *ret. Returns 1 on success and 0 on failure (too short list).
 * The first item on the list is at index 0. */
 //use a fake head to eliminate corner cases


	struct sorted_points *fakeHead=malloc(sizeof(struct sorted_points));
	fakeHead->next=sp;
	struct sorted_points *cur=fakeHead;
	int i=0;
	int flag=0;
	while(cur!=NULL){
		if(i==index){
			if(cur->next==NULL){
				flag=0;
				break;
			}		
			else{
				struct sorted_points *tmp=cur->next;
				cur->next=cur->next->next;
				if(tmp->pt!=NULL){

					ret=malloc(sizeof(struct point));
					point_set(ret,tmp->pt->x,tmp->pt->y);
				}	
				free(tmp->pt);
				free(tmp->next);
				free(tmp);
				flag=1;
				break;

			}
		}
		else{
			cur=cur->next;
			i++;
		}
	}
	printf("%p\n",ret );
	if(!flag)
		free(ret);
	//printf("x=%f y=%f \n",ret->x,ret->y );
	free(fakeHead);
	return flag;
}

int
sp_delete_duplicates(struct sorted_points *sp)
{
	TBD();
	return -1;
}
