#include "lt768ui_widget.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

static bool judge_rect(rect_t *rect, tp_info_t *tp_info)
{
	if(tp_info->x <= rect->end_x && tp_info->x >= rect->start_x && tp_info->y <= rect->end_y && tp_info->y >= rect->start_y)
		return true;
	return false;
}

static bool judge_circle(circle_t *circle, tp_info_t *tp_info)
{
	int32_t sum = 0;
	int32_t temp;
	temp = tp_info->x - circle->center_x;
	temp *= temp;
	sum += temp;

	temp = tp_info->y- circle->center_y;
	temp *= temp;
	sum += temp;
	
	temp = circle->r;
	temp *= temp;
	if(sum <= temp)
		return true;
	else
		return false;
}


bool judge_widget_shape(int shape, void *shape_ptr, tp_info_t *tp_info)
{
	switch(shape)
	{
		case SHAPE_RECT:
			return judge_rect((rect_t *)shape_ptr, tp_info);
			break;

		case SHAPE_CIRCLE:
			return judge_circle((circle_t *)shape_ptr, tp_info);
			break;
		
		default :
			STD_END("undefined shape: %d", shape);
			break;
	}
	return false;
}

widget_t *build_widget(int type, void *widget_ptr)
{
	widget_t *widget = STD_CALLOC(sizeof(widget_t), 1);
	widget->widget_type = type;
	switch(type)
	{
		case WIDGET_BUTTON:
			widget->reflect_type = REFLECT_ACTIVE;
			break;
			
		case WIDGET_SLIDER:
			widget->reflect_type = REFLECT_ACTIVE;
			break;

		case WIDGET_SWITCH:
			widget->reflect_type = REFLECT_ACTIVE;
			break;
		
		case WIDGET_ENUMPIC:
			widget->reflect_type = REFLECT_INACTIVE;
			break;

		case WIDGET_CHECKBOX:
			widget->reflect_type = REFLECT_ALL;
			break;

		case WIDGET_TEXT:
			widget->reflect_type = REFLECT_INACTIVE;
			break;

		default :
			STD_END("undefine widget type");
			break;
	}

	widget->common = widget_ptr;
	if(widget->common->widget_init)
		widget->common->widget_init(widget->common);

	return widget;
}

void *build_sharp(int shape, int x, int y, int w, int h)
{
	rect_t *rect;
	circle_t *circle;
	switch(shape)
	{
		case SHAPE_RECT:
			rect = STD_CALLOC(sizeof(rect_t), 1);
			rect->start_x = x;
			rect->start_y = y;
			rect->end_x = x + w;
			rect->end_y = y + h;
			return rect;
			break;

		case SHAPE_CIRCLE:
			circle = STD_CALLOC(sizeof(circle_t), 1);
			circle->center_x = x + w / 2;
			circle->center_y = y + h / 2;
			circle->r = (w + h) / 4;
			return circle;
			break;
		
		default :
			STD_END("undefined shape: %d", shape);
			break;		
	}
	return NULL;
}

uint16_t get_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t v = 0;
	uint16_t temp = 0;

	temp = r;
	v|=((temp>>3)<<11);
	temp = g;
	v|=((temp>>2)<<5);
	temp = b;
	v|=((temp>>3)<<0);

	return v;
}
