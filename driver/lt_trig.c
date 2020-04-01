#include "lt_trig.h"
#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

#define TASK_PRIO (ESP_TASK_MAIN_PRIO + 1)
#define TASK_SIZE 2048

static SemaphoreHandle_t lt_isr_sem;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	BaseType_t p;
	xSemaphoreGiveFromISR(lt_isr_sem, &p);
}

static int trig_init()
{
    gpio_config_t io_conf;
		//interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = 1ULL<<LT768_PIN_NUM_INIT;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    gpio_set_intr_type(LT768_PIN_NUM_INIT, GPIO_INTR_NEGEDGE);
	    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(LT768_PIN_NUM_INIT, gpio_isr_handler, NULL);
	return 0;
}

void lt_wait_trig(int type)
{
	switch(type)
	{
		case LT_DMA:
			Check_Busy_SFI_DMA();
			break;
	}
	return;
	
	int res;
	res = xSemaphoreTake(lt_isr_sem, 100/portTICK_PERIOD_MS);
	if(res != pdTRUE)
	{
		print_all_regs();
		STD_END("trig timeout");		
	}
	Clear_DMA_Draw_BTE_Interrupt_Flag();
}

void lt_trig_init()
{
	Clear_DMA_Draw_BTE_Interrupt_Flag();
	Enable_DMA_Draw_BTE_Interrupt();
	trig_init();
	lt_isr_sem = xSemaphoreCreateBinary();
}
