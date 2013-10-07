#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>

extern int imin (int a, int b)
{
	return (a < b ? a : b);
}

extern int imax (int a, int b)
{
	return (a > b ? a : b);
}

#endif