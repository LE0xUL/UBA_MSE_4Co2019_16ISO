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

// uint32_t		millis;

/*==================[internal functions declaration]=========================*/
/** @brief Inicializa pila de tarea
 *	@return puntero de pila
 */
static 		uint32_t* 	iniStackTask( uint32_t *stack , taskFunction_t functionName , void *argFunction );
static		void 		schedule( void );

static 		void* 		idleTask( void* );

	 		void 		SysTick_Handler( void );

/*==================[internal data definition]===============================*/

static taskStatus_t		taskStatus[ _TOS_MAX_TASK_ ];
static taskData_t		taskData[ _TOS_MAX_TASK_ ];

static	uint32_t		stackIdleTask[ _TOS_IDLE_TASK_STACK_SIZE_ ];
// static	uint32_t*		spIdleTask;
// static	uint32_t		idleTaskID;

/*==================[external data definition]===============================*/

/*==================[external functions declaration]=========================*/

uint32_t*	getNextSP			( uint32_t *currentSP );


/*==================[internal functions definition]==========================*/

void* idleTask( void *arg )
{
	// Board_LED_Set( _EDUCIAA_LED_R_ , TRUE );
	while( 1 )
	{
		__WFI();
	}

	return 0;
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
	// millis++;
	// Escanea las tareas en estado wait
	for( uint8_t i = 0 ; i < _TOS_MAX_TASK_ ; i++ )
	{
		if( taskStatus[ i ].state == _TOS_TASK_STATE_WAIT_ && --taskData[ i ].delayTime == 0)
		{
			taskStatus[ i ].state 	= _TOS_TASK_STATE_READY_;
		}
	}

	schedule();
}

uint32_t* iniStackTask( uint32_t *pStack , taskFunction_t functionName , void *argFunction )
{
	*pStack-- = 1 << 24;				// xPSR
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

	return pStack;
}


/*===========================================================================*/
/*==================[external functions definition]==========================*/
/*===========================================================================*/


uint32_t*	getNextSP( uint32_t *currentSP )
{
	uint32_t *nextSP;

	// Guardo El contexto y actualiza estado de ser necesario
	if( _TOS_NULL_ID_TASK_VALUE_ != tosData.idCurrentTask )
	{
		taskData[ tosData.indexCurrentTask ].pStack = currentSP;
		if( _TOS_TASK_STATE_RUN_ == taskStatus[ tosData.indexCurrentTask ].state )
			taskStatus[ tosData.indexCurrentTask ].state = _TOS_TASK_STATE_READY_;
	}

	// Busca una tarea en estado ready
	for( uint8_t i = _TOS_IDLE_TASK_ID_INDEX_ + 1 ; i < _TOS_MAX_TASK_ ; i++ )
	{
		if( taskStatus[ i ].state == _TOS_TASK_STATE_READY_ )
		{
			tosData.indexCurrentTask = i;
			nextSP = taskData[ i ].pStack;
			tosData.idCurrentTask = taskData[ i ].id;
			taskStatus[ i ].state = _TOS_TASK_STATE_RUN_;
			return nextSP;
		}
	}

	// Si no encuentra tarea, ejecuta la IdleTask
	nextSP = taskData[ _TOS_IDLE_TASK_ID_INDEX_ ].pStack;
	tosData.idCurrentTask = _TOS_IDLE_TASK_ID_INDEX_;
	tosData.indexCurrentTask = _TOS_IDLE_TASK_ID_INDEX_;
	taskStatus[ _TOS_IDLE_TASK_ID_INDEX_ ].state = _TOS_TASK_STATE_RUN_;

	// if( ++tosData.indexCurrentTask == _TOS_MAX_TASK_ )
	// 	tosData.indexCurrentTask = 0;

	// nextSP = taskData[ tosData.indexCurrentTask ].pStack;

	// tosData.idCurrentTask = taskData[ tosData.indexCurrentTask ].id;

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

// Retorna id de la tarea creada o 0 si no pudo crearla.
uint32_t	tosAddTask_ui32		( uint32_t *pStack , taskFunction_t functionName , void *argFunction , taskPriority_t priority)
{
	for( uint8_t i = 0 ; i < _TOS_MAX_TASK_ ; i++)
	{
		if( taskStatus[ i ].state == _TOS_TASK_STATE_VOID_ )
		{
			if ( tosData.contadorID != _TOS_NULL_ID_TASK_VALUE_ )
				taskData[ i ].id = tosData.contadorID++;
			else
				return 0;

			taskData[ i ].pStack = iniStackTask( pStack , functionName , argFunction );

			taskStatus[ i ].state = _TOS_TASK_STATE_READY_;
			taskStatus[ i ].priority = priority;

			return taskData[ i ].id;
		}
	}
	return 0;		// Si no encuentra un slot disponible retorna 0
}

void 		tosDelayMs_v		( uint32_t timeMs )
{
	if( timeMs )
	{
		taskStatus[ tosData.indexCurrentTask ].state 	= _TOS_TASK_STATE_WAIT_;
		taskData[ tosData.indexCurrentTask ].delayTime 	= timeMs;
		// uint32_t timeActual = millis;
		// while( millis - timeActual < timeMs)
		// 	__WFI();
		schedule();
	}
}


void 		tosIniOs_v 			( void )
{
	// millis	= 0;
	tosAddTask_ui32( &stackIdleTask[ _TOS_IDLE_TASK_STACK_SIZE_ - 1 ] , idleTask , 0 , _TOS_TASK_PRIORITY_IDLE_ );
}

void 		tosIniSchedule_v	( void )
{
	SysTick_Config(SystemCoreClock / 1000);			// Se fija a 1mS
	NVIC_SetPriority(PendSV_IRQn , (1 << __NVIC_PRIO_BITS) - 1 );
	tosData.idCurrentTask	= _TOS_NULL_ID_TASK_VALUE_;
	schedule();
	// idleTask( 0 );
}


/** @} doxygen end group definition */

/*==================[end of file]============================================*/
