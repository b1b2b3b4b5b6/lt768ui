#include "lt768ui.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_DEBUG

void lt768ui_init()
{
    interface_init();
	interactive_init();
    lt768ui_loop_init();
    STD_LOGI("lt768ui init success");
}