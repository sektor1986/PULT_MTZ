#ifndef __KEYBOARD_H
#define __KEYBOARD_H

                                    
/** Set the scan interval for reading the pin state in milliseconds (20 ... 100) */
#define BUTTON_SCAN_INTERVAL_MS     20

/** Set debounce count i.e. number of scan intervals which must have the same state (1 ... 50) */
#define BUTTON_DEBOUNCE_COUNT       3

/*****************************************************************************/
/* Global type definitions ('typedef')                                       */
/*****************************************************************************/
typedef enum en_button_state
{
    StateInvalid,
    StateLow,
    StateHigh,
    StateLong
} en_button_state_t;

typedef void (*button_callback_func_ptr_t)(unsigned char u16ButtonId, en_button_state_t enState);

extern unsigned int ADC_value;

void Button_update(void);
void Button_init(button_callback_func_ptr_t pfnCallback);

#endif // __KEYBOARD_H