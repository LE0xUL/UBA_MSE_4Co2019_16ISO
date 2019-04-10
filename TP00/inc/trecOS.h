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

#define _TOS_NULL_ID_TASK_VALUE_		0xFFFFFFFF
#define _TOS_MAX_TASK_ 					_TOS_MAX_USER_TASK_ + 1			// Numero Máximo de tareas del SO
#define _TOS_IDLE_TASK_ID_INDEX_		0
#define _TOS_IDLE_TASK_STACK_SIZE_		64
#define _TOS_SEMPHR_MAX_ID_				0xFF

/*==================[typedef]================================================*/

typedef void* (*taskFunction_t)( void*);		// Prototipo de tareas del SO

typedef uint8_t		semaphoreHandle_t;

/** prioridades */
typedef enum taskPriority_t
{
	_TOS_TASK_PRIORITY_0_,						// Alta Prioridad
	_TOS_TASK_PRIORITY_1_,
	_TOS_TASK_PRIORITY_2_,
	_TOS_TASK_PRIORITY_IDLE_					// Baja Prioridad
}taskPriority_t;

/** Estados de una tarea */
typedef enum taskState_t
{
	_TOS_TASK_STATE_VOID_,
	_TOS_TASK_STATE_READY_,
	_TOS_TASK_STATE_RUN_,
	_TOS_TASK_STATE_WAIT_,
	_TOS_TASK_STATE_TERMINATED_
}taskState_t;

/** estructura de definicón de tareas */
typedef struct taskData_t {
	volatile	taskState_t		state		;
				taskPriority_t	priority	;			// 4 niveles de prioridad 0: Alta 3: Baja
				uint32_t		id			;
				uint32_t*		pStack		;
	volatile	uint32_t		delayTime	;			// En ticks (ms)
}taskData_t;

/** Estados para un semaforo binario */
typedef enum semphrState_t
{
	_TOS_SEMPHR_TAKEN_,
	_TOS_SEMPHR_FREE_
}semphrState_t;

/** Estructura para un semaforo binario */
typedef struct semphrData_t
{
	semaphoreHandle_t	id 				;
	semphrState_t		state			;
	uint32_t			taskIdWaiting	;
}semphrData_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

// void 		tosIniOs_v 			( void );
void 		tosIniSchedule_v	( void );
void 		tosDelayMs_v		( uint32_t timeMs );
uint32_t	tosAddTask_ui32		( uint32_t *pStack , taskFunction_t functionName , void *argFunction , taskPriority_t priority);
uint8_t		tosTaskDelete_ui8	( uint32_t idTask );

semaphoreHandle_t	tosSemaphoreNewBin	( void );
semphrState_t		tosSemaphoreTake	( semaphoreHandle_t ids );
void 				tosSemaphoreGive	( semaphoreHandle_t ids );

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/
#endif /* #ifndef _TRECOS_H_ */
