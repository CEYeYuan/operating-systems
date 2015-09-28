#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "point.h"
#include "sorted_points.h"

struct sorted_points {
	/* you can define this struct to have whatever fields you want. */
	struct points *head;
	int size;
};

struct points{
	struct point *pt;
	struct points *next;
};
struct sorted_points *
sp_init()
{
	struct sorted_points *sp;

	sp = (struct sorted_points *)malloc(sizeof(struct sorted_points));
	assert(sp);
	sp->head=NULL;
	sp->size=0;
	//TBD();

	return sp;
}

void
sp_destroy(struct sorted_points *sp)
{
	//TBD();
	struct points *cur=sp->head;
	while(cur!=NULL){
		struct points* tmp=cur->next;
		free(cur->pt);
		free(cur->next);
		free(cur);
		cur=tmp;
	}
	sp->size=0;
	free(sp->head);
	free(sp);
}

int
sp_add_point(struct sorted_points *sp, double x, double y)
{
	/* Note: Points are sorted by their distance from the origin (0,0). If two
 * points are the same distance form the origin, then the one with a smaller x
 * coordinate should appear before one with a larger one. If two points are the
 * same distance and have the same x coordinate, then the one with the smaller
 * y coordinate should appear first.
 *
 * e.g., the following order is legal:
 * (0,0), (0, 1), (1, 0), (-2, 0), (0, 2), (2, 0)
 */
	struct points *p=malloc(sizeof(struct points));
	p->pt=malloc(sizeof(struct point));
	point_set(p->pt,x,y);
	p->next=NULL;

	if(sp->size==0){
		sp->head=p;
	}
	else{
		int i=0;
		struct points *node=sp->head;
		while(node->next!=NULL&&point_compare(node->next->pt,p->pt)<0){
			node=node->next;
			i++;
		}
		if(i==sp->size-1){
			node->next=p;
		}
		else{
			if(node->next!=NULL&&point_compare(node->next->pt,p->pt)>0){
				struct points *tmp=node->next->next;
				node->next=p;
				p->next=tmp;
			}
			else{//with same distance
				while(node->next!=NULL&&point_compare(node->next->pt,p->pt)==0 && node->next->pt->x < p->pt->x){
					node=node->next;
					i++;	
				}
				while(node->next!=NULL&&point_compare(node->next->pt,p->pt)==0 && node->next->pt->x == p->pt->x && node->next->pt->y < p->pt->y){
					node=node->next;
					i++;	
				}
				struct points *tmp=node->next;
				node->next=p;
				p->next=tmp;
				
			}
		}
	}
	
	sp->size+=1;
	return 0;
}

int
sp_remove_first(struct sorted_points *sp, struct point *ret)
{
/* Remove the first point from the sorted list.  Caller provides a pointer to a
 * Point where this procedure stores the values of that point. Returns 1 on
 * success and 0 on failure (empty list). */
 printf("%p\n",ret );
	if(sp->head==NULL){
		return 	0;
	}
	else{
		point_set(ret,sp->head->pt->x,sp->head->pt->y);
		struct points *tmp=sp->head->next;
		free(sp->head->pt);
		free(sp->head->next);
		free(sp->head);
		sp->head=tmp;
		sp->size-=1;
		return 1;

	}
}

int
sp_remove_last(struct sorted_points *sp, struct point *ret)
{
/* Remove the last point from the sorted list, storing its value in
 * *ret. Returns 1 on success and 0 on failure (empty list). */
	//TBD();
	if(sp->size==0){
		return 0;
	}
	else if(sp->size==1){
		free(sp->head->pt);
		free(sp->head->next);
		free(sp->head);
		sp->size=0;
		return 1;
	}
	else{
		struct points *node=sp->head;
		int i=2;
		while(i<sp->size){
			node=node->next;
			i++;
		}
		free(node->next->pt);
		free(node->next->next);
		free(node->next);
		sp->size-=1;
		return 1;
	}
}

int
sp_remove_by_index(struct sorted_points *sp, int index, struct point *ret)
{
 /* Remove the point that appears in position <index> on the sorted list, storing
 * its value in *ret. Returns 1 on success and 0 on failure (too short list).
 * The first item on the list is at index 0. */
 //use a fake head to eliminate corner cases
 	if(index>=sp->size||index<0){
 		return 0;
 	}
 	else{
 		if(sp->size==1){
 			free(sp->head->pt);
 			free(sp->head->next);
 			free(sp->head);
 		}
 		else{
 			struct points *node=sp->head;
 			int i=0;
 			while(i<index-1){
 				node=node->next;
 				i++;
 			}
 			struct points* tmp=node->next;
 			node->next=node->next->next;
 			free(tmp->pt);
 			free(tmp->next);
 			free(tmp);
 		}
 		sp->size-=1;
 		return 1;
 	}

}

int
sp_delete_duplicates(struct sorted_points *sp)
{
	TBD();
	return -1;
}
