#ifndef LT768UI_GRAPHICS_H
#define LT768UI_GRAPHICS_H

#define SCROLL_VERTICAL 1

#include "LT768_Lib.h"
#include "lt768ui_data.h"
#include "lt_trig.h"

#define SCREEN_NUM 8

typedef enum
{
	SCREEN_0 = 0,
	SCREEN_1 = 1,
	SCREEN_2 = 2,
	SCREEN_3 = 3,
	SCREEN_4 = 4,
	SCREEN_5 = 5,
	SCREEN_6 = 6,
	SCREEN_7 = 7,
	SCREEN_ALL = 0xffff,
} screen_num_t;

typedef struct {
    uint64_t addr;
    int src_x;
    int src_y;
    int dis_x;
    int dis_y;
    int dis_w;
    int dis_h;
} pip_info_t;
int get_pre_screen();
int get_next_screen();
int get_screen_count();
int get_screen();
bool is_vertcial();
void graphics_init();
void jump_to_screen(int screen_type);
void pip_set(int pip, pip_info_t *info);
void pip_enable(int pip);
void pip_disable(int pip);
void show_screen(int screen_dst);
void resume_screen();
void handle_screen(int screen_type);
void scroll_to_screen(int screent_dst);
void set_scroll_vertical(bool is_vertcial);
int get_image_width();
void pip_test();
void memeory_write_test();
void singal_check();

void cpy_screen(int src, int dst);
void cpy_screen_key(int src, int dst, uint16_t key);
void fill_screen(int screen, uint16_t color);

void resume_canvas_width();
void change_canvas_width(int width);
uint64_t get_screen_addr(int screen_type);

#endif