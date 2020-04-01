#include "lt768ui_data.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

static int g_screen_count = -1;


void load_dma_info(dma_info_t *dma_info)
{			   
	struct timeval start;
	STD_LOGD("start to dma, w: %d, h: %d", dma_info->w,dma_info->h);
    gettimeofday(&start, NULL);
	Enable_SFlash_SPI();
	Goto_Pixel_XY(dma_info->x, dma_info->y);							// 在图形模式中设置内存的位置
	SFI_DMA_Destination_Upper_Left_Corner(dma_info->x,dma_info->y);		// DMA传输的目的地（内存的位置）
	SFI_DMA_Transfer_Width_Height(dma_info->w,dma_info->h);				// 设置块数据的宽度和高度
	SFI_DMA_Source_Width(dma_info->src_w);								// 设置源数据的宽度
	SFI_DMA_Source_Start_Address(dma_info->addr); 						// 设置源数据在Flash的地址
	Start_SFI_DMA();													// 开始DMA传输
	lt_wait_trig(LT_DMA);												// 检测DMA是否传输完成
	
	Disable_SFlash_SPI();

    struct timeval end;
    gettimeofday(&end, NULL);

    struct timeval elapsed;
    timersub(&end, &start, &elapsed);
    float s = elapsed.tv_sec + (float)elapsed.tv_usec/1000000;
	
    STD_LOGD("dma success, time : %5.2f", s);
}

void ui_data_init()
{
	w25q_init();
	w25q_mode(STD);
	w25q_deint();
	Select_SFI_0();						
	Select_SFI_DMA_Mode();							
	Select_SFI_24bit_Address();
	Select_SFI_Dual_Mode0();
	SPI_Clock_Period(0);
	STD_LOGI("ui data init success");
}

