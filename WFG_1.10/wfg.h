#ifndef _WFG_H_
#define _WFG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef double OBJECTIVE;

#undef POINT

typedef struct
{
	OBJECTIVE *objectives;
} Point;

typedef struct
{
	int nPoints;
	int n;
	Point *points;
} FRONT;

typedef struct
{
	int nFronts;
	FRONT *fronts;
} FILECONTENTS;

FILECONTENTS *readFile(char[]);

extern void printContents(FILECONTENTS *);

#endif
