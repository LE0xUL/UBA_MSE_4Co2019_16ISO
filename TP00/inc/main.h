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

#define _EDUCIAA_LED_R_		0
#define _EDUCIAA_LED_G_		1
#define _EDUCIAA_LED_B_		2
#define _EDUCIAA_LED_1_		3
#define _EDUCIAA_LED_2_		4
#define _EDUCIAA_LED_3_		5

/** delay in milliseconds */
#define _DELAY_T1_ 100
#define _DELAY_T2_ 500
#define _DELAY_T3_ 1000

/** led number to toggle */
#define _LED_T1_ 	_EDUCIAA_LED_1_
#define _LED_T2_ 	_EDUCIAA_LED_2_
#define _LED_T3_ 	_EDUCIAA_LED_3_

/*==================[typedef]================================================*/

typedef void* (*taskFunction_t)( void*);		// Prototipo de tareas del SO


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
