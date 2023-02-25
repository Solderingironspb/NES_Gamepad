#include "main.h"
#include "usb_device.h"
#include "usbd_customhid.h"

#define NSS_ON  GPIOA->BSRR = GPIO_BSRR_BR4;
#define NSS_OFF GPIOA->BSRR = GPIO_BSRR_BS4;

uint8_t NES_State = 0;
//Биты: 
//7 - A
//6 - B
//5 - Select
//4 - Start
//3 - Up
//2 - Down
//1 - Left
//0 - Right

extern PCD_HandleTypeDef hpcd_USB_FS;
extern USBD_HandleTypeDef hUsbDeviceFS;

typedef struct __attribute__((packed)) {
	int8_t x;
	int8_t y;
	uint8_t buttons;
}USB_Custom_HID_Gamepad;

USB_Custom_HID_Gamepad Gamepad_data;

void USB_LP_CAN1_RX0_IRQHandler(void) {
	HAL_PCD_IRQHandler(&hpcd_USB_FS);
}

void USB_HP_CAN1_TX_IRQHandler(void) {
    
}

void TIM3_IRQHandler(void) {
	if (READ_BIT(TIM3->SR, TIM_SR_UIF)) {
		NSS_ON;
		CMSIS_SPI_Data_Receive_8BIT(SPI1, &NES_State, 1, 5);
		NSS_OFF;
		
		if (NES_State)
		{
			GPIOC->BSRR = GPIO_BSRR_BS13;
		}
		else {
			GPIOC->BSRR = GPIO_BSRR_BR13;
		}
		
		if (READ_BIT(NES_State, (1 << 3)) && !READ_BIT(NES_State, (1 << 2)))
		{
			Gamepad_data.y = -128;
		}
		else if (READ_BIT(NES_State, (1 << 2)) && !READ_BIT(NES_State, (1 << 3))) {
			Gamepad_data.y = 127;
		}
		else if (READ_BIT(NES_State, (1 << 3)) && READ_BIT(NES_State, (1 << 2))) {
			Gamepad_data.y = 0;
		}
		else {
			Gamepad_data.y = 0;
		}
		
		if (READ_BIT(NES_State, (1 << 1)) && !READ_BIT(NES_State, (1 << 0)))
		{
			Gamepad_data.x = -128;
		}
		else if (READ_BIT(NES_State, (1 << 0)) && !READ_BIT(NES_State, (1 << 1))) {
			Gamepad_data.x = 127;
		}
		else if (READ_BIT(NES_State, (1 << 1)) && READ_BIT(NES_State, (1 << 0))) {
			Gamepad_data.x = 0;
		}
		else {
			Gamepad_data.x = 0;
		}
		
	}
		
	Gamepad_data.buttons = NES_State >> 4;
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t*)&Gamepad_data, sizeof(Gamepad_data));
	CLEAR_BIT(TIM3->SR, TIM_SR_UIF); //Сбросим флаг прерывания

}



int main(void) {
	CMSIS_Debug_init();
	CMSIS_RCC_SystemClock_72MHz();
	CMSIS_SysTick_Timer_init();
	CMSIS_SPI1_init();
	CMSIS_TIM3_init();
	CMSIS_PC13_OUTPUT_Push_Pull_init();
    
	//PA4 - NSS
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN); //Запуск тактирования порта A
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE4, 0b10 << GPIO_CRL_MODE4_Pos); //Настройка GPIOA Pin 4 на выход со максимальной скоростью в 50 MHz
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF4, 0b00 << GPIO_CRL_CNF4_Pos); //Настройка GPIOA Pin 4 на выход в режиме Push-Pull
	NSS_OFF;
	
	MX_USB_DEVICE_Init();
	
	while (1) {     
   
	}
  
}


