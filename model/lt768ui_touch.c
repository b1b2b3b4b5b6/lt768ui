#include "lt768ui_touch.h"

#define TRIGGER_MS 1000
static touch_info_t *g_touch_info_root = NULL;

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

void touch_cb_register(touch_info_t *touch_info)
{
	if(g_touch_info_root == NULL)
	{
		g_touch_info_root = touch_info;
		return;
	}
	touch_info_t *temp;
	temp = g_touch_info_root->next;
	for (;;)
	{
		if(temp->next == NULL)
		{
			temp->next = touch_info;
			break;
		}
	}
}

void touch_trigger()
{
	tp_info_t *tp_info;
	static touch_info_t *touch_press = NULL;
	tp_info = tp_receive_message(TRIGGER_MS);
	if(tp_info == NULL)
		return;

	STD_LOGD("trigger start");
	int screen_now = get_screen();
	touch_info_t *touch_info = g_touch_info_root;
	if(touch_info == NULL)
		return;

	if(tp_info->event != PRESS_DOWN)
	{
		STD_LOGD("trigger re enter");
		touch_press->cb(tp_info);
		if(tp_info->event == LIFT_UP)
		{
			STD_LOGD("reset triger");
			touch_press = NULL;
		}
		goto CLEAN;
	}
			
	for (;;)
	{
		if(touch_info->screen == screen_now)
		{
			STD_LOGD("test judge trigger");
			if(touch_info->judge(touch_info, tp_info))
			{
				STD_LOGD("screen: %d triggered", screen_now);
				touch_press = touch_info;
				touch_info->cb(tp_info);
				goto CLEAN;
			}
		}
		if(touch_info->next == NULL)
			break;
		else
			touch_info = touch_info->next;
	}

	for (;;)
	{
		if(touch_info->screen == SCREEN_ALL)
		{
			STD_LOGD("test slide judge trigger");
			if(touch_info->judge(touch_info, tp_info))
			{
				STD_LOGD("screen: %d , slide triggered", screen_now);
				touch_press = touch_info;
				touch_info->cb(tp_info);
				goto CLEAN;
			}
		}
		if(touch_info->next == NULL)
			break;
		else
			touch_info = touch_info->next;
	}

CLEAN:
	STD_LOGD("trigger end");
	STD_FREE(tp_info);
	return;
}


bool judeg_shape_normal(touch_info_t *touch_info, tp_info_t *tp_info)
{
	return false;
}


#define SLIDE_OFFSET_X 50
#define SLIDE_OFFSET_Y 50

bool judge_slide(touch_info_t *touch_info, tp_info_t *tp_info)
{
	return true;
}

#define FINISH 1
#define IDLE 0

void slide_cb(tp_info_t *tp_info)
{
	static int res  = IDLE;

	switch(tp_info->event)
	{
		case PRESS_DOWN:
			STD_LOGD("slide press down");
			break;
		
		case CONTACT:
			if(res != IDLE)
				return;
			int exec = -1;
			STD_LOGD("slide contact");
			if(tp_info->offset_x >= SLIDE_OFFSET_X)
				exec = TP_MOVE_RIGHT;
			if(tp_info->offset_x <= -SLIDE_OFFSET_X)
				exec = TP_MOVE_LEFT;
			if(tp_info->offset_y >= SLIDE_OFFSET_Y)
				exec = TP_MOVE_DOWN;
			if(tp_info->offset_y <= -SLIDE_OFFSET_Y)
				exec = TP_MOVE_UP;
			if(exec == -1)
				return;
			res = FINISH;
			STD_LOGD("slide contact finish: %d", exec);
			if(is_vertcial())
			{
				switch(exec)
				{
					case TP_MOVE_RIGHT:
						scroll_to_screen(get_pre_screen());
						return;

					case TP_MOVE_LEFT:
						scroll_to_screen(get_next_screen());
						return;

					default:
						return;
				}
			}
			else
			{
				switch(exec)
				{
					case TP_MOVE_DOWN:
						scroll_to_screen(get_pre_screen());
						return;

					case TP_MOVE_UP:
						scroll_to_screen(get_next_screen());
						return;

					default:
						return;
				}	
			}
			break;
		
		case LIFT_UP:
			STD_LOGD("slide lift up");
			res = IDLE;
			break;
	}

}

static void touch_register_init()
{
	touch_info_t *touch_info = calloc(1, sizeof(touch_info_t));
	touch_info->judge = judge_slide;
	touch_info->cb = slide_cb;
	touch_info->screen = SCREEN_ALL;
	touch_cb_register(touch_info);
}

void touch_driver_init()
{
	ft6x36_init();
}