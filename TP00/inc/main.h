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

/** delay in milliseconds */
#define DELAY_MS 100

/** led number to toggle */
#define LED 0

/*==================[typedef]================================================*/

typedef void* (*taskFunction_t)( void*);


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
