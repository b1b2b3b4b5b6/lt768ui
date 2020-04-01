#include "lt768ui_loop.h"

#define FRESH_TIME 100
#define LOOP_TASK_SIZE 4096
#define LOOP_TASK_PRI (ESP_TASK_MAIN_PRIO + 1)

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

static bool g_run_status = true;

void lt768ui_loop_set_run(bool status)
{
	g_run_status = status;
	STD_LOGI("loop set status[%d]", status);
}

static void surface_loop(void *arg)
{
	tp_info_t *tp_info;
	for (;;)
	{
		while(g_run_status == false)
			vTaskDelay(2000 / portTICK_PERIOD_MS);
		tp_info = tp_receive_message(FRESH_TIME);
		interface_triger(tp_info);
		STD_FREE(tp_info);
		overlay_refresh();
		STD_LOGD("surface loop cycle");
	}
}

void lt768ui_loop_init()
{
	xTaskCreate(surface_loop, "surface loop", LOOP_TASK_SIZE, NULL, LOOP_TASK_PRI, NULL);
	STD_LOGI("lt768 loop task create");
}

#ifdef PRODUCT_TEST

void jump_test()
{
	STD_LOGI("jump test start");
	ui_data_init();
	int count = get_screen_count();
	for (int n = SCREEN_0; n < count; n++)
	{
		jump_to_screen(n);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void scroll_test()
{
	STD_LOGI("scroll test start");
	ui_data_init();
	scroll_to_screen(SCREEN_0);
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	scroll_to_screen(get_screen_count() - 1);
	vTaskDelay(3000 / portTICK_PERIOD_MS);
}

void surface_test()
{
	while(1)
	{
		STD_LOGI("set vertical");
		set_scroll_vertical(true);
		pip_test();
		jump_test();
		scroll_test();

		STD_LOGI("cancle vertical");
		set_scroll_vertical(false);
		pip_test();
		jump_test();
		scroll_test();
	}
}

void touch_test()
{
	while(1)
	{
		touch_trigger();
	}
}	

#endif