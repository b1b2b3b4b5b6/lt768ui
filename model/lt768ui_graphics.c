#include "lt768ui_graphics.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

#define SCROLL_DELAY do{}while(0)
#define KEY_COLOR 0

int get_screen_count()
{
	return SCREEN_NUM;
}

typedef enum
{
    PIP_1 = 1,
    PIP_2,
}
pip_t;

static int g_screen = SCREEN_0;
static int g_image_width = 0;

bool is_vertcial()
{
	if(g_image_width > LCD_XSIZE_TFT)
		return true;
	else
		return false;
}	

int get_image_width()
{
	return g_image_width;
}

uint64_t get_screen_addr(int screen_type)
{
	if(is_vertcial())
		return LCD_XSIZE_TFT * screen_type * 2;
	else
		return LCD_XSIZE_TFT * LCD_YSIZE_TFT * screen_type * 2;   
}

pip_info_t pip_info = {0};

int get_screen()
{
    return g_screen;
}

int get_next_screen()
{
	int max_num = get_screen_count() - 1;
	if(g_screen < max_num)
		return g_screen + 1;
	else
		return max_num;
}

int get_pre_screen()
{
	if(g_screen == 0)
		return 0;
	else
		return g_screen - 1;
}

void assert_screen(int screen)
{
	int max = SCREEN_NUM - 1;
	if(screen > max)
	{
		STD_END("input screen: %d illegal", screen);
	}
}

void scroll_to_screen(int screen_dst)
{
	assert_screen(screen_dst);
	STD_LOGD("scroll to screen %d", screen_dst);
	int step;
	if(is_vertcial())
		step = 1 * 2;
	else
		step = 1 * LCD_XSIZE_TFT * 2;

	if(screen_dst == g_screen)
		return;
	if(screen_dst > g_screen)
		step = step;
	else
		step = -step;
	uint64_t start_addr = get_screen_addr(get_screen());
	uint64_t end_addr = get_screen_addr(screen_dst);
	for (;;)
	{
		start_addr += step;
		Main_Image_Start_Address(start_addr);
		if(start_addr == end_addr)
			break;
		SCROLL_DELAY;
	}
	Canvas_Image_Start_address(screen_dst);
	g_screen = screen_dst;
}

void jump_to_screen(int screen_dst)
{
	assert_screen(screen_dst);
	STD_LOGD("jump to screen %d", screen_dst);
    Main_Image_Start_Address(get_screen_addr(screen_dst));
    Canvas_Image_Start_address(get_screen_addr(screen_dst));
    g_screen = screen_dst;
}

void show_screen(int screen_dst)
{
	assert_screen(screen_dst);
	STD_LOGD("show screen %d", screen_dst);
	Main_Image_Start_Address(get_screen_addr(screen_dst));
}

void handle_screen(int screen_dst)
{
	assert_screen(screen_dst);
	STD_LOGD("handle screen %d", screen_dst);
    Canvas_Image_Start_address(get_screen_addr(screen_dst));
}

void resume_screen()
{
    Main_Image_Start_Address(get_screen_addr(get_screen()));
    Canvas_Image_Start_address(get_screen_addr(get_screen()));
}

#define CHECK_PIP(value)                         \
    do                                           \
    {                                            \
        if (value % 4 != 0)                      \
        {                                        \
            STD_LOGE("pip x,y is not mod by 4"); \
            vTaskDelay(portMAX_DELAY);           \
        }                                        \
    } while (0)

void pip_set(int pip, pip_info_t *info)
{
    CHECK_PIP(info->dis_x);
    CHECK_PIP(info->dis_y);
    CHECK_PIP(info->dis_w);
    CHECK_PIP(info->dis_h);

    switch(pip)
    {
        case PIP_1:
            Select_PIP1_Parameter();
            break;

        case PIP_2:
            Select_PIP2_Parameter();
            break;

        default :
            STD_LOGE("undefined pip");
            vTaskDelay(portMAX_DELAY);
            break;
    }

    PIP_Display_Start_XY(info->dis_x,info->dis_y);
	PIP_Image_Start_Address(info->addr);
	PIP_Window_Image_Start_XY(info->src_x,info->src_y);
	PIP_Window_Width_Height(info->dis_w,info->dis_h);
}

void pip_disable(int pip)
{
    switch(pip)
    {
        case PIP_1:
            Disable_PIP1();
            break;

        case PIP_2:
            Disable_PIP2();
            break;

        default :
            STD_LOGE("undefined pip");
            vTaskDelay(portMAX_DELAY);
            break;
    }
}

void pip_enable(int pip)
{
    switch(pip)
    {
        case PIP_1:
            Enable_PIP1();
            break;

        case PIP_2:
            Enable_PIP2();
            break;

        default :
            STD_LOGE("undefined pip");
            vTaskDelay(portMAX_DELAY);
            break;
    }
}

static void pip_init()
{
    Select_PIP1_Parameter();
    PIP_Image_Width(g_image_width);
    Select_PIP1_Window_16bpp();

    Select_PIP2_Parameter();
	PIP_Image_Width(g_image_width);
    Select_PIP2_Window_16bpp();
    
    pip_disable(PIP_1);
    pip_disable(PIP_2);
}


static void display_init(bool is_vertcial)
{
	if(is_vertcial)
		g_image_width = LCD_XSIZE_TFT * 8;
	else
		g_image_width = LCD_XSIZE_TFT;

	Main_Image_Width(g_image_width);
	Canvas_image_width(g_image_width);

	pip_init();
	Display_ON();
}

void resume_canvas_width()
{
	Canvas_image_width(g_image_width);
}

void change_canvas_width(int width)
{
	Canvas_image_width(width);
}

void set_scroll_vertical(bool is_vertcial)
{
	display_init(is_vertcial);
}


void cpy_screen(int src, int dst)
{
	LT768_BTE_Memory_Copy(
		get_screen_addr(src),
		get_image_width(),
		0,
		0,
		0,
		0,
		0,
		0,
		get_screen_addr(dst),
		get_image_width(),
		0,
		0,
		0x0c,
		LCD_XSIZE_TFT,
		LCD_YSIZE_TFT);
	
}

void cpy_screen_key(int src, int dst, uint16_t key)
{
	LT768_BTE_Memory_Copy_Chroma_key(
		get_screen_addr(src),
		get_image_width(),
		0,
		0,
		get_screen_addr(dst),
		get_image_width(),
		0,
		0,
		key,
		LCD_XSIZE_TFT,
		LCD_YSIZE_TFT);
}



void fill_screen(int screen, uint16_t color)
{
	BTE_Solid_Fill(get_screen_addr(screen), get_image_width(), 0, 0, color, LCD_XSIZE_TFT, LCD_YSIZE_TFT);
}

void graphics_init()
{
    LT768_Init();
	display_init(true);
	//lt_trig_init();
	for (int n = 0; n < SCREEN_NUM; n++)
	{
		handle_screen(n);
		LT768_DrawSquare_Fill(0,0,719,719,Blue);
	}
	jump_to_screen(SCREEN_0);
}

#ifdef PRODUCT_TEST
void pip_test()
{
	STD_LOGI("pip test start");
	handle_screen(SCREEN_0);
    LT768_DrawSquare_Fill(0,0,719,719,White);
    LT768_Print_Internal_Font_String(400, 0, 0, 0, (char *)"SCREEN_0");
    handle_screen(SCREEN_1);
    LT768_DrawSquare_Fill(0,0,719,719,Blue);
    LT768_Print_Internal_Font_String(400, 0, 0, 0, (char *)"SCREEN_1");
    handle_screen(SCREEN_2);
    LT768_DrawSquare_Fill(0,0,719,719,Magenta);
    LT768_Print_Internal_Font_String(400, 0, 0, 0, (char *)"SCREEN_2");
    handle_screen(SCREEN_3);
    LT768_DrawSquare_Fill(0,0,719,719,Green);
    LT768_Print_Internal_Font_String(400, 0, 0, 0, (char *)"SCREEN_3");
    handle_screen(SCREEN_4);
    LT768_DrawSquare_Fill(0,0,719,719,Cyan);
    LT768_Print_Internal_Font_String(400, 0, 0, 0, (char *)"SCREEN_4");
    handle_screen(SCREEN_5);
    LT768_DrawSquare_Fill(0,0,719,719,Grey);
    LT768_Print_Internal_Font_String(400, 0, 0, 0, (char *)"SCREEN_5");

    handle_screen(SCREEN_6);
    LT768_DrawSquare_Fill(0,0,719,719,Yellow);
    LT768_Print_Internal_Font_String(400, 0, 0, 0, (char *)"SCREEN_6");
    LT768_Print_Internal_Font_String(0, 0, 0, 0, (char *)"PIP 1");

    handle_screen(SCREEN_7);
    LT768_DrawSquare_Fill(0,0,719,719,Blue2);
    LT768_Print_Internal_Font_String(400, 0, 0, 0, (char *)"SCREEN_7");
    LT768_Print_Internal_Font_String(0, 0, 0, 0, (char *)"PIP 2");

    jump_to_screen(SCREEN_0);

    pip_info_t pip_info_temp ={0};
    pip_info_temp.addr = get_screen_addr(SCREEN_6);
    pip_info_temp.src_x = 0;
    pip_info_temp.src_y = 0;
    pip_info_temp.dis_x = 0;
    pip_info_temp.dis_y = 0;
    pip_info_temp.dis_w = 300;
    pip_info_temp.dis_h = 300;
    pip_set(PIP_1, &pip_info_temp);
    pip_enable(PIP_1);

    pip_info_temp.addr = get_screen_addr(SCREEN_7);
    pip_info_temp.src_x = 0;
    pip_info_temp.src_y = 0;
    pip_info_temp.dis_x = 300;
    pip_info_temp.dis_y = 300;
    pip_info_temp.dis_w = 300;
    pip_info_temp.dis_h = 300;
    pip_set(PIP_2, &pip_info_temp);
    pip_enable(PIP_2);

    for (int n = 0; n < 8; n++)
    {
        jump_to_screen(n);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    pip_info_temp.addr = get_screen_addr(SCREEN_7);
    pip_info_temp.src_x = 0;
    pip_info_temp.src_y = 0;
    pip_info_temp.dis_x = 0;
    pip_info_temp.dis_y = 0;
    pip_info_temp.dis_w = 300;
    pip_info_temp.dis_h = 300;
    pip_set(PIP_1, &pip_info_temp);

    pip_info_temp.addr = get_screen_addr(SCREEN_6);
    pip_info_temp.src_x = 0;
    pip_info_temp.src_y = 0;
    pip_info_temp.dis_x = 300;
    pip_info_temp.dis_y = 300;
    pip_info_temp.dis_w = 300;
    pip_info_temp.dis_h = 300;
    pip_set(PIP_2, &pip_info_temp);

    for (int n = 0; n < 8; n++)
    {
        jump_to_screen(n);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
	pip_disable(PIP_1);
	pip_disable(PIP_2);
}


void memeory_write_test()
{

	// int res;
	// uint16_t **image;
	// jump_to_screen(SCREEN_3);
	// res = decode_image(&image);
	// ESP_ERROR_CHECK(res);
	// for (int w = 0; w < 720; w++)
	// {
	// 	MPU8_16bpp_Memory_Write(0, w, 720, 1, (unsigned char *)image[w]);
	// 	free(image[w]);
	// }
	// free(image);
}

void singal_check()
{
	uint16_t temp = 1;
	LT768_DrawSquare_Fill(0, 0, 719, 719, 0);
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	for(int n = 0; n < 16; n++)
	{
		LT768_DrawSquare_Fill(45*n, 0, 719, 719, temp<<n);
	}
		
}

#endif