/* Copyleft 2019, Leonardo Urrego
 *
 */

#ifndef _MAIN_H_
#define _MAIN_H_

/** \addtogroup TP_16ISO_00 Cambio de contexto
 ** @{ */

/*==================[inclusions]=============================================*/

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/** UART USB de la CIAA */
// #define uartSendChar(x)   uartSend(1, (uint8_t *)(x), 1)
#define uartSendChar(x)   		uartSend( _CIAA_UART_USB_ , (uint8_t *)(&x) 	, 1 		)
#define uartSendString(x)   	uartSend( _CIAA_UART_USB_ , (uint8_t *)(x)	, strlen(x)	)
#define uartSendStruct(x)   	uartSend( _CIAA_UART_USB_ , (uint8_t *)(x)	, sizeof(x) )
#define uartSendBuffer(x , len) uartSend( _CIAA_UART_USB_ , (uint8_t *)(x)	, len 		)

/** LEDS de la EDU-CIAA */
#define _EDUCIAA_LED_R_		0
#define _EDUCIAA_LED_G_		1
#define _EDUCIAA_LED_B_		2
#define _EDUCIAA_LED_1_		3
#define _EDUCIAA_LED_2_		4
#define _EDUCIAA_LED_3_		5

/** Teclas De la EDU-CIAA */
#define _EDUCIAA_TEC_1_		0b0001
#define _EDUCIAA_TEC_2_		0b0010
#define _EDUCIAA_TEC_3_		0b0100
#define _EDUCIAA_TEC_4_		0b1000

#define tec1Press() 		( (uint8_t)Buttons_GetStatus() &  _EDUCIAA_TEC_1_ ) ? 1 : 0
#define tec2Press() 		( (uint8_t)Buttons_GetStatus() &  _EDUCIAA_TEC_2_ ) ? 1 : 0
#define tec3Press() 		( (uint8_t)Buttons_GetStatus() &  _EDUCIAA_TEC_3_ ) ? 1 : 0
#define tec4Press() 		( (uint8_t)Buttons_GetStatus() &  _EDUCIAA_TEC_4_ ) ? 1 : 0

/** delay in milliseconds */
#define _DELAY_T1_ 80
#define _DELAY_T2_ 500
#define _DELAY_T3_ 1000
#define _DELAY_T4_ 500

/** led number to toggle */
#define _LED_T1_ 	_EDUCIAA_LED_1_
#define _LED_T2_ 	_EDUCIAA_LED_2_
#define _LED_T3_ 	_EDUCIAA_LED_3_
#define _LED_T4_ 	_EDUCIAA_LED_B_




/*==================[typedef]================================================*/


/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/** @brief main function
 * @return main function should never return
 */
int main(void);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/
#endif /* #ifndef _MAIN_H_ */
