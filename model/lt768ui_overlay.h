#ifndef LT768UI_OVERLAY_H
#define LT768UI_OVERLAY_H

#include "lt768ui_graphics.h"

typedef enum overlay_type_t
{
	LAY_SHOW_LEFT = SCREEN_0,
	LAY_SHOW_RIGHT = SCREEN_1,
	LAY_BACK = SCREEN_2,
	LAY_WIDGET0 = SCREEN_3,
	LAY_WIDGET1 = SCREEN_4,
	LAY_WIDGET2 = SCREEN_5,
	LAY_WIDGET_SWAP = SCREEN_6,
}overlay_type_t;

void overlay_init();
void overlay_refresh();
void overlay_load_bag(dma_info_t *dma_info);
void overlay_empty_control();
void overlay_load_widget(dma_info_t *dma_info, int pri);
void overlay_call_refresh();
void overlay_empty_rect(int start_x, int start_y, int w, int h, int pri);
void overlay_fill_rect(int start_x, int start_y, int w, int h, uint16_t color, int pri);
int overlay_add_preload(dma_info_t *dma_info);
void overlay_preload_cpy_key(int preload, int x, int y);
void overlay_preload_cpy(int preload, int x, int y);

#endif