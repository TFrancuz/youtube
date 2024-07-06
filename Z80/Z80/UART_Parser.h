/*
 * UART_Parser.h
 *
 * Created: 12/3/2023 12:29:38 PM
 *  Author: tmf
 */ 


#ifndef UART_PARSER_H_
#define UART_PARSER_H_

//Makro umieszczaj¹ce zadany ³añcuch w przestrzeni adresowej __flash
#define PGM_STR(X) ((const char[]) { X })

extern const char msg_UnknownCmd[];
extern const char msg_OK[];
extern const char msg_UnknownCmd[];		//Nie rozpoznane polecenie przes³ane z PC
extern const char msg_Err[];
extern const char msg_InvalidArgument[];
extern const char mgs_OutOfRange[];


_Bool GetUInt8Argument(char *param, char **last, uint8_t *val);		//Pobiera 8-bitowy argument
_Bool GetUInt16Argument(char *param, char **last, uint16_t *val);
_Bool GetUInt32Argument(char *param, char **last, uint32_t *val);


#endif /* UART_PARSER_H_ */