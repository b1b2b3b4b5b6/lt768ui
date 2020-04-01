#include "lt768ui_text.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

static void  text_fresh_status(void *ptr)
{
	text_t *text = ptr;
	strcpy(text->str, text->new_str);
	handle_screen(text->pri);
	LT768_Select_Internal_Font_Init(text->size, 1, 1, 0, 0);
	overlay_empty_rect(text->x, text->y, 100, 50, text->pri);
	LT768_Print_Internal_Font_String(text->x, text->y, text->color, 0, text->str);
}	

static void text_check(void *ptr)
{
	text_t *text = ptr;
	if(strcmp(text->str, text->new_str))
	{
		text_fresh_status(text);
	}
}


text_t *build_text(int size, int color, int x, int y, char *str, int pri)
{
	text_t *text = STD_CALLOC(sizeof(text_t), 1);
	text->size = size;
	text->color = color;
	text->x = x;
	text->y = y;
	text->new_str = str;
	text->common.widget_status = text_fresh_status;
	text->common.widget_check = text_check;
	text->pri = pri;
	return text;
}