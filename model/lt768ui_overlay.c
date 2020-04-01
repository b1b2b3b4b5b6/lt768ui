#include "lt768ui_overlay.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_DEBUG
#define KEY_COLOR 0
#define PRELOAD_NUM 20
 
int g_now_show = LAY_SHOW_LEFT;
bool g_switch_show;

typedef struct preload_t{
	uint64_t addr;
	int w;
	int h;
} preload_t;

preload_t g_preload_list[PRELOAD_NUM] = {{0}};


void overlay_init()
{
	graphics_init();
	g_switch_show = false;
}

static int get_next_show()
{
	if(g_now_show == LAY_SHOW_LEFT)
		return LAY_SHOW_RIGHT;
	else
		return LAY_SHOW_LEFT;
}

static void switch_show()
{
	g_now_show = get_next_show();
	show_screen(g_now_show);
}

static void compose_show()
{
	cpy_screen(LAY_BACK, get_next_show());
	cpy_screen_key(LAY_WIDGET0, get_next_show(), KEY_COLOR);
	cpy_screen_key(LAY_WIDGET1, get_next_show(), KEY_COLOR);
	//cpy_screen_key(LAY_WIDGET2, get_next_show(), KEY_COLOR);
}

void overlay_refresh()
{	
	if(g_switch_show)
	{
		compose_show();
		switch_show();
		g_switch_show = false;
	}
}

void overlay_call_refresh()
{
	g_switch_show = true;
}

void overlay_empty_control()
{
	fill_screen(LAY_WIDGET0, KEY_COLOR);
	fill_screen(LAY_WIDGET1, KEY_COLOR);
	//fill_screen(LAY_WIDGET2, KEY_COLOR);
}

void overlay_load_widget(dma_info_t *dma_info, int pri)
{
	handle_screen(pri);
	load_dma_info(dma_info);
	g_switch_show = true;
}

void overlay_load_bag(dma_info_t *dma_info)
{
	handle_screen(LAY_BACK);
	load_dma_info(dma_info);
	g_switch_show = true;
	handle_screen(LAY_WIDGET0);
}

void overlay_empty_rect(int start_x, int start_y, int w, int h, int pri)
{
	handle_screen(pri);
	LT768_DrawSquare_Fill(start_x, start_y, start_x + w, start_y + h, KEY_COLOR);
	g_switch_show = true;
}

void overlay_fill_rect(int start_x, int start_y, int w, int h, uint16_t color, int pri)
{
	handle_screen(pri);
	LT768_DrawSquare_Fill(start_x, start_y, start_x + w, start_y + h, color);	
	g_switch_show = true;
}

int overlay_add_preload(dma_info_t *dma_info)
{
	uint64_t offset = 0;
	uint32_t len;
	for (int n = 0; n < PRELOAD_NUM; n++)
	{
		if(g_preload_list[n].addr == 0)
		{
			handle_screen(LAY_WIDGET_SWAP);
			change_canvas_width(dma_info->w);
			dma_info->x = 0;
			dma_info->y = 0;
			load_dma_info(dma_info);
			resume_canvas_width();
			g_preload_list[n].addr = offset + get_screen_addr(LAY_WIDGET_SWAP);
			g_preload_list[n].w = dma_info->w;
			g_preload_list[n].h = dma_info->h;

			len = dma_info->w * dma_info->h * 2;
			offset += len;
			return n;
		}
	}
	STD_END("preload num overflow");
	return -1;
}

void overlay_preload_cpy(int preload, int x, int y)
{
	LT768_BTE_Memory_Copy(
		g_preload_list[preload].addr,
		g_preload_list[preload].w,
		0,
		0,
		0,
		0,
		0,
		0,
		get_screen_addr(LAY_WIDGET0),
		get_image_width(),
		x,
		y,
		0x0c,
		g_preload_list[preload].w,
		g_preload_list[preload].h);
	g_switch_show = true;
}

void overlay_preload_cpy_key(int preload, int x, int y)
{
	LT768_BTE_Memory_Copy_Chroma_key(
		g_preload_list[preload].addr,
		g_preload_list[preload].w,
		0,
		0,
		get_screen_addr(LAY_WIDGET0),
		get_image_width(),
		x,
		y,
		KEY_COLOR,
		g_preload_list[preload].w,
		g_preload_list[preload].h);
	g_switch_show = true;
}