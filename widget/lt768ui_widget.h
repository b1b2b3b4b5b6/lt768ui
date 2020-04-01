#ifndef LT768UI_WIDGET_H
#define LT768UI_WIDGET_H

#include "lt768ui_touch.h"
#include "lt768ui_data.h"
#include "lt768ui_overlay.h"
typedef enum 
{
	WIDGET_BUTTON = 1,
	WIDGET_SLIDER,
	WIDGET_SWITCH,
	WIDGET_ENUMPIC,
	WIDGET_CHECKBOX,
	WIDGET_TEXT,
} widget_type_t;

typedef enum 
{
	REFLECT_INACTIVE = 1,
	REFLECT_ACTIVE,
	REFLECT_ALL,
} reflect_type_t;

typedef void (*widget_check_t)(void *);
typedef void (*widget_status_t)(void *);
typedef int (*widget_judge_t)(void *, tp_info_t *);
typedef void (*widget_init_t)(void *);

typedef struct widget_common_t{
	widget_status_t widget_status;
	widget_check_t widget_check;
	widget_judge_t widget_judge;
	widget_init_t widget_init;
} widget_common_t;


typedef struct widget_t{
	int widget_type;
	int reflect_type;
	widget_common_t *common;
} widget_t;

bool judge_widget_shape(int shape, void *shape_ptr, tp_info_t *tp_info);
widget_t *build_widget(int type, void *widget_ptr);
void *build_sharp(int shape, int x, int y, int w, int h);
uint16_t get_rgb565(uint8_t r, uint8_t g, uint8_t b);
#endif