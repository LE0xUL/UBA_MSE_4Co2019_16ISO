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

/*==================[internal functions declaration]=========================*/
/** @brief Inicializa pila de tarea
 *	@return puntero de pila
 */
static 	uint32_t* 	iniStackTask	( uint32_t *stack , taskFunction_t functionName , void *argFunction );
static	void 		schedule 		( void );
static 	void* 		idleTask 		( void* );


/*==================[internal data definition]===============================*/

static 	taskStatus_t		taskStatus[ _TOS_MAX_TASK_ ];
static 	taskData_t		taskData[ _TOS_MAX_TASK_ ];

static	uint32_t		stackIdleTask[ _TOS_IDLE_TASK_STACK_SIZE_ ];

/*==================[external data definition]===============================*/

/*==================[external functions declaration]=========================*/

void 		SysTick_Handler		( void );
uint32_t*	getNextSP			( uint32_t *currentSP );


/*==================[internal functions definition]==========================*/

void* idleTask( void *arg )
{
	while( 1 )
		__WFI();

	return 0;
}

void taskReturn()
{
	while( 1 )
		__WFI();
}

void schedule()
{
	__ISB();
	__DSB();
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void SysTick_Handler()
{
	// Escanea las tareas en estado wait
	for( uint8_t i = 0 ; i < _TOS_MAX_TASK_ ; i++ )
		if( taskStatus[ i ].state == _TOS_TASK_STATE_WAIT_ && --taskData[ i ].delayTime == 0)
			taskStatus[ i ].state 	= _TOS_TASK_STATE_READY_;

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

	// De ser necesario guarda el contexto y actualiza estado
	if( _TOS_NULL_ID_TASK_VALUE_ != tosData.idCurrentTask )
	{
		taskData[ tosData.indexCurrentTask ].pStack = currentSP;
		if( _TOS_TASK_STATE_RUN_ == taskStatus[ tosData.indexCurrentTask ].state )
			taskStatus[ tosData.indexCurrentTask ].state = _TOS_TASK_STATE_READY_;
	}

	// Busca la siguiente tarea en READY por orden de prioridad
	for( int pry = _TOS_TASK_PRIORITY_0_ ; pry <= _TOS_TASK_PRIORITY_IDLE_ ; pry++)
	{
		for( int idx = _TOS_IDLE_TASK_ID_INDEX_ ; idx < _TOS_MAX_TASK_ ; idx++ )
		{
			if( taskStatus[ idx ].state == _TOS_TASK_STATE_READY_ &&
				taskStatus[ idx ].priority == pry )
			{
				tosData.indexCurrentTask = idx;
				nextSP = taskData[ idx ].pStack;
				tosData.idCurrentTask = taskData[ idx ].id;
				taskStatus[ idx ].state = _TOS_TASK_STATE_RUN_;
				return nextSP;
			}
		}
	}

	// Si no encuentra tarea, ejecuta la IdleTask
	nextSP = taskData[ _TOS_IDLE_TASK_ID_INDEX_ ].pStack;
	tosData.idCurrentTask = _TOS_IDLE_TASK_ID_INDEX_;
	tosData.indexCurrentTask = _TOS_IDLE_TASK_ID_INDEX_;
	taskStatus[ _TOS_IDLE_TASK_ID_INDEX_ ].state = _TOS_TASK_STATE_RUN_;

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
		schedule();
	}
}


void 		tosIniOs_v 			( void )
{
	tosAddTask_ui32( &stackIdleTask[ _TOS_IDLE_TASK_STACK_SIZE_ - 1 ] , idleTask , 0 , _TOS_TASK_PRIORITY_IDLE_ );
}

void 		tosIniSchedule_v	( void )
{
	SysTick_Config(SystemCoreClock / 1000);			// Se fija a 1mS
	NVIC_SetPriority(PendSV_IRQn , (1 << __NVIC_PRIO_BITS) - 1 );
	tosData.idCurrentTask	= _TOS_NULL_ID_TASK_VALUE_;
	schedule();
}


/** @} doxygen end group definition */

/*==================[end of file]============================================*/
