#include "FT6X36.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

#define TP_MESSAGE_SIZE 1
#define CHECK_HZ 60
#define OFFSET_COUNT_MAX 3


#define TASK_PRIO (ESP_TASK_MAIN_PRIO + 3)
#define TASK_SIZE 2048

#define I2C_MASTER_NUM             I2C_NUM_1        /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE  0                /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ         400000           /*!< I2C master clock frequency */

#define SLAVE_ADDR                         0x90          /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            0x0              /*!< I2C ack value */
#define NACK_VAL                           0x1              /*!< I2C nack value */

#define INFO_SATRT (uint8_t)0x03
#define TH_GROUP (uint8_t)0x80
#define TH_DIFF (uint8_t)0x85
#define CTRL (uint8_t)0x86
#define TIMEENTERMONITOR (uint8_t)0x87
#define PERIODACTIVE (uint8_t)0x88
#define PERIODMONITOR (uint8_t)0x89
#define RADIAN_VALUE (uint8_t)0x91
#define OFFSET_LEFT_RIGHT (uint8_t)0x92
#define OFFSET_UP_DOWN (uint8_t)0x93
#define DISTANCE_LEFT_RIGHT (uint8_t)0x94
#define DISTANCE_UP_DOWN (uint8_t)0x95
#define DISTANCE_ZOOM (uint8_t)0x96
#define G_MODE (uint8_t)0xa4

#define WIDTH 720
#define HEIGHT 720

typedef struct {
    uint8_t threshold;
    uint8_t diff;
    uint8_t ctrl;
    uint8_t time_enter_monitor;
    uint8_t period_active;
    uint8_t period_monitor;
    uint8_t radian_value;
    uint8_t offset_left_right;
    uint8_t offset_up_dwon;
    uint8_t distance_left_right;
    uint8_t distance_up_down;
    uint8_t distance_zoom;
    uint8_t g_mode;
} tp_configure_t;

typedef struct {
    uint8_t gest_id;
    uint8_t td_status;

    uint8_t p1_xh;
    uint8_t p1_xl;
    uint8_t p1_yh;
    uint8_t p1_yl;
    uint8_t p1_weight;
    uint8_t p1_misc;

    uint8_t p2_xh;
    uint8_t p2_xl;
    uint8_t p2_yh;
    uint8_t p2_yl;
    uint8_t p2_weight;
    uint8_t p2_misc;
} tp_raw_info_t;



static tp_raw_info_t tp_raw_info = {0};
static tp_configure_t tp_configure = {0};
static QueueHandle_t tp_message_queue;
static SemaphoreHandle_t tp_isr_sem;

static bool disable_flag = false;
static uint8_t g_address = SLAVE_ADDR;
static TimerHandle_t timer_handle;
static int touch_status = NO_EVENT;
static int last_x = 0;
static int last_y = 0;

static esp_err_t i2c_example_master_read_slave(uint8_t* data_rd, size_t size)
{
    if (size == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, g_address | READ_BIT, ACK_CHECK_EN);
    if (size > 1) {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    ESP_ERROR_CHECK(ret);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t i2c_example_master_write_slave(uint8_t* data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, g_address | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    ESP_ERROR_CHECK(ret);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t find_address()
{
    int ret;
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, SLAVE_ADDR | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if(ret == 0)
        return 0;
    
    STD_LOGI("default address error, finding");
    for (int n = 0; n < 0xff; n+=2)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        STD_LOGI("test %x", n);

        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, n | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);

        if(ret == ESP_OK)
        {
            STD_LOGI("find addrsss : %x", n);
            g_address = n;
            return 0;
        }
            
    }
    return -1;
}

static esp_err_t i2c_example_master_write_address(uint8_t address)
{
    return i2c_example_master_write_slave(&address, 1);
}

static int write_reg(uint8_t reg, uint8_t value)
{
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    ESP_ERROR_CHECK(i2c_example_master_write_slave(buf, 2));
    return 0;
}

static int read_reg(uint8_t reg, uint8_t *value)
{
    int res = 0;
    res = i2c_example_master_write_slave(&reg, 1);
    if(res !=0 )
        return -1;
    i2c_example_master_read_slave(value, 1);
    if(res !=0 )
        return -1;
    return 0;
}

static void print_info(tp_info_t *tp_info)
{
	STD_LOGV("gest_id: %d, event: %d, x: %d, y: %d", tp_info->gest_id, tp_info->event, tp_info->x, tp_info->y);
}





static int read_info(tp_info_t *tp_info)
{
    int res = 0;
	if(i2c_example_master_write_address(INFO_SATRT) != 0)
        return -1;
    if(i2c_example_master_read_slave((uint8_t *)&(tp_raw_info.p1_xh), 4) != 0)
        return -1;
	//tp_info->gest_id = tp_raw_info.gest_id;
	tp_info->event = (tp_raw_info.p1_xh & 0xc0) >> 6;
    tp_info->x = tp_raw_info.p1_xh & 0x0f;
    tp_info->x <<= 8;
    tp_info->x |= tp_raw_info.p1_xl;

    //tp_info->touch_id = (tp_raw_info.p1_yh & 0xf0) >> 4;
    tp_info->y = tp_raw_info.p1_yh & 0x0f;
    tp_info->y <<= 8;
    tp_info->y |= tp_raw_info.p1_yl;

	if((tp_info->x <= 1) || (tp_info->y <= 1))
		return -1;
	if((tp_info->x >= 720) || (tp_info->y >= 720))
		return -1;
	print_info(tp_info);
	return 0;
}


static int configure()
{
    tp_configure.threshold = 0x10;
    tp_configure.diff = 0x00;
    tp_configure.ctrl = 0x00;
    tp_configure.time_enter_monitor = 0x01;
    tp_configure.period_active = 1000/CHECK_HZ;
    tp_configure.period_monitor = 1000/CHECK_HZ;
    tp_configure.radian_value = 0x1a;
    tp_configure.offset_left_right = 0x30;
    tp_configure.offset_up_dwon = 0x30;
    tp_configure.distance_left_right = 0x30;
    tp_configure.distance_up_down = 0x30;
    tp_configure.distance_zoom = 0x32;
    tp_configure.g_mode = 0x01;

    write_reg(TH_GROUP, tp_configure.threshold);
    write_reg(TH_DIFF, tp_configure.diff);
    write_reg(CTRL, tp_configure.ctrl);
    write_reg(TIMEENTERMONITOR, tp_configure.time_enter_monitor);
    write_reg(PERIODACTIVE, tp_configure.period_active);
    write_reg(PERIODMONITOR, tp_configure.period_monitor);
    write_reg(RADIAN_VALUE, tp_configure.radian_value);
    write_reg(OFFSET_LEFT_RIGHT, tp_configure.offset_left_right);
    write_reg(OFFSET_UP_DOWN, tp_configure.offset_up_dwon);
    write_reg(DISTANCE_LEFT_RIGHT, tp_configure.distance_left_right);
    write_reg(DISTANCE_UP_DOWN, tp_configure.distance_up_down);
    write_reg(DISTANCE_ZOOM, tp_configure.distance_zoom);
    write_reg(G_MODE, tp_configure.g_mode);
    return 0;
}

static void i2c_master_init()
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = TP_I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = TP_I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_MASTER_RX_BUF_DISABLE,
                       I2C_MASTER_TX_BUF_DISABLE, 0);
}

tp_info_t *tp_receive_message(int timeout_ms)
{
    tp_info_t *temp;
    int res;
    res = xQueueReceive(tp_message_queue, &temp, timeout_ms/portTICK_PERIOD_MS);
    if(res == pdFALSE)
        return NULL;
    else
        return temp;
}


static void tp_send_message(tp_info_t *tp_info)
{
	int res;
	int delay = 0;

	switch(tp_info->event)
	{
		case PRESS_DOWN:
			STD_LOGD("SEND MESSAGE x: %d, y: %d, PRESS_DOWN", tp_info->x, tp_info->y);
			delay = portMAX_DELAY;
			break;
		case LIFT_UP:
			STD_LOGD("SEND MESSAGE x: %d, y: %d, LIFT_UP",  tp_info->x, tp_info->y);
			delay = portMAX_DELAY;
			break;
		case CONTACT:
			STD_LOGD("SEND MESSAGE x: %d, y: %d, offset_x: %d, offset_y: %d, CONTACT", tp_info->x, tp_info->y, tp_info->offset_x, tp_info->offset_y);
			break;
		default:
			assert(NULL);
			break;
	}
	res = xQueueSend(tp_message_queue, &tp_info, delay);
	if(res != pdTRUE)
	{
		STD_LOGW("tp message full! abandoned!");
		free(tp_info);
	}	
}


void enable_tp_message()
{
    disable_flag = false;
}

void disable_tp_message()
{
    disable_flag = true;
}


static bool check_stabe(int x, int y)
{
	static int x1, y1, x2, y2;
	bool res = false;

	if(x1 == x2 && y1 == y2)
		goto FINAL;
	if(x == x1 && y == y1)
		goto FINAL;
	if(x == x2 && y == y2)
		goto FINAL;
	res = true;
FINAL:
	x1 = x2;
	y1 = y2;
	x2 = x;
	y2 = y;
	return res;
}

static void tp_check_task(void *arg)
{
	static int start_x;
	static int start_y;
	
	static tp_info_t *send;
	static tp_info_t temp;
	int res;
    for (;;)
    {

		xSemaphoreTake(tp_isr_sem, portMAX_DELAY);

		if(xTimerStart(timer_handle, 0) == pdFALSE)
            STD_ASSERT(NULL);

		if(read_info(&temp) != 0)
			continue;

		if(touch_status == NO_EVENT)
		{
			send = calloc(1, sizeof(tp_info_t));
			touch_status = CONTACT;
			start_x = temp.x;
			start_y = temp.y;
			last_x = temp.x;
			last_y = temp.y;
			send->x = temp.x;
			send->y = temp.y;
			send->event = PRESS_DOWN;
			send->gest_id = TP_NO_GESTRE;
			check_stabe(start_x, start_y);
			check_stabe(start_x, start_y);
			tp_send_message(send);
			continue;
		}

		int x_offset = temp.x - start_x;
		int y_offset = temp.y - start_y;

		if(check_stabe(temp.x, temp.y))
		{
			send = calloc(1, sizeof(tp_info_t));
			last_x = temp.x;
			last_y = temp.y;
			send->x = temp.x;
			send->y = temp.y;
			send->offset_x = temp.x - start_x;
			send->offset_y = temp.y - start_y;
			send->gest_id = TP_NO_GESTRE;
			send->event = CONTACT;
			tp_send_message(send);
		}
	}
    return NULL;
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	BaseType_t p;
	xSemaphoreGiveFromISR(tp_isr_sem, &p);
}

static int trig_init()
{
    gpio_config_t io_conf;
		//interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = 1ULL<<TP_PIN_NUM_INIT;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    gpio_set_intr_type(TP_PIN_NUM_INIT, GPIO_INTR_NEGEDGE);
	    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(TP_PIN_NUM_INIT, gpio_isr_handler, NULL);
	return 0;
}

void stop_msg_cb(void *arg)
{
	touch_status = NO_EVENT;
	tp_info_t *send;
	send = calloc(1, sizeof(tp_info_t));
	send->gest_id = TP_NO_GESTRE;
	send->event = LIFT_UP;
	send->x = last_x;
	send->y = last_y;
	tp_send_message(send);
}

int ft6x36_init()
{

#ifdef TP_PIN_NUM_RESET
    gpio_set_direction(TP_PIN_NUM_RESET, GPIO_MODE_OUTPUT);
	gpio_set_level(TP_PIN_NUM_RESET, 0);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    gpio_set_level(TP_PIN_NUM_RESET, 1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
#endif

    i2c_master_init();
    int ret;
    ret = find_address();
    ESP_ERROR_CHECK(ret);
    configure();
    enable_tp_message();
    tp_message_queue = xQueueCreate(TP_MESSAGE_SIZE, sizeof(tp_info_t *));
	tp_isr_sem = xSemaphoreCreateBinary();
	assert(tp_isr_sem);
	trig_init();
	int max_delay = (1000 / CHECK_HZ)*2;
	timer_handle = xTimerCreate("stop msg", max_delay/portTICK_PERIOD_MS, pdFALSE, NULL, stop_msg_cb);
	xTaskCreate(tp_check_task, "tp_check_task", TASK_SIZE, NULL, TASK_PRIO, NULL);
    STD_LOGI("fx6x36 int success");
    return ESP_OK;
}

#ifdef PRODUCT_TEST

void tp_recevie_test()
{
	tp_info_t *tp_info;
	for (;;)
	{
		tp_info = tp_receive_message(10);
		if(tp_info != NULL)
		{
			STD_LOGI("[receive] x: %d, y: %d, gest_id: %d", tp_info->x, tp_info->y, tp_info->gest_id);
			STD_FREE(tp_info);
		}
			
	}
}

#endif



