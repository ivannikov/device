#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <misc.h>
#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include <stm32f4xx_exti.h>
#include <stm32f4xx_syscfg.h>
#include <hd44780_driver.h>
void Delay(__IO uint32_t nCount);
uint32_t time;
uint32_t i,n;
u16 flag = 0;
uint16_t counter=0; //Счётчик
void EXTILine1_Config(void);
void EXTILine0_Config(void);
void EXTILine2_Config(void);
void EXTILine3_Config(void);
void TIM6_DAC_IRQHandler();
void PORT_D_Config(void);
void init_timer(void);
// Порт к которому подключен индикатор E
#define IND_PORT GPIOE
// Общие выводы индикатора
#define D3 GPIO_Pin_14
#define D2 GPIO_Pin_12
#define D1 GPIO_Pin_10
#define D0 GPIO_Pin_8
// К какой ноге какой сегмент подключен
#define SEG_A GPIO_Pin_2
#define SEG_B GPIO_Pin_0
#define SEG_C GPIO_Pin_4
#define SEG_D GPIO_Pin_5
#define SEG_E GPIO_Pin_6
#define SEG_F GPIO_Pin_3
#define SEG_G GPIO_Pin_7
//Собираем цифры из сегментов
#define DIG0 ( SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F )
#define DIG1 ( SEG_B | SEG_C )
#define DIG2 ( SEG_A | SEG_B | SEG_G | SEG_E | SEG_D )
#define DIG3 ( SEG_A | SEG_B | SEG_G | SEG_C | SEG_D )
#define DIG4 ( SEG_F | SEG_G | SEG_B | SEG_C)
#define DIG5 ( SEG_A | SEG_F | SEG_G | SEG_C | SEG_D )
#define DIG6 ( SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G )
#define DIG7 ( SEG_A | SEG_B | SEG_C )
#define DIG8 ( SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define DIG9 ( SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)
#define ALL_PINS (DIG8 | D0 | D1 | D2 | D3 )
//Функция выводит в порт нужную цифру
#define LCD_PORT GPIOD
#define LCD_CD 0
#define LCD_EN 1
#define LCD_DB4 2
#define LCD_DB5 3
#define LCD_DB6 4
#define LCD_DB7 5

void digit_to_port (uint8_t digit) {
uint8_t digitsp[]={DIG0,DIG1,DIG2,DIG3,DIG4,DIG5,DIG6,DIG7,DIG8,DIG9};
GPIOE->ODR &= ~DIG8; //Выключаем все сегменты
GPIOE->ODR |= digitsp[digit]; //Зажигаем нужные
}
int main(void) {
i=2000;  // время свечения
n=1;

uint16_t tmp; // Содержит копию counter
// (из него по очереди исключаются последние цифры)
uint8_t digit; // В эту переменную поочередно записываются цифры
// из которых состоит число counter
//uint8_t button_flag=0;
time=300; // делитель 8000-1, время 1:3 ???
EXTILine0_Config();
EXTILine1_Config();
EXTILine2_Config();
EXTILine3_Config();
PORT_D_Config();  
init_timer();
 while (1)   {
//counter=time/10;
tmp=10000/time*6/2*10;
//Вытаскиваем первую справа цифру из числа counter
digit = tmp % 10;
tmp = tmp / 10;
//Выключаем все разряды
GPIO_ResetBits(GPIOE, D0|D1|D2|D3);
//Выводим цифру в нулевой разряд
digit_to_port(digit);
//Включаем нулевой разряд индикатора
GPIO_SetBits(GPIOE, D0); 
Delay(i);
//Вытаскиваем вторую справа цифру из числа counter
digit = tmp % 10;
tmp = tmp / 10;
//Выключаем все разряды
GPIO_ResetBits(GPIOE, D0|D1|D2|D3);
//Выводим цифру в первый разряд
digit_to_port(digit);
//Включаем первый разряд индикатора
GPIO_SetBits(GPIOE, D1); 
Delay(i);
//Вытаскиваем третью справа цифру из числа counter
digit = tmp % 10;
tmp = tmp / 10;
//Выключаем все разряды
GPIO_ResetBits(GPIOE, D0|D1|D2|D3);
//Выводим цифру в второй разряд
digit_to_port(digit);

//Включаем второй разряд индикатора
GPIO_SetBits(GPIOE, D2|GPIO_Pin_1); 
Delay(i);
//Тут мы цифр не вытаскиваем. В переменной counter уже самая старшая цифра
//Выключаем все разряды
GPIO_ResetBits(GPIOE, D0|D1|D2|D3|GPIO_Pin_1);
//Выводим цифру в третий разряд
digit_to_port(tmp);
//Включаем третий разряд индикатора
GPIO_SetBits(GPIOE, D3); 
Delay(i);

             }
}
void Delay(__IO uint32_t nCount) {
  while(nCount--);               }
void EXTI0_IRQHandler(void) {  // РА0
time=time+15;
//Delay(i*100);
EXTI_ClearITPendingBit(EXTI_Line0);}
void EXTI1_IRQHandler(void) {  // PA1
  if (time>=30)  {time=time-15;};
 // Delay(i*100);
EXTI_ClearITPendingBit(EXTI_Line1);}
void EXTI2_IRQHandler(void) {  // PA2
time=time+150;
//Delay(i*100);
EXTI_ClearITPendingBit(EXTI_Line2);}
void EXTI3_IRQHandler(void) {  // PA3
  if (time>300)  {time=time-150;};
//  Delay(i*100);
EXTI_ClearITPendingBit(EXTI_Line3);}
void TIM6_DAC_IRQHandler() {// обработчик переполнения таймера
if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
  {  
if (flag==0 ) {GPIO_SetBits(GPIOD, GPIO_Pin_14); flag++;}
else {GPIO_ResetBits(GPIOD, GPIO_Pin_14); flag=0;}
  
TIM6->ARR = time;


  }
TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
}



