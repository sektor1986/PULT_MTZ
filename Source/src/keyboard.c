#include "keyboard.h"
#include "74hc595.h"
#include "adc.h"

unsigned int ADC_value = 0;

typedef struct stc_button_control
{	
	unsigned char  currState;
	unsigned char  debounceCounter;
	unsigned char  stateRepeat; 
	unsigned char  saveState;
	unsigned char  light;

} stc_button_control_t;

static stc_button_control_t  m_astcButtonCtrl[16];
static button_callback_func_ptr_t   m_pfnCallback;

void Button_init(button_callback_func_ptr_t pfnCallback)
{
	unsigned char i = 0;
	for (i = 0; i < 16; i++)
	{
		m_astcButtonCtrl[i].currState = StateLow;
		m_astcButtonCtrl[i].saveState = 0; 
		m_astcButtonCtrl[i].light = 1; 				
	}
	m_astcButtonCtrl[4].light = 0;
	m_astcButtonCtrl[5].light = 0;	
	m_astcButtonCtrl[6].light = 0;
	m_astcButtonCtrl[7].light = 0;
	Init74hc595();
	SetChanADC(ADC_CH0);
	m_pfnCallback = pfnCallback;	
}

void Button_update(void)
{
	unsigned char i = 0;

	unsigned int mask = 0;
	unsigned int stateLed = 0;
    unsigned int temp_i = 0x0001;
	en_button_state_t enSampledState;
                                                                                                                       
	for (i = 0; i < 16; i++)
	{
		/* Подаем напряжение на кнопку, которую считаваем*/
		Sethc595(temp_i << i);	
		/* Измеряем напряжение на выходе*/
		ConvertADC();
    	while(BusyADC());
		ADC_value = ReadADC();
		if (ADC_value > 100)
			enSampledState = StateHigh;
		else
			enSampledState = StateLow;
		/* Сравниваем состояние кнопки с предыдущим */
		if (m_astcButtonCtrl[i].currState != enSampledState)
		{
			m_pfnCallback(i, enSampledState);
			m_astcButtonCtrl[i].currState = enSampledState;
			if  (enSampledState == StateHigh)
				m_astcButtonCtrl[i].saveState = ~m_astcButtonCtrl[i].saveState;	
		}			 
		
	}
	/* В перерывах между сканированием зажигаем светодиоды*/
	for (i = 0; i < 16; i++)
	{
		if ((m_astcButtonCtrl[i].saveState) && (m_astcButtonCtrl[i].light))
			stateLed = stateLed | (temp_i << i); 			
	}
	Sethc595(stateLed);
}