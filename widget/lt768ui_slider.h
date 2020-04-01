#ifndef LT768UI_SLIDER_H
#define LT768UI_SLIDER_H

#include "lt768ui_widget.h"

typedef struct slider_t{
	widget_common_t common;
	int x;
	int y;
	int shape;
	void *shape_ptr;
	bool active;
	int *new_value;
	int value;
	int center_x;
	int center_y;
	dma_info_t *back;
	dma_info_t *block;
	int back_thickness;
	uint16_t back_color;
	int pre_back;
	int pre_block;
	int pri;
	void (*exec)(void);
} slider_t;

slider_t *build_slider(uint16_t color, dma_info_t *back, dma_info_t *block, int back_thickness, int x, int y, int *new_value, void (*exec)(void), int pri);

#endif