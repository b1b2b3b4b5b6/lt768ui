#include "lt768ui_enumpic.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

static void enumpic_fresh_status(void *ptr)
{
	enumpic_t *enumpic = ptr;
	enumpic->index = *(enumpic->new_index);
	dma_info_t *dma_info = enumpic->case_list[enumpic->index];
	dma_info->x = enumpic->x;
	dma_info->y = enumpic->y;
	overlay_load_widget(dma_info, enumpic->pri);
}

static void enumpic_check(void *ptr)
{
	enumpic_t *enumpic = ptr;
	if(*(enumpic->new_index) != enumpic->index)
	{
		enumpic_fresh_status(enumpic);
	}
}

enumpic_t *build_enumpic(int x, int y, int *p_index, int pri)
{
	enumpic_t *enumpic = STD_CALLOC(sizeof(enumpic_t), 1);
	enumpic->x = x;
	enumpic->y = y;
	enumpic->new_index = p_index;
	enumpic->common.widget_status = enumpic_fresh_status;
	enumpic->common.widget_check = enumpic_check;
	enumpic->pri = pri;
	return enumpic;
}

void enumpic_add_case(enumpic_t *enumpic, dma_info_t *dma_info)
{
	for (int n = 0; n < ENUMPIC_CASE_NUM; n++)
	{
		if(enumpic->case_list[n] == NULL)
		{
			enumpic->case_list[n] = dma_info;
			enumpic->index = n;
			return;
		}
	}
	STD_END("enumpic num overflow");
}

