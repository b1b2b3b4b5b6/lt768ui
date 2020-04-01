#ifndef LT_TRIG_H
#define LT_TRIG_H

#include "LT768_Lib.h"
#include "lt768ui_config.h"

typedef enum lt_trig_t
{
	LT_DMA = 1,
} lt_trig_t;

void lt_wait_trig(int type);
void lt_trig_init();

#endif