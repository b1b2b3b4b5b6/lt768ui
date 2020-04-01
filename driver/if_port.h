/********************* COPYRIGHT  **********************
* File Name        : if_port.h
* Author           : Levetop Electronics
* Version          : V1.0
* Date             : 2017-9-11
* Description      : ѡ��ͬ�������ӿ�
********************************************************/

#ifndef _if_port_h
#define _if_port_h
#include "std_port_common.h"  
#include "lt768ui_config.h"


void Parallel_Init(void);

void LCD_CmdWrite(uint8_t cmd);
void LCD_DataWrite(uint8_t data);
void LCD_DataWrite_Pixel(uint16_t data);
void LCD_DATASWRITE(uint8_t *data, int len);
uint8_t LCD_StatusRead(void);
uint8_t LCD_DataRead(void);
	 
void Delay_us(uint16_t time);     // ��ʱ����us��
void Delay_ms(uint16_t time);     // ��ʱ����ms��
void lt768_reset();


#endif
