/* Copyleft 2019, Leonardo Urrego
 *
 */

#ifndef _TRECOS_H_
#define _TRECOS_H_

/** \addtogroup TRECOS_H Headers del Sistema Operativo
 ** @{ */

/*==================[inclusions]=============================================*/

#include "trecOS_config.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

typedef void* (*taskFunction_t)( void*);		// Prototipo de tareas del SO

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void 		vTosIniOs ( void );
void 		delayMs( uint32_t timeMs );

/*==================[internal functions declaration]=========================*/

/** @brief Inicializa pila de tarea
 *	@return puntero de pila
 */
uint32_t* 	iniStackTask( uint32_t *stack , taskFunction_t functionName , void *argFunction );

void 		schedule( void );

void 		taskVoid( void );

void 		SysTick_Handler( void );

uint32_t*	getNextSP( uint32_t *currentSP );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/
#endif /* #ifndef _TRECOS_H_ */
