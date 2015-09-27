#include <assert.h>
#include "common.h"
#include "point.h"
#include "math.h"

void
point_translate(struct point *p, double x, double y)
{
	//TBD();
	p->x+=x;
	p->y+=y;
}

double
point_distance(const struct point *p1, const struct point *p2)
{
	//TBD();
	double  disX,disY,dis;
	disX=(p1->x-p2->x)*(p1->x-p2->x);
	disY=(p1->y-p2->y)*(p1->y-p2->y);
	dis=sqrt(disX+disY);
	return dis;
}

int
point_compare(const struct point *p1, const struct point *p2)
{
	double d1=(p1->x)*(p1->x)+(p1->y)*(p1->y);
	double d2=(p2->x)*(p2->x)+(p2->y)*(p2->y);
	if(d1==d2)
		return 0;
	else if(d1<d2)
		return -1;
	else
		return 1;
}
