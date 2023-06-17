/*
 * PinMacros.h
 *
 * Created: 2018-09-08 10:37:53
 *  Author: tmf
 */ 


#ifndef PINMACROS_H_
#define PINMACROS_H_

#define xCAT2(a,b) a##b
#define CAT2(a,b) xCAT2(a,b)

#define xCAT3(a,b,c) a##b##c
#define CAT3(a,b,c) xCAT3(a,b,c)


#define CAT_PORT(a,b,c) xCAT2(a,b)
#define Port(a) CAT_PORT(PORT,a)

#define CAT_PIN_CTRL4a(a,b,c,d) xCAT3(a,c,d)
#define Pin_control(a)  CAT_PIN_CTRL4a(PIN,a,CTRL)

#define xISOLATE_PIN_NUMBER(a, b) b
#define ISOLATE_PIN_NUMBER(a, b) xISOLATE_PIN_NUMBER(a,b)
#define Pin(a)  ISOLATE_PIN_NUMBER(a)

#define Shift_TX_PIN(a) (1 << (a*4 +3))
#define TX_Pin(my_uart)  Shift_TX_PIN(ISOLATE_PIN_NUMBER(my_uart))

#define Shift_SCK_PIN(a) (1 << (a*4 +1))
#define SCK_Pin(my_uart)  Shift_SCK_PIN(ISOLATE_PIN_NUMBER(my_uart))

#define Make_Uart_Name(uart)  CAT2(USART, uart)
#define Uart(my_uart)  Make_Uart_Name(CAT2(my_uart))

#endif /* PINMACROS_H_ */