#ifndef LT768UI_CHECKBOX_H
#define LT768UI_CHECKBOX_H

#include "lt768ui_widget.h"

typedef struct checkbox_t{
	widget_common_t common;
	int x;
	int y;
	bool status;
	bool *new_status;
	int shape;
	void *shape_ptr;
	bool active;
	dma_info_t *dma_on;
	dma_info_t *dma_off;
	void (*exec_cb)(bool);
	int pri;
} checkbox_t;

checkbox_t *build_checkbox(int shape, dma_info_t *dma_on, dma_info_t *dma_off, int x, int y, void (*exec_cb)(bool), bool *new_status, int pri);

#endif