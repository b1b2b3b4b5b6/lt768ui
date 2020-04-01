/********************* COPYRIGHT  **********************
* File Name        : if_port.c
* Author           : Levetop Electronics
* Version          : V1.0
* Date             : 2017-9-11
* Description      : ��ͬ�������ӿ�
********************************************************/

#include "if_port.h"
#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

#define ID (uint8_t)0
#define DEVICE_ID (uint8_t)0x70
// #define REG_S (DEVICE_ID & (ID << 2) & 0)
// #define DATA_S (DEVICE_ID & (ID << 2) & 0x02)
#define CMD_WRITE (uint8_t)0x00
#define DATA_WRITE (uint8_t)0x80
#define STATUS_READ (uint8_t)0x40
#define DATA_READ (uint8_t)0xc0

#define SPI_NUM HSPI_HOST



static spi_device_handle_t spi;

static esp_err_t lcd_write_cmd(uint8_t cmd)
{
    esp_err_t ret;
	spi_transaction_t *trans = (spi_transaction_t *)calloc(1, sizeof(spi_transaction_t));
    assert(trans);
    trans->length=2 * 8;                     
    trans->tx_data[0] = CMD_WRITE;
    trans->tx_data[1] = cmd;
    trans->flags = SPI_TRANS_USE_TXDATA;
    ret=spi_device_transmit(spi, trans);
    assert(ret == ESP_OK);
    free(trans);
    return ret;
}

static esp_err_t lcd_write_data(uint8_t data)
{
    esp_err_t ret;
	spi_transaction_t *trans = (spi_transaction_t *)calloc(1, sizeof(spi_transaction_t));
    assert(trans);
    trans->length=2 * 8;                     
    trans->tx_data[0] = DATA_WRITE;
    trans->tx_data[1] = data;
    trans->flags = SPI_TRANS_USE_TXDATA;
    ret=spi_device_transmit(spi, trans);
    assert(ret == ESP_OK);    
    free(trans); 
    return ret;
}

static esp_err_t lcd_write_datas(uint8_t *data, int len)
{
    esp_err_t ret;
	spi_transaction_t *trans = (spi_transaction_t *)calloc(1, sizeof(spi_transaction_t));
    assert(trans);
	uint8_t *temp = malloc(len + 1);
	temp[0] = DATA_WRITE;
	memcpy(temp + 1, data, len);
	trans->length=(1 + len) * 8;
	trans->tx_buffer = temp;
    trans->flags = 0;
    ret=spi_device_transmit(spi, trans);
    assert(ret == ESP_OK);
	free(temp);
	free(trans); 
    return ret;
}

static esp_err_t lcd_read_data(uint8_t *data)
{
    //STD_LOGI("enter");
    esp_err_t ret;
	spi_transaction_t *trans = (spi_transaction_t *)calloc(1, sizeof(spi_transaction_t));
    assert(trans);
    trans->length=1 * 8;          
    trans->rxlength = 1 * 8;           
    trans->tx_data[0] = DATA_READ;
    trans->flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
    ret=spi_device_transmit(spi, trans);
    assert(ret == ESP_OK);
    *data = trans->rx_data[0];
    //STD_LOGI("data: %x", *data);
    free(trans);
    return ret; 
}

static esp_err_t lcd_read_status(uint8_t *data)
{
    //STD_LOGI("enter");
    esp_err_t ret;
	spi_transaction_t *trans = (spi_transaction_t *)calloc(1, sizeof(spi_transaction_t));
    assert(trans);
    trans->length= 1 * 8;
    trans->rxlength = 1 * 8;
    trans->tx_data[0] = STATUS_READ;
    trans->flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
    ret=spi_device_transmit(spi, trans);
    assert(ret == ESP_OK);
    *data = trans->rx_data[0];
    //STD_LOGI("status: %x", *data);
    free(trans);
    return ret;
}


void IRAM_ATTR lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    
}

void IRAM_ATTR lcd_spi_after_transfer_callback(spi_transaction_t *t)
{
	//free(t);
}

//Initialize the display 
static void lcd_pin_init()
{
#ifdef LT768_LT768_PIN_NUM_RESET
    gpio_set_direction(LT768_LT768_PIN_NUM_RESET, GPIO_MODE_OUTPUT);
#endif

#ifdef LT768_PIN_NUM_TFT_RESET
    gpio_set_direction(LT768_PIN_NUM_TFT_RESET, GPIO_MODE_OUTPUT);
#endif

    gpio_set_direction(LT768_PIN_NUM_BL, GPIO_MODE_OUTPUT);
}


void LCD_CmdWrite(uint8_t cmd)
{
	lcd_write_cmd(cmd);
}

void LCD_DataWrite(uint8_t data)
{
	lcd_write_data(data);
}

void LCD_DATASWRITE(uint8_t *data, int len)
{
	lcd_write_datas(data, len);
}

void LCD_DataWrite_Pixel(uint16_t data)
{
    lcd_write_data(data);
    lcd_write_data(data >> 8);
}

uint8_t LCD_StatusRead(void)
{
    uint8_t temp = 0;
    lcd_read_status(&temp);
    //STD_LOGI("status: %x", temp);
    return temp;
}

uint8_t LCD_DataRead(void)
{
    uint8_t temp = 0;
    lcd_read_data(&temp);
    //STD_LOGI("data: %x", temp);
    return temp;
}
	  
	 
void Delay_us(uint16_t time)
{    
	vTaskDelay(1 / portTICK_RATE_MS);
}

void Delay_ms(uint16_t time)
{
	vTaskDelay(time / portTICK_RATE_MS);
}




void Parallel_Init(void)
{
 	esp_err_t ret;
	lcd_pin_init();
    spi_bus_config_t buscfg={
        .miso_io_num=-1,
        .mosi_io_num=LT768_PIN_NUM_MOSI,
        .sclk_io_num=LT768_PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=240*320*2+8
    };
    spi_device_interface_config_t devcfg = {

        .clock_speed_hz = SPI_MASTER_FREQ_10M,   //Clock out at 10 MHz
        .mode = 3,                               //SPI mode 0
        .spics_io_num = LT768_PIN_NUM_CS,                      //CS pin
        .queue_size = 100,                       //We want to be able to queue 7 transactions at a time
        .pre_cb = lcd_spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
        .post_cb = lcd_spi_after_transfer_callback,
        .flags = SPI_DEVICE_3WIRE | SPI_DEVICE_HALFDUPLEX,
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(SPI_NUM, &buscfg, 0);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(SPI_NUM, &devcfg, &spi);
	ESP_ERROR_CHECK(ret);
	STD_LOGI("spi init success");

}

static void tft_reset()
{
#ifdef LT768_PIN_NUM_TFT_RESET
    gpio_set_level(LT768_PIN_NUM_TFT_RESET, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(LT768_PIN_NUM_TFT_RESET, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
#endif
    gpio_set_level(LT768_PIN_NUM_BL, 0);
    
}


void lt768_reset()
{
    tft_reset();
#ifdef LT768_LT768_PIN_NUM_RESET
    gpio_set_level(LT768_LT768_PIN_NUM_RESET, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(LT768_LT768_PIN_NUM_RESET, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
#endif

}

