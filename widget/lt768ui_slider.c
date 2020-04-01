#include "lt768ui_slider.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO


static void slider_fresh_status(void *ptr)
{
	slider_t *slider = ptr;
	int x, y;

	slider->back->x = slider->x;
	slider->back->y = slider->y;
	//overlay_preload_cpy(slider->pre_back, slider->x, slider->y);
	overlay_load_widget(slider->back, slider->pri);

	slider->value = *(slider->new_value);

	x = slider->x;
	y = slider->center_y - slider->back_thickness / 2;
	overlay_fill_rect(x, y, slider->value, slider->back_thickness, slider->back_color, slider->pri);

	dma_info_t *dma_info = slider->block;
	x = slider->x + slider->value;
	y = slider->center_y;
	x = x - dma_info->w / 2;
	y = y - dma_info->h / 2;
	dma_info->x = x;
	dma_info->y = y;
	//load_dma_info(dma_info);
	overlay_preload_cpy_key(slider->pre_block, x, y);

}

static void slider_check(void *ptr)
{
	slider_t *slider = ptr;
	if(*(slider->new_value) != slider->value)
	{
		slider_fresh_status(slider);
	}
}


static int slider_judge(void *ptr, tp_info_t *tp_info)
{
	slider_t *slider = ptr;
	if(slider->active == false)
	{
		if(tp_info->event != PRESS_DOWN)
			return -1;

		if(judge_widget_shape(slider->shape, slider->shape_ptr, tp_info))
		{
			*(slider->new_value) = tp_info->x - slider->back->x;
			slider_fresh_status(slider);
			slider->exec();
			slider->active = true;
			return 0;
		}
		else
			return -1;
	}
	else
	{
		if(judge_widget_shape(slider->shape, slider->shape_ptr, tp_info))
		{
			*(slider->new_value) = tp_info->x - slider->back->x;
			slider_fresh_status(slider);
			slider->exec();
		}

		if(tp_info->event == LIFT_UP)
		{
			slider->active = false;
		}

		return 0;
	}
}

void slider_init(void *ptr)
{
	slider_t *slider = ptr;
	slider->pre_block = overlay_add_preload(slider->block);
	//slider->pre_back = overlay_add_preload(slider->back);	
}

slider_t *build_slider(uint16_t color, dma_info_t *back, dma_info_t *block, int back_thickness, int x, int y, int *new_value, void (*exec)(void), int pri)
{
	slider_t *slider = STD_CALLOC(sizeof(slider_t), 1);
	if(back->w < block->w)
		STD_END("slider back and block width illgal");
	slider->shape = SHAPE_RECT;
	slider->shape_ptr = build_sharp(SHAPE_RECT, x, y, back->w, back->h);
	slider->active = false;
	slider->back_color = color;
	slider->back = back;
	slider->block = block;
	slider->new_value = new_value;
	slider->common.widget_judge = slider_judge;
	slider->common.widget_check = slider_check;
	slider->common.widget_status = slider_fresh_status;
	slider->common.widget_init = slider_init;
	slider->x = x;
	slider->y = y;
	slider->back_thickness = back_thickness;
	slider->center_y = y + back->h / 2;
	slider->pri = pri;
	slider->exec = exec;
	return slider;
}