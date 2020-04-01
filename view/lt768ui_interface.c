#include "lt768ui_interface.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_DEBUG

interface_t *g_interface_list[INTERFACE_MAX] = {0};
interface_t *g_now_interface;

interface_t *get_now_interface()
{
	return g_now_interface;
}

interface_t *get_interface_by_id(int id)
{
	for (int n = 0; n < INTERFACE_MAX; n++)
	{
		if(g_interface_list[n] == NULL)
			goto FAIL;
		
		if(g_interface_list[n]->id == id)
			return g_interface_list[n];
	}

FAIL:
	STD_END("udefined interface id");
	return NULL;
}

interface_t *build_interface(int id, dma_info_t *bag)
{
	interface_t *interface = STD_CALLOC(sizeof(interface_t), 1);
	interface->id = id;
	interface->bag = bag;
	return interface;
}

void interface_triger(tp_info_t *tp_info)
{
	interface_t *temp_interface = get_now_interface();
	if(tp_info != NULL)
	{
		for (int n = 0; n < WIDGET_MAX; n++)
		{
			if(g_now_interface->widget_list[n] == NULL)
				break;
			if(g_now_interface->widget_list[n]->common->widget_judge)
			{
				if(g_now_interface->widget_list[n]->common->widget_judge(
					g_now_interface->widget_list[n]->common,
					tp_info) == 0)
					break;
			}

		}
	}

	if(temp_interface != get_now_interface())
		return;

	for (int n = 0; n < WIDGET_MAX; n++)
	{
		if(g_now_interface->widget_list[n] == NULL)
			break;
			if(g_now_interface->widget_list[n]->common->widget_check)
		{
			g_now_interface->widget_list[n]->common->widget_check(
				g_now_interface->widget_list[n]->common);
		}

	}
}

void interface_add_widget(interface_t *interface, widget_t *widget)
{
	for (int n = 0; n < WIDGET_MAX; n++)
	{
		if(interface->widget_list[n] == NULL)
		{
			interface->widget_list[n] = widget;
			STD_LOGI("add active widget: %d", widget->widget_type);
			return;
		}
	}

	STD_END("add widget num overflow");
}

void resgister_interface(interface_t *interface)
{
	for (int n = 0; n < INTERFACE_MAX; n++)
	{
		if(g_interface_list[n] == NULL)
		{
			g_interface_list[n] = interface;
			STD_LOGI("register interface : %d", interface->id);
			return;
		}
	}
	STD_END("interface num overflow");
}

void interface_jump(int id)
{
	g_now_interface = get_interface_by_id(id);
	overlay_load_bag(g_now_interface->bag);
	widget_t *widget;
	overlay_empty_control();

	for (int n = 0; n < WIDGET_MAX; n++)
	{
		if(g_now_interface->widget_list[n] == NULL)
			break;
		widget = g_now_interface->widget_list[n];
		widget->common->widget_status(widget->common);
	}
	STD_LOGI("jump to interface : %d", id);
}

void interface_init()
{
	overlay_init();
	ui_data_init();
	touch_driver_init();
	STD_LOGI("interface init success");
}