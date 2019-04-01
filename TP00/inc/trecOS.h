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

#define  _TOS_MAX_ID_TASK_VALUE_		0xFFFFFFFF
#define  _TOS_ID_IDLE_TASK_				0

/*==================[typedef]================================================*/

typedef void* (*taskFunction_t)( void*);		// Prototipo de tareas del SO

/** prioridades */
typedef enum taskPriority_t {
	_TOS_TASK_PRIORITY_0_,
	_TOS_TASK_PRIORITY_1_,
	_TOS_TASK_PRIORITY_2_,
	_TOS_TASK_PRIORITY_3_,
}taskPriority_t;

/** Estados de una tarea */
typedef enum taskState_t {
	_TOS_TASK_STATE_VOID_,
	_TOS_TASK_STATE_READY_,
	_TOS_TASK_STATE_RUN_,
	_TOS_TASK_STATE_WAIT_,
	_TOS_TASK_STATE_TERMINATED_
}taskState_t;

/** estructura de definicón de tareas */
typedef struct taskStatus_t {
	taskState_t		state		:3		;
	taskPriority_t	priority	:2		;			// 4 niveles de prioridad 0: Alta 3: Baja
}taskStatus_t;

typedef struct taskData_t {
				uint32_t	id			;
				uint32_t*	pStack		;
	volatile	uint32_t	delayTime	;			// En ticks (ms)
}taskData_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

void 		tosIniOs_v 			( void );
uint32_t	tosAddTask_ui32		( uint32_t *pStack , taskFunction_t functionName , void *argFunction , taskPriority_t priority);

/*==================[internal functions declaration]=========================*/

/** @brief Inicializa pila de tarea
 *	@return puntero de pila
 */
// uint32_t* 	iniStackTask( uint32_t *stack , taskFunction_t functionName , void *argFunction );

void 		schedule( void );

void 		taskVoid( void );

void 		SysTick_Handler( void );

uint32_t*	getNextSP( uint32_t *currentSP );

void 		delayMs( uint32_t timeMs );
/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/
#endif /* #ifndef _TRECOS_H_ */
