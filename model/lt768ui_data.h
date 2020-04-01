#ifndef LT768UI_DATA_H
#define LT768UI_DATA_H

#include "LT768_Lib.h"
#include "w25q.h"
#include "std_common.h"
#include "lt768ui_graphics.h"
#include "lt_trig.h"
#include "std_math.h"

typedef struct {
    uint64_t addr;
    int src_w;
    int x;
    int y;
    int w;
    int h;
}dma_info_t;

void ui_data_init();
void load_dma_info(dma_info_t *dma_info);

#endif