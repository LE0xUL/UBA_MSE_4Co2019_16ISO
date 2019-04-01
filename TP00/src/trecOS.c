/* Copyleft 2019, Leonardo Urrego
 *
 */

/** @brief This is a simple OS Implementation.
 */

/** \addtogroup TRECOS Simple OS
 ** @{ */

/*==================[inclusions]=============================================*/

#include "board.h"
#include "trecOS.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

static struct
{
	uint32_t	contadorID;
	uint32_t	idCurrentTask;
	uint8_t		indexCurrentTask;
}tosData;

uint32_t		millis;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

static taskStatus_t		taskStatus[ _TOS_MAX_TASK_ ];
static taskData_t		taskData[ _TOS_MAX_TASK_ ];

/*==================[external data definition]===============================*/


/*==================[internal functions definition]==========================*/

void taskVoid()
{
	// Board_LED_Set( _EDUCIAA_LED_R_ , TRUE );
	while( 1 )
	{
		__WFI();
	}
}

void taskReturn()
{
	// Board_LED_Set( _EDUCIAA_LED_R_ , TRUE );
	while( 1 )
	{
		__WFI();
	}
}

void schedule()
{
	__ISB();
	__DSB();
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void SysTick_Handler()
{
	millis++;

	schedule();
}

void tosIniOs_v ( void )
{
	millis			= 0;
	tosData.indexCurrentTask		= 0;

}

// uint32_t* iniStackTask( uint32_t *stack , taskFunction_t functionName , void *argFunction )
// Retorna id de la tarea creada o 0 si no pudo crearla.
uint32_t		tosAddTask_ui32		( uint32_t *pStack , taskFunction_t functionName , void *argFunction , taskPriority_t priority)
{
	for( uint8_t i = 0 ; i < _TOS_MAX_TASK_ ; i++)
	{
		if( taskStatus[ i ].state == _TOS_TASK_STATE_VOID_ )
		{
			if ( tosData.contadorID !=  _TOS_MAX_ID_TASK_VALUE_ )
				taskData[ i ].id = ++tosData.contadorID;
			else
				return 0;

			*pStack-- = 1 << 24;					// xPSR
			*pStack-- = (uint32_t)functionName;	// PC
			*pStack-- = (uint32_t)taskReturn;	// LR
			*pStack-- = 0;						// R12
			*pStack-- = 0;						// R3
			*pStack-- = 0;						// R2
			*pStack-- = 0;						// R1
			*pStack-- = (uint32_t)argFunction;	// R0

			*pStack-- = 0xFFFFFFF9;				// LR IRQ
			*pStack-- = 0;						// R11
			*pStack-- = 0;						// R10
			*pStack-- = 0;						// R9
			*pStack-- = 0;						// R8
			*pStack-- = 0;						// R7
			*pStack-- = 0;						// R6
			*pStack-- = 0;						// R5
			*pStack-- = 0;						// R4

			pStack++;

			taskData[ i ].pStack = pStack;

			taskStatus[ i ].state = _TOS_TASK_STATE_READY_;
			taskStatus[ i ].priority = priority;

			return taskData[ i ].id;
		}
	}
	return 0;		// Si no encuentra un slot disponible retorna 0
}

void delayMs( uint32_t timeMs )
{
	uint32_t timeActual = millis;
	while( millis - timeActual < timeMs)
		__WFI();
		// schedule();
}


uint32_t*	getNextSP( uint32_t *currentSP )
{
	uint32_t *nextSP;

	// Guardo El contexto
	if( tosData.idCurrentTask != _TOS_ID_IDLE_TASK_ )
	{
		taskData[ tosData.indexCurrentTask ].pStack = currentSP;
	}


	if( ++tosData.indexCurrentTask == _TOS_MAX_TASK_ )
		tosData.indexCurrentTask = 0;

	nextSP = taskData[ tosData.indexCurrentTask ].pStack;

	tosData.idCurrentTask = taskData[ tosData.indexCurrentTask ].id;

	// switch( currentTask )
	// {
	// 	case 0:
	// 		nextSP = pTosArrSP[ 0 ];
	// 		currentTask = 1;
	// 	break;

	// 	case _TOS_NUM_TASK_:
	// 		pTosArrSP[ _TOS_NUM_TASK_ - 1 ] = currentSP;
	// 		nextSP = pTosArrSP[ 0 ];
	// 		currentTask = 1;
	// 	break;

	// 	default:
	// 		pTosArrSP[ currentTask - 1 ] = currentSP;
	// 		nextSP = pTosArrSP[ currentTask ];
	// 		currentTask++;
	// }

	return nextSP;
}

/*==================[external functions definition]==========================*/


/** @} doxygen end group definition */

/*==================[end of file]============================================*/
