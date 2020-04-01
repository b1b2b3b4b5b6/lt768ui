#ifndef LT768UI_BUTTON_H
#define LT768UI_BUTTON_H

#include "lt768ui_widget.h"

typedef struct button_t{
	widget_common_t common;
	int x;
	int y;
	bool active;
	int shape;
	void *shape_ptr;
	dma_info_t *normal;
	dma_info_t *press;
	void (*exec_hook)(void);
	int pri;
} button_t;

button_t *build_button(int shape, dma_info_t *normal, dma_info_t *press, int x, int y, void (*exec_hook)(void), int pri);

#endif