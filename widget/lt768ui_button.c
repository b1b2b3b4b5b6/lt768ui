#include "lt768ui_button.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

static void press(button_t *button)
{
	button->press->x = button->x;
	button->press->y = button->y;
	overlay_load_widget(button->press, button->pri);
}

static void release_success(button_t *button)
{
	button->normal->x = button->x;
	button->normal->y = button->y;
	overlay_load_widget(button->normal,button->pri);
	if(button->exec_hook)
		button->exec_hook();
}

static void release_fail(button_t *button)
{
	button->normal->x = button->x;
	button->normal->y = button->y;
	overlay_load_widget(button->normal,button->pri);
}

static void fresh_status(void *ptr)
{
	button_t *button = ptr;
	button->normal->x = button->x;
	button->normal->y = button->y;
	overlay_load_widget(button->normal,button->pri);
}

static int button_judge(void *ptr, tp_info_t *tp_info)
{
	button_t *button = ptr;
	if(button->active ==false)
	{
		if(tp_info->event != PRESS_DOWN)
			return -1;

		if(judge_widget_shape(button->shape, button->shape_ptr, tp_info))
		{
			press(button);
			button->active = true;
			return 0;
		}
		else
			return -1;
	}

	if(tp_info->event == LIFT_UP)
	{
		if(judge_widget_shape(button->shape, button->shape_ptr, tp_info))
			release_success(button);
		else
			release_fail(button);
		button->active = false;
	}
	return 0;
}

button_t *build_button(int shape, dma_info_t *normal, dma_info_t *press, int x, int y, void (*exec_hook)(void),  int pri)
{
	button_t *button = STD_CALLOC(sizeof(button_t), 1);
	button->shape = shape;
	button->shape_ptr = build_sharp(shape, x, y, normal->w, normal->h);
	button->common.widget_judge = button_judge;
	button->common.widget_status = fresh_status;
	button->normal = normal;
	button->press = press;
	if(normal->w != press->w || normal->h != press->h)
		STD_END("dma_info not equal");
	button->x = x;
	button->y = y;
	button->exec_hook = exec_hook;
	button->active = false;
	button->pri = pri;
	return button;
}