#ifndef LT768UI_TEXT_H
#define LT768UI_TEXT_H

#include "lt768ui_widget.h"

#define TEXT_LEN 15

typedef int (*enumpic_cb_t)(void);

typedef struct text_t{
	widget_common_t common;
	int x;
	int y;
	int size;
	uint16_t color;
	char str[TEXT_LEN];
	char *new_str;
	int pri;
} text_t;

text_t *build_text(int size, int color, int x, int y, char *str, int pri);

#endif