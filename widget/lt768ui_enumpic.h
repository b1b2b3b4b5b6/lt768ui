#ifndef LT768UI_ENUMPIC_H
#define LT768UI_ENUMPIC_H

#include "lt768ui_widget.h"
#define ENUMPIC_CASE_NUM 15

typedef int (*enumpic_cb_t)(void);

typedef struct enumpic_t{
	widget_common_t common;
	int x;
	int y;
	int index;
	dma_info_t *case_list[ENUMPIC_CASE_NUM];
	int *new_index;
	int pri;
} enumpic_t;

enumpic_t *build_enumpic(int x, int y, int *p_index, int pri);
void enumpic_add_case(enumpic_t *enumpic, dma_info_t *dma_info);

#endif
