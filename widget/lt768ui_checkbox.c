#include "lt768ui_checkbox.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

static void checkbox_fresh_status(void *ptr)
{
	checkbox_t *checkbox = ptr;
	dma_info_t *dma_info;
	if(checkbox->status)
		dma_info = checkbox->dma_on;
	else
		dma_info = checkbox->dma_off;
	
	dma_info->x = checkbox->x;
	dma_info->y = checkbox->y;
	overlay_load_widget(dma_info, checkbox->pri);
}

static void checkbox_check(void *ptr)
{
	checkbox_t *checkbox = ptr;
	if(checkbox->new_status == NULL)
		return;

	if(checkbox->status != *(checkbox->new_status))
	{
		checkbox->status = *(checkbox->new_status);
		checkbox_fresh_status(checkbox);
	}
}

static void release_success(checkbox_t *checkbox)
{
	if(checkbox->status)
		checkbox->status = false;
	else
		checkbox->status = true;
	if(checkbox->exec_cb != NULL)
		checkbox->exec_cb(checkbox->status);
	*(checkbox->new_status) = checkbox->status;

	checkbox_fresh_status(checkbox);
}

static void release_fail(checkbox_t *checkbox)
{

}


static int checkbox_judge(void *ptr, tp_info_t *tp_info)
{
	checkbox_t *checkbox = ptr;
	if(checkbox->active ==false)
	{
		if(tp_info->event != PRESS_DOWN)
			return -1;

		if(judge_widget_shape(checkbox->shape, checkbox->shape_ptr, tp_info))
		{
			checkbox->active = true;
			return 0;
		}
		else
			return -1;
	}

	if(tp_info->event == LIFT_UP)
	{
		if(judge_widget_shape(checkbox->shape, checkbox->shape_ptr, tp_info))
			release_success(checkbox);
		else
			release_fail(checkbox);
		checkbox->active = false;
	}
	return 0;
}

checkbox_t *build_checkbox(int shape, dma_info_t *dma_on, dma_info_t *dma_off, int x, int y, void (*exec_cb)(bool), bool *new_status, int pri)
{
	checkbox_t *checkbox = STD_CALLOC(sizeof(checkbox_t), 1);
	if(dma_on->w != dma_off->w || dma_on->h != dma_off->h)
		STD_END("dma_info not equal");
	checkbox->shape = shape;
	checkbox->shape_ptr = build_sharp(shape, x, y, dma_on->w, dma_on->h);
	checkbox->x = x;
	checkbox->y = y;
	checkbox->dma_on = dma_on;
	checkbox->dma_off = dma_off;
	checkbox->new_status = new_status;
	checkbox->status = *new_status;
	checkbox->exec_cb = exec_cb;
	checkbox->active = false;
	checkbox->common.widget_check = checkbox_check;
	checkbox->common.widget_status = checkbox_fresh_status;
	checkbox->common.widget_judge = checkbox_judge;
	checkbox->pri = pri;
	return checkbox;
}