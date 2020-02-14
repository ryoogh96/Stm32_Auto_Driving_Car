#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_usart.h"
#include "core_cm3.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_HC-SR04.h"
#include "ctype.h"
#include "stdlib.h"


//flash load "C:\Users\Team02\Desktop\team0808\team08\flashclear.axf"
//flash load "C:\Users\Team02\Desktop\team0808\team08\Debug\team08.axf"

char a;
char direction;
char mode = '2';
int32_t dist;

void EnableHCSR04PeriphClock(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
}

void GPIO_set() {
    GPIO_InitTypeDef gpioA9_USART1_TX, gpioA10_USART1_RX, gpioA2_USART2_TX, gpioA1_Piezo, gpioA3, gpioC_Motor, gpioD_LED;

    //바퀴
    //GPIOD Port D 2, 3, 4, 7
    //PD2:왼쪽 전진, PD3:왼쪽 후진, PD4:오른쪽 전진, PD7:오른쪽 후진
    gpioC_Motor.GPIO_Pin = (GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11);
    gpioC_Motor.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioC_Motor.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpioC_Motor);

    gpioD_LED.GPIO_Pin = (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7);
    gpioD_LED.GPIO_Mode = GPIO_Mode_Out_PP;
    gpioD_LED.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &gpioD_LED);

    //피에조
    gpioA1_Piezo.GPIO_Pin = GPIO_Pin_1;
    gpioA1_Piezo.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioA1_Piezo.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &gpioA1_Piezo);

    //USART2
    //USART2 TX: GPIOA Port 2
    // bluetooth to phone
    gpioA2_USART2_TX.GPIO_Pin = GPIO_Pin_2;
    gpioA2_USART2_TX.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioA2_USART2_TX.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioA2_USART2_TX);

    //USART2 RX: GPIOA Port 3
    // phone to bluetooth
    gpioA3.GPIO_Pin = GPIO_Pin_3;
    gpioA3.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &gpioA3);

    //USART1
    //USART1 TX: GPIOA Port 9
    //putty to board
    gpioA9_USART1_TX.GPIO_Pin = GPIO_Pin_9;
    gpioA9_USART1_TX.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioA9_USART1_TX.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioA9_USART1_TX);

    //USART1 RX: GPIOA Port 10
    //board to putty
    gpioA10_USART1_RX.GPIO_Pin = GPIO_Pin_10;
    gpioA10_USART1_RX.GPIO_Mode = GPIO_Mode_IPD;
    //gpioA10_USART1_RX.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioA10_USART1_RX);

}

void USART_Set() {
    USART_InitTypeDef usart;
    usart.USART_BaudRate = 9600;
    usart.USART_Mode = (USART_Mode_Tx | USART_Mode_Rx);
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_StopBits = USART_StopBits_1;

    USART_Init(USART1, &usart);
    USART_Cmd(USART1, ENABLE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    USART_Init(USART2, &usart);
    USART_Cmd(USART2, ENABLE);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void NVIC_set(){
    NVIC_InitTypeDef nvic12, nvic123;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    //USART1
    nvic12.NVIC_IRQChannel = USART1_IRQn;
    nvic12.NVIC_IRQChannelCmd = ENABLE;
    nvic12.NVIC_IRQChannelPreemptionPriority = 1;
    nvic12.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic12);

    //USART2
    nvic123.NVIC_IRQChannel = USART2_IRQn;
    nvic123.NVIC_IRQChannelCmd = ENABLE;
    nvic123.NVIC_IRQChannelPreemptionPriority = 1;
    nvic123.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic123);
}

void USART1_IRQHandler() {
    a = USART_ReceiveData(USART1);
    direction = a;
    USART_SendData(USART1, a);
    USART_SendData(USART2, a);
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
}

void USART2_IRQHandler() {
    a = USART_ReceiveData(USART2);
    if(isalpha(a)){
        direction = a;
    }
    else if(isdigit(a)){
        mode = a;
    }
    USART_SendData(USART1, a);
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}

void InitSound(int x)
{
    //22727(솔) 20242(라) 18050(시)17035(높은도)

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    TIM_TimeBaseStructure.TIM_Period = x;   // 높은 도
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // TIM_OCMode_PWM1, TIM_OCMode_PWM2
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Toggle;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = x/2;
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, DISABLE);
}


void SendToUSART2(char* str){
    int i;
    while(*str){
        USART_SendData(USART2, *str++);
        for (i = 0; i < 100000; i++);
    }
}


void SendToUSART1(char* str){
    int i;
    while(*str){
        USART_SendData(USART1, *str++);
        for (i = 0; i < 10000; i++);
    }
}

void delay() {
    int i;
    for (i = 0; i < 100000; i++);
}


void longdelay() {
    int i;
    for (i = 0; i < 8000000; i++);
}
void A_goStraight(){
	GPIO_SetBits(GPIOC, GPIO_Pin_8);
    GPIO_SetBits(GPIOC, GPIO_Pin_9);
}

void A_goBack(){
	GPIO_ResetBits(GPIOC, GPIO_Pin_8);
    GPIO_ResetBits(GPIOC, GPIO_Pin_9);
    longdelay();
    TIM_Cmd(TIM2, ENABLE);
    GPIO_SetBits(GPIOC, GPIO_Pin_10);
    GPIO_SetBits(GPIOC, GPIO_Pin_11);
    longdelay();
    TIM_Cmd(TIM2, DISABLE);
    GPIO_ResetBits(GPIOC, GPIO_Pin_10);
    GPIO_ResetBits(GPIOC, GPIO_Pin_11);
}

void A_goLeft(){
	 TIM_Cmd(TIM2, ENABLE);
     GPIO_SetBits(GPIOC, GPIO_Pin_8);
     GPIO_SetBits(GPIOC, GPIO_Pin_11);   //왼쪽으로 방향전환
     longdelay();
     TIM_Cmd(TIM2, DISABLE);
     GPIO_ResetBits(GPIOC, GPIO_Pin_8);
     GPIO_ResetBits(GPIOC, GPIO_Pin_11);
}
void A_goRight() {
	TIM_Cmd(TIM2, ENABLE);
    GPIO_SetBits(GPIOC, GPIO_Pin_9);
    GPIO_SetBits(GPIOC, GPIO_Pin_10);   //오른쪽으로 방향전환
    longdelay();
    TIM_Cmd(TIM2, DISABLE);
    GPIO_ResetBits(GPIOC, GPIO_Pin_9);
    GPIO_ResetBits(GPIOC, GPIO_Pin_10);
}

void M_goStraight(){
    GPIO_SetBits(GPIOC, GPIO_Pin_8);
    GPIO_SetBits(GPIOC, GPIO_Pin_9);
    delay();
}

void M_turnLeft(){
    GPIO_SetBits(GPIOC, GPIO_Pin_8);      //좌회전
    GPIO_SetBits(GPIOD, GPIO_Pin_7);      //좌회전
    delay();
    GPIO_ResetBits(GPIOD, GPIO_Pin_7);      //좌회전
    delay();
}

void M_turnRight(){
    GPIO_SetBits(GPIOC, GPIO_Pin_9);     // 우회전
    GPIO_SetBits(GPIOD, GPIO_Pin_2);      // 우회전
    delay();
    GPIO_ResetBits(GPIOD, GPIO_Pin_2);      // 우회전
    delay();
}

void M_goBack(){
    GPIO_SetBits(GPIOC, GPIO_Pin_10);    //후진
    GPIO_SetBits(GPIOC, GPIO_Pin_11);
    InitSound(20242);
    TIM_Cmd(TIM2, ENABLE);
    delay();
    TIM_Cmd(TIM2, DISABLE);
    InitSound(18050);
    TIM_Cmd(TIM2, ENABLE);
    delay();
    TIM_Cmd(TIM2, DISABLE);
    InitSound(22727);
    TIM_Cmd(TIM2, ENABLE);
    delay();
    TIM_Cmd(TIM2, DISABLE);
    delay();
}

void RCC_set() {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB1Periph_USART2, ENABLE);
}
void motorReset(){
    GPIO_ResetBits(GPIOC, GPIO_Pin_8);
    GPIO_ResetBits(GPIOC, GPIO_Pin_9);
    GPIO_ResetBits(GPIOC, GPIO_Pin_10);
    GPIO_ResetBits(GPIOC, GPIO_Pin_11);
}

int main() {
    SystemInit();
    RCC_set();
    GPIO_set();
    InitHCSR04();
    USART_Set();
    NVIC_set();

    longdelay();

    SendToUSART2("AT+BTSCAN\r\n");

    while(1){
        int random = (rand() % 10); //0~9
        int leftrigth = random % 2; // 0 or 1
        dist = HCSR04GetDistance();

        //자동
        //c : 1이면 자동, 2면 수동(블루투스)
        if(mode == '1') {
        	A_goStraight();
            if(dist < 160) {
                InitSound(17035);
                SendToUSART2("Detected\r\n");
                A_goBack();
            }
            if((dist < 300) && (160 <= dist) ) {
                InitSound(17035);
                SendToUSART2("Detected\r\n");
                motorReset();	//stop
                longdelay();
                if(leftrigth == 0){
                	A_goLeft();
                }
                else if(leftrigth == 1){
                	A_goRight();
                }
            }
        }
        // 수동
        else if (mode == '2'){
            if(direction == 'w'){
                M_goStraight();
            }
            else if(direction == 'a'){
                M_turnLeft();
            }
            else if(direction == 'd'){
                M_turnRight();

            }
            else if(direction == 's'){
                M_goBack();
            }
            else{
                motorReset();
            }
        }

    }
}
