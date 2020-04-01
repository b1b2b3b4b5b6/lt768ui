#ifndef FT6X36_H
#define FT6X36_H

#include "std_common.h"
#include "std_port_common.h"
#include "lt768ui_config.h"

typedef struct {
	
    int x;
    int y;
    int event;
	int gest_id;
	int offset_x;
	int offset_y;
	//uint8_t touch_id;
} tp_info_t;

typedef enum
{
    TP_NO_GESTRE = 0,
    TP_MOVE_UP = 0x10,
    TP_MOVE_RIGHT = 0x14,
    TP_MOVE_DOWN = 0x18,
    TP_MOVE_LEFT = 0x1c,
    TP_ZOOM_IN = 0x48,
    TP_ZOOM_OUT = 0x49
} gest_id_t;

typedef enum
{
    PRESS_DOWN = 0,
    LIFT_UP = 1,
    CONTACT = 2,
    NO_EVENT = 3,
}touch_event_t;

int ft6x36_init();
void enable_tp_message();
void disable_tp_message();
tp_info_t *tp_receive_message(int timeout_ms);
void tp_recevie_test();

#endif