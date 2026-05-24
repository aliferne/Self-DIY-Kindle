#include "bsp_irq_hal.h"
#include "bsp_config.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == pgup_btn.Pinx) {
        
    } else if (GPIO_Pin == pgdown_btn.Pinx) {

    } else if (GPIO_Pin == back_btn.Pinx) {

    } else if (GPIO_Pin == confirm_btn.Pinx) {

    } else if (GPIO_Pin == home_btn.Pinx) {
    }
}
