#ifndef W25Q_H
#define W25Q_H

#include "std_common.h"
#include "std_port_common.h"
#include "lt768ui_config.h"


typedef enum {
    STD = 0,
    DO,
    QO,
    QPI
} spi_mode_t;

int w25q_init();

int w25q_deint();

int w25q_reint();

int w25q_mode(int mode);

int w25q_wait_trans();

int w25q_read(size_t addr, void *dest, size_t size);

int w25q_write(size_t addr, const void *src, size_t size);

uint32_t w25q_sector_size();

uint32_t w25q_chip_size();

int w25q_erase_chip();

int w25q_erase_sector(size_t addr);

//size, addr must mod by sector
int w25q_erase_range(size_t addr, size_t size);

int w25q_write(size_t addr, const void *src, size_t size);

int w25q_test();

#endif