/*
 * CFile1.c
 *
 * Created: 4/21/2017 11:12:37 PM
 *  Author: Tomasko
 */ 
#include "tlak.h"

int tlak() 
{
	if((PINB & (1<<2))) {
		return 0;
	} else {
		return 1;
	}
	return -1;
}