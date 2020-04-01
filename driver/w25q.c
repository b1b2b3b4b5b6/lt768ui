#include "w25q.h"

#define SPI_NUM VSPI_HOST


#define MAX_DMA_SIZE 120*1024
#define MAX_QUENE_SIZE 1
#define DMA_CHANNEL 1
#define CLOCK SPI_MASTER_FREQ_40M
#define PAGE_SIZE 256

#define CMD_WRITE_STATUS_REG1               0x01
#define CMD_PAGE_PROGRAM                    0x02
#define CMD_QUAD_PAGE_PROGRAM               0x32
#define CMD_READ_DATA                       0x03
#define CMD_READ_STATUS_REG1                0x05
#define CMD_WRITE_ENABLE                    0x06
#define CMD_FAST_READ                       0x0b
#define CMD_SECTOR_ERASE                    0x20
#define CMD_READ_SFDP                       0x5a
#define CMD_ENABLE_RESET                    0x66
#define CMD_RESET_DEVICE                    0x99
#define CMD_READ_JEDEC_ID                   0x9f
#define CMD_CHIP_ERASE                      0xc7
#define CMD_WRITE_STATUS_REG2               0x31
#define CMD_READ_STATUS_REG2                0x35
#define CMD_ENTER_QPI_MODE                  0x38
#define CMD_FAST_READ_DUAL_OUTPUT           0x3b
#define CMD_SR_WRITE_ENABLE                 0x50
#define CMD_BLOCK_ERASE_32K                 0x52
#define CMD_FAST_READ_QUAD_OUTPUT           0x6b
#define CMD_FAST_READ_DUAL_IO               0xbb
#define CMD_SET_READ_PARAMETERS             0xc0
#define CMD_BLOCK_ERASE_64K                 0xd8
#define CMD_OCTAL_WORD_READ_QUAD_IO         0xe3
#define CMD_WORD_READ_QUAD_IO               0xe7
#define CMD_FAST_READ_QUAD_IO               0xeb
#define CMD_EXIT_QPI_MODE                   0xff

#define STD_LOCAL_LOG_LEVEL STD_LOG_DEBUG

typedef struct {
    uint32_t sector_size;
    uint32_t chip_size;
    spi_mode_t mode;
    uint32_t tflags;
    spi_transaction_ext_t trans;
    spi_device_handle_t spi;
    int on_trans;
} w25q_dev_t;

typedef struct {
    uint8_t hasaddr;
    uint8_t isread;
    uint8_t cmd;
    int64_t addr;
    uint8_t dummy;
    uint8_t *buf;
    size_t size;
} w25q_cmd_t;

static DRAM_ATTR w25q_dev_t g_w25q_dev = {0};
static w25q_cmd_t g_w25q_cmd = {0};

static int cmd(w25q_cmd_t *cmd)
{
    memset(&g_w25q_dev.trans, 0, sizeof(g_w25q_dev.trans));
    g_w25q_dev.trans.base.flags = g_w25q_dev.tflags;
    g_w25q_dev.trans.base.cmd = cmd->cmd;
    g_w25q_dev.trans.base.addr = cmd->addr <<  cmd->dummy;
    if(cmd->hasaddr != 0)
        g_w25q_dev.trans.address_bits = 24 + cmd->dummy;
    else
        g_w25q_dev.trans.address_bits = 0;

    if (cmd->isread != 0)
    {
        g_w25q_dev.trans.base.rx_buffer = cmd->buf;
        g_w25q_dev.trans.base.rxlength = cmd->size * 8;
    }
    else
    {
        g_w25q_dev.trans.base.tx_buffer = cmd->buf;
        g_w25q_dev.trans.base.length = cmd->size * 8;
    }
    ESP_ERROR_CHECK(spi_device_transmit(g_w25q_dev.spi, &g_w25q_dev.trans.base));
    return 0;
}

static int write_enable() 
{
    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.cmd = CMD_WRITE_ENABLE;
    cmd(&g_w25q_cmd);
    return 0;
}

static uint8_t read_status_register1()
{
    uint8_t status;

    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.cmd = CMD_READ_STATUS_REG1;
    g_w25q_cmd.isread = 1;
    g_w25q_cmd.buf = &status;
    g_w25q_cmd.size = 1;
    cmd(&g_w25q_cmd);

    return status;    
}

static uint8_t write_status_register1()
{
    uint8_t status;

    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.cmd = CMD_SR_WRITE_ENABLE;
    cmd(&g_w25q_cmd);

    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.cmd = CMD_READ_STATUS_REG1;
    g_w25q_cmd.isread = 0;
    g_w25q_cmd.buf = &status;
    g_w25q_cmd.size = 1;
    cmd(&g_w25q_cmd);

    return status;    
}


static int w25q_wait_idle(int time_ms)
{
    while (read_status_register1() & 0x01)
    {
        if(time_ms > 0)
            vTaskDelay(time_ms/portTICK_PERIOD_MS);
    }
    return 0;
}

int write_status_register2(uint8_t status)
{

    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.cmd = CMD_SR_WRITE_ENABLE;
    cmd(&g_w25q_cmd);

    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.cmd = CMD_WRITE_STATUS_REG2;
    g_w25q_cmd.isread = 0;
    g_w25q_cmd.buf = &status;
    g_w25q_cmd.size = 1;
    cmd(&g_w25q_cmd);

    w25q_wait_idle(20);
    return 0;
}

uint8_t read_status_register2()
{
    uint8_t status;

    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.cmd = CMD_READ_STATUS_REG2;
    g_w25q_cmd.isread = 1;
    g_w25q_cmd.buf = &status;
    g_w25q_cmd.size = 1;
    cmd(&g_w25q_cmd);

    return status;
}


static int reset()
{
    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.cmd = CMD_ENABLE_RESET;
    cmd(&g_w25q_cmd);

    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.cmd = CMD_RESET_DEVICE;
    cmd(&g_w25q_cmd);

    w25q_wait_idle(50);
    return 0;
}

static int read_sfdp()
{
    uint8_t sfdp[256] = {};
    uint32_t *p = (uint32_t *) sfdp;
    uint32_t capacity = 0;
    uint32_t sector_sz = 0;

    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.isread = 1;
    g_w25q_cmd.cmd = CMD_READ_SFDP;
    g_w25q_cmd.hasaddr = 1;
    g_w25q_cmd.addr = 0;
    g_w25q_cmd.dummy = 8;
    g_w25q_cmd.buf = sfdp;
    g_w25q_cmd.size = 8;
    cmd(&g_w25q_cmd);

    if (p[0] != ('S' | 'F' << 8 | 'D' << 16 | 'P' << 24))
    {
        return false;
    }

    int8_t nph = (p[1] >> 16) & 0xff;

    uint32_t ptp = 0;
    uint8_t ptl = 0;
    uint32_t off = 8;
    while (nph-- >= 0)
    {

        memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
        g_w25q_cmd.isread = 1;
        g_w25q_cmd.cmd = CMD_READ_SFDP;
        g_w25q_cmd.hasaddr = 1;
        g_w25q_cmd.addr = off;
        g_w25q_cmd.dummy = 8;
        g_w25q_cmd.buf = sfdp;
        g_w25q_cmd.size = 8;
        cmd(&g_w25q_cmd);

        uint32_t dword1 = p[0];
        uint32_t dword2 = p[1];

        if (((dword2 >> 24) & 0xff) == 0xff)
        {
            ptp = dword2 & 0x00ffffff;
            ptl = (dword1 >> 24) & 0xff; 
        }
    }

    if (ptp == 0 || ptl == 0 || ptl > 32)
    {
        return -1;
    }

    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.isread = 1;
    g_w25q_cmd.cmd = CMD_READ_SFDP;
    g_w25q_cmd.hasaddr = 1;
    g_w25q_cmd.addr = ptp;
    g_w25q_cmd.dummy = 8;
    g_w25q_cmd.buf = sfdp;
    g_w25q_cmd.size = ptl << 3;
    cmd(&g_w25q_cmd);

    for (int i = 0; i < ptl; i++)
    {
        uint32_t dword = p[i];
        switch (i)
        {
            case 0:
                sector_sz = (dword & 0x03) == 1 ? 4096 : 0;
            break;

            case 1:
                if (dword & 0x80000000)
                {
                    capacity = 1 << (dword & 0x7fffffff);
                }
                else
                {
                    capacity = (dword + 1) / 8;
                }
            break;

            case 7:
                sector_sz = 1 << (dword & 0xff);
            break;
        }
    }

    if (capacity == 0)
    {
        WORD_ALIGNED_ATTR uint8_t id[3];

        memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
        g_w25q_cmd.isread = 1;
        g_w25q_cmd.cmd = CMD_READ_JEDEC_ID;
        g_w25q_cmd.buf = id;
        g_w25q_cmd.size = sizeof(id);
        cmd(&g_w25q_cmd);

        if (id[2] >= 0x10 && id[2] <= 0x17)
        {
            capacity = 1 << id[2];
        }
    }

    if (capacity == 0 || sector_sz == 0)
    {
        return ESP_FAIL;
    }

    g_w25q_dev.chip_size = capacity;
    g_w25q_dev.sector_size = sector_sz;

    return 0;
}

void IRAM_ATTR spi_pre_transfer_callback(spi_transaction_t *t)
{
     g_w25q_dev.on_trans = 0;
}

void IRAM_ATTR spi_after_transfer_callback(spi_transaction_t *t)
{
    g_w25q_dev.on_trans = 0;
}

int w25q_init()
{
 	esp_err_t ret;
    spi_bus_config_t buscfg={
        .miso_io_num=W25Q_PIN_NUM_MISO,
        .mosi_io_num=W25Q_PIN_NUM_MOSI,
        .sclk_io_num=W25Q_PIN_NUM_CLK,
#ifdef W25Q_PIN_NUM_WP
        .quadwp_io_num=W25Q_PIN_NUM_WP,
#else
        .quadwp_io_num=-1,
#endif

#ifdef W25Q_PIN_NUM_HD
        .quadhd_io_num=W25Q_PIN_NUM_HD,
#else
        .quadhd_io_num=-1,
#endif
        
        .max_transfer_sz= MAX_DMA_SIZE
    };
    spi_device_interface_config_t devcfg = {
        .command_bits = 8,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 3,
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = CLOCK,   //Clock out at 10 MHz
        .input_delay_ns=0,
        .spics_io_num = W25Q_PIN_NUM_CS,                      //CS pin
        .queue_size = MAX_QUENE_SIZE,                       //We want to be able to queue 7 transactions at a time
        .pre_cb = spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
        .post_cb = spi_after_transfer_callback,
        .flags = SPI_DEVICE_HALFDUPLEX,
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(SPI_NUM, &buscfg, DMA_CHANNEL);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(SPI_NUM, &devcfg, &g_w25q_dev.spi);
	ESP_ERROR_CHECK(ret);
    w25q_mode(STD);
    reset();

    read_sfdp();
	STD_LOGI("chip size: %d, sector size: %d", g_w25q_dev.chip_size, g_w25q_dev.sector_size);

    STD_LOGI("w25q init success");
    return 0;
}

int w25q_deint()
{
	spi_bus_remove_device(g_w25q_dev.spi);
	spi_bus_free(SPI_NUM);
	return 0;
}

int w25q_reint()
{
 	esp_err_t ret;
    spi_bus_config_t buscfg={
        .miso_io_num=W25Q_PIN_NUM_MISO,
        .mosi_io_num=W25Q_PIN_NUM_MOSI,
        .sclk_io_num=W25Q_PIN_NUM_CLK,

#ifdef W25Q_PIN_NUM_WP
        .quadwp_io_num=W25Q_PIN_NUM_WP,
#endif
#ifdef W25Q_PIN_NUM_HD
        .quadhd_io_num=W25Q_PIN_NUM_HD,
#endif
        
        .max_transfer_sz= MAX_DMA_SIZE
    };
    spi_device_interface_config_t devcfg = {
        .command_bits = 8,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 3,
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = CLOCK,   //Clock out at 10 MHz
        .input_delay_ns=0,
        .spics_io_num = W25Q_PIN_NUM_CS,                      //CS pin
        .queue_size = MAX_QUENE_SIZE,                       //We want to be able to queue 7 transactions at a time
        .pre_cb = spi_pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
        .post_cb = spi_after_transfer_callback,
        .flags = SPI_DEVICE_HALFDUPLEX,
    };
	ret=spi_bus_initialize(SPI_NUM, &buscfg, DMA_CHANNEL);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(SPI_NUM, &devcfg, &g_w25q_dev.spi);
	ESP_ERROR_CHECK(ret);
	return ret;
}

int w25q_mode(int mode)
{
    switch(g_w25q_dev.mode)
    {
        case STD:

            break;

        case DO:

            break;

        case QO:
            write_status_register2(read_status_register2() & ~0x02);
            break;

        case QPI:

            break;
        default :
            break;     
    }
    switch(mode)
    {
        case STD:
            g_w25q_dev.mode = SPI_TRANS_VARIABLE_ADDR;
            break;

        case DO:
            g_w25q_dev.mode = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_MODE_DIO;
            break;

        case QO:
            write_status_register2(read_status_register2() | 0x02);
            g_w25q_dev.mode = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_MODE_QIO;
            break;

        case QPI:
            g_w25q_dev.mode = SPI_TRANS_VARIABLE_ADDR;
            break;
        default :
            STD_LOGE("undefined type");
            return -1;
            break;
    }
    g_w25q_dev.mode = mode;
    g_w25q_dev.tflags = SPI_TRANS_VARIABLE_ADDR;
    STD_LOGI("w25q set mode [%d]", mode);
    return 0;
}

int w25q_wait_trans()
{
    spi_transaction_t *p = &(g_w25q_dev.trans.base);
    ESP_ERROR_CHECK(spi_device_get_trans_result(g_w25q_dev.spi, &p, portMAX_DELAY));
    return 0;
}

static int read_nocrm(uint8_t inst, uint8_t dummy, size_t addr, void *dest, size_t size)
{
    STD_LOGV("%s - inst=0x%02x dummy=%d addr=0x%08x size=%d", __func__, inst, dummy, addr, size);

    size_t len = 32 * 1024;
    uint8_t *rbuf = (uint8_t *) PORT_DMA_MALLOC(len);
    while (size > 0)
    {
        if (len >= size)
        {
            len = size;
        }

        memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
        g_w25q_cmd.isread = 1;
        g_w25q_cmd.hasaddr = 1;
        g_w25q_cmd.addr = addr;
        g_w25q_cmd.cmd = inst;
        g_w25q_cmd.dummy = dummy;
        g_w25q_cmd.buf = rbuf;
        g_w25q_cmd.size = len;
        cmd(&g_w25q_cmd);

        memcpy(dest, rbuf, len);
        addr += len;
        dest += len;
        size -= len;
    }
    STD_FREE(rbuf);
    return ESP_OK;
}

int w25q_read(size_t addr, void *dest, size_t size)
{
    STD_LOGD("w25q start to read addr=0x%08x size=%d", addr, size);
    switch(g_w25q_dev.mode)
    {
        case STD:
            g_w25q_dev.tflags = SPI_TRANS_VARIABLE_ADDR;
            read_nocrm(CMD_FAST_READ, 8, addr, dest, size);
            g_w25q_dev.tflags = SPI_TRANS_VARIABLE_ADDR;
            break;

        case DO:
            g_w25q_dev.tflags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_MODE_DIO;;
            read_nocrm(CMD_FAST_READ_DUAL_OUTPUT, 8, addr, dest, size);
            g_w25q_dev.tflags = SPI_TRANS_VARIABLE_ADDR;
            break;

        case QO:
            g_w25q_dev.tflags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_MODE_QIO;
            read_nocrm(CMD_FAST_READ_QUAD_OUTPUT, 8, addr, dest, size);
            g_w25q_dev.tflags = SPI_TRANS_VARIABLE_ADDR;
            break;

        case QPI:
            return 0;
            break;
    }
    return 0;
}

uint32_t w25q_sector_size()
{
    return g_w25q_dev.sector_size;
}

uint32_t w25q_chip_size()
{
    return g_w25q_dev.chip_size;
}

int w25q_erase_chip()
{
    STD_LOGW("w25q start erase chip");

    write_enable();
    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.cmd = CMD_CHIP_ERASE;
    cmd(&g_w25q_cmd);

    w25q_wait_idle(1000);

    STD_LOGW("w25q erase chip success");
    return ESP_OK;
}

int w25q_erase_sector(size_t addr)
{
    if((addr % g_w25q_dev.sector_size) != 0)
    {
        STD_LOGE("unaligned erase sector addr !");
        vTaskDelay(portMAX_DELAY);
    }

    write_enable();

    memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
    g_w25q_cmd.cmd = CMD_SECTOR_ERASE;
    g_w25q_cmd.hasaddr = 1;
    g_w25q_cmd.addr = addr;
    cmd(&g_w25q_cmd);

    w25q_wait_idle(50);

    return ESP_OK;
}

int w25q_erase_range(size_t addr, size_t size)
{
    STD_LOGW("w25q start erase range addr=0x%08x size=%d", addr , size);
    if((size % g_w25q_dev.sector_size) != 0)
    {
        STD_LOGE("unaligned erase range size !");
        STD_ASSERT(-1);
    }
  
    
    int count = size / g_w25q_dev.sector_size;

    for (int n = 0; n < count; n++)
    {
        w25q_erase_sector(addr);
        addr += g_w25q_dev.sector_size;
    }
    STD_LOGW("w25q erase range success");
    return ESP_OK;
}

int w25q_write(size_t addr, const void *src, size_t size)
{
    STD_LOGD("w25q start to write addr=0x%08x size=%d", addr, size);
    if((addr % PAGE_SIZE) != 0)
    {
        STD_LOGE("unaligned write addr !");
        vTaskDelay(portMAX_DELAY);
    }

    if((size % PAGE_SIZE) != 0)
    {
        STD_LOGE("unaligned write size !");
        vTaskDelay(portMAX_DELAY);
    }

    uint8_t cmd_value;
    int cycle = size / PAGE_SIZE;
    uint8_t *wbuf = (uint8_t *) PORT_DMA_MALLOC(PAGE_SIZE);
    if(g_w25q_dev.mode == QO)
        cmd_value = CMD_QUAD_PAGE_PROGRAM;
    else
        cmd_value = CMD_PAGE_PROGRAM;

    for (int n = 0; n < cycle; n++)
    { 
        write_enable();

        if(g_w25q_dev.mode == QO)
            g_w25q_dev.tflags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_MODE_QIO;
        memset(&g_w25q_cmd, 0, sizeof(w25q_cmd_t));
        g_w25q_cmd.isread = 0;
        g_w25q_cmd.hasaddr = 1;
        g_w25q_cmd.addr = addr;
        g_w25q_cmd.cmd = cmd_value;
        memcpy(wbuf, src, PAGE_SIZE);
        g_w25q_cmd.buf = wbuf;
        g_w25q_cmd.size = PAGE_SIZE;
        cmd(&g_w25q_cmd);
        if(g_w25q_dev.mode == QO)
            g_w25q_dev.tflags = SPI_TRANS_VARIABLE_ADDR;

        w25q_wait_idle(0);

        addr += PAGE_SIZE;
        src += PAGE_SIZE;
    }
    STD_FREE(wbuf);
    STD_LOGD("w25q write success");
    return ESP_OK;
}


static int read_test()
{
    STD_LOGI("w25q read test start");
    STD_LOGI("make sure chip has been programed correctly");
    uint32_t bs = 1024 * 1024;
    uint8_t *buf = (uint8_t *) STD_MALLOC(bs);
    int count = w25q_chip_size()/bs;
    size_t addr = 0;
    for (int i = 0; i < count; i++)
    {
        w25q_read(addr, buf, bs);
        for (int n = 0; n < bs; n++)
        {
            if(buf[n] != n%256)
            {
                STD_LOGE("addr=0x%08x error", addr);
                PRINT_HEX(buf, 256);
                vTaskDelay(portMAX_DELAY);
            }
        }
        addr += bs;
    }

    addr = 0;
    struct timeval start;
    gettimeofday(&start, NULL);
    for (int i = 0; i < count; i++)
    {
        w25q_read(addr, buf, bs);
        addr += bs;
    }
    struct timeval end;
    gettimeofday(&end, NULL);

    struct timeval elapsed;
    timersub(&end, &start, &elapsed);
    float s = elapsed.tv_sec + (float)elapsed.tv_usec/1000000;
    free(buf);
    STD_LOGI("read test success, time : %5.2f", s);
    return 0;
}

static int erase_chip_test()
{
    STD_LOGI("erase_chip_test enter");
    struct timeval start;
    gettimeofday(&start, NULL);

    w25q_erase_chip();

    struct timeval end;
    gettimeofday(&end, NULL);

    struct timeval elapsed;
    timersub(&end, &start, &elapsed);
    float s = elapsed.tv_sec + (float)elapsed.tv_usec/1000000;

    uint32_t bs = 65536;
    uint8_t *buf = (uint8_t *) STD_MALLOC(bs);
    int count = w25q_chip_size()/bs;
    size_t addr = 0;
    for (int i = 0; i < count; i++)
    {
        w25q_read(addr, buf, bs);
        for (int n = 0; n < bs; n++)
        {
            if(buf[n] != 0xff)
            {
                STD_LOGE("addr=0x%08x error", addr);
                PRINT_HEX(buf, 256);
                vTaskDelay(portMAX_DELAY);
            }
        }
        addr += bs;
    }
    STD_LOGI("erase chip success, time : %5.2f", s);
    free(buf);
    return 0;
}


static int write_test()
{
    STD_LOGI("w25q write test start");
    int block = 1024 * 1024;
    int count = w25q_chip_size() / block;
    int addr = 0;
    uint8_t *rbuf = (uint8_t *) STD_MALLOC(block);
    uint8_t *wbuf = (uint8_t *) STD_MALLOC(block);
    for (int i = 0; i < block; i++)
    {
        wbuf[i] = i & 0xff;
        rbuf[i] = 0;
    }
    for (int n = 0; n < count; n++)
    {
        w25q_erase_range(addr, block);
        w25q_write(addr, wbuf, block);
        w25q_read(addr, rbuf, block);
        for (int i = 0; i < block; i++)
        {
            if (rbuf[i] != wbuf[i]){
                STD_LOGE("addr=0x%08x i[%d] error", addr, i);
                PRINT_HEX(rbuf, 256);
                PRINT_HEX(wbuf, 256);
                vTaskDelay(portMAX_DELAY);
            }
        }
        addr += block;
    }

    addr = 0;
    struct timeval start;
    gettimeofday(&start, NULL);
    for (int n = 0; n < count; n++)
    {
        w25q_erase_range(addr, block);
        w25q_write(addr, wbuf, block);
        addr += block;
    }

    struct timeval end;
    gettimeofday(&end, NULL);

    struct timeval elapsed;
    timersub(&end, &start, &elapsed);
    float s = elapsed.tv_sec + (float)elapsed.tv_usec/1000000;

    free(wbuf);
    free(rbuf);
    STD_LOGI("%d MB write test success, time : %5.2f", w25q_chip_size()/1024/1024, s);
    return 0;
}

int w25q_test()
{
    //erase_chip_test();
    write_test();
    //read_test();
    return 0;
}


