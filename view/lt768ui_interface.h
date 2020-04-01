#ifndef LT768UI_INTERFACE_H
#define LT768UI_INTERFACE_H

#include "lt768ui_overlay.h"
#include "lt768ui_touch.h"

#include "lt768ui_button.h"
#include "lt768ui_slider.h"
#include "lt768ui_checkbox.h"
#include "lt768ui_enumpic.h"
#include "lt768ui_text.h"

#define WIDGET_MAX 20
#define INTERFACE_MAX 8

typedef struct interface_t{
	int id;
	dma_info_t *bag;
	widget_t *widget_list[WIDGET_MAX];
} interface_t;

interface_t *get_now_interface();

interface_t *build_interface(int id, dma_info_t *bag);
void resgister_interface(interface_t *interface);
interface_t *get_interface_by_id(int id);

void interface_add_widget(interface_t *interface, widget_t *widget);
void interface_triger(tp_info_t *tp_info);
void interface_jump(int id);
void interface_init();

#endif