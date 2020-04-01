#ifndef LT768UI_TOUCH_H
#define LT768UI_TOUCH_H

#include "FT6X36.h"
#include "lt768ui_graphics.h"
struct touch_info_t;
typedef bool (*touch_judge_t)(struct touch_info_t * , tp_info_t *);
typedef void (*touch_cb_t)(tp_info_t *);


typedef enum
{
	NO_SHAPE = 0,
	SHAPE_RECT,
	SHAPE_CIRCLE,

} judge_shape_t;

typedef struct{
	int start_x;
	int start_y;
	int end_x;
	int end_y;
} rect_t;

typedef struct{
	int center_x;
	int center_y;
	int r;
} circle_t;

typedef struct touch_info_t{
	touch_judge_t judge;
	touch_cb_t cb;
	int screen;
	int shape;
	void *ptr;
	struct touch_info_t *next;
} touch_info_t;

void touch_driver_init();
void touch_trigger();
void touch_cb_register(touch_info_t *touch_info);
bool judeg_shape_normal(touch_info_t *touch_info, tp_info_t *tp_info);
bool judge_slide(touch_info_t *touch_info, tp_info_t *tp_info);

#endif