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
static	void 		iniIdleTask		( void );
static	void 		callSchedule	( void );
static 	void* 		idleTask 		( void* );

/*==================[internal data definition]===============================*/

static 	taskStatus_t	taskStatus[ _TOS_MAX_TASK_ ];
static 	taskData_t		taskData[ _TOS_MAX_TASK_ ];

static	uint32_t		stackIdleTask[ _TOS_IDLE_TASK_STACK_SIZE_ ];
static	uint8_t			roundRobinControl[ _TOS_TASK_PRIORITY_IDLE_ + 1 ];		// Almacena el index de la ultima tarea ejecutada por cada nivel de prioridad

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

void callSchedule()
{
	__ISB();
	__DSB();
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void SysTick_Handler()
{
	// Escanea las tareas en estado wait
	for( uint8_t i = 0 ; i < _TOS_MAX_TASK_ ; i++ )
		if( taskStatus[ i ].state == _TOS_TASK_STATE_WAIT_ && --taskData[ i ].delayTime == 0 )
			taskStatus[ i ].state 	= _TOS_TASK_STATE_READY_;

	callSchedule();
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

void 		iniIdleTask 			( void )
{
	taskData[ _TOS_IDLE_TASK_ID_INDEX_ ].id 		= _TOS_IDLE_TASK_ID_INDEX_;
	taskData[ _TOS_IDLE_TASK_ID_INDEX_ ].pStack 	= iniStackTask( &stackIdleTask[ _TOS_IDLE_TASK_STACK_SIZE_ - 1 ] , idleTask , 0 );
	taskStatus[ _TOS_IDLE_TASK_ID_INDEX_ ].state 	= _TOS_TASK_STATE_READY_;
	taskStatus[ _TOS_IDLE_TASK_ID_INDEX_ ].priority = _TOS_TASK_PRIORITY_IDLE_;
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
	for( int pry = _TOS_TASK_PRIORITY_0_ ; pry <= _TOS_TASK_PRIORITY_IDLE_ ; pry++ )
	{
		int idx = roundRobinControl[ pry ] + 1 == _TOS_MAX_TASK_ ? _TOS_IDLE_TASK_ID_INDEX_ : roundRobinControl[ pry ] + 1 ;
		int endIndex = idx;

		do
		{
			if( taskStatus[ idx ].state == _TOS_TASK_STATE_READY_ &&
				taskStatus[ idx ].priority == pry )
			{
				tosData.indexCurrentTask = idx;
				nextSP = taskData[ idx ].pStack;
				tosData.idCurrentTask = taskData[ idx ].id;
				taskStatus[ idx ].state = _TOS_TASK_STATE_RUN_;
				roundRobinControl[ pry ] = idx;
				return nextSP;
			}

			idx = ++idx == _TOS_MAX_TASK_ ? _TOS_IDLE_TASK_ID_INDEX_ : idx ;
		} while ( idx != endIndex);
	}

	// Si no encuentra tarea, ejecuta la IdleTask (En teoria esto nunca se debe ejecutar pero se deja por seguridad)
	nextSP 											= taskData[ _TOS_IDLE_TASK_ID_INDEX_ ].pStack;
	tosData.idCurrentTask 							= _TOS_IDLE_TASK_ID_INDEX_;
	tosData.indexCurrentTask 						= _TOS_IDLE_TASK_ID_INDEX_;
	taskStatus[ _TOS_IDLE_TASK_ID_INDEX_ ].state 	= _TOS_TASK_STATE_RUN_;

	return nextSP;
}

// Retorna id de la tarea creada o 0 si no pudo crearla.
uint32_t	tosAddTask_ui32		( uint32_t *pStack , taskFunction_t functionName , void *argFunction , taskPriority_t priority)
{
	for( uint8_t i = 1 ; i < _TOS_MAX_TASK_ ; i++)
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

uint8_t		tosTaskDelete_ui8	( uint32_t idTask )
{
	for( uint8_t i = 1 ; i < _TOS_MAX_TASK_ ; i++)
	{
		if( idTask == taskData[ i ].id )
		{
			taskStatus[ i ].state = _TOS_TASK_STATE_VOID_;
			return 1;
		}
	}
	return 0;
}


void 		tosDelayMs_v		( uint32_t timeMs )
{
	if( timeMs )
	{
		taskStatus[ tosData.indexCurrentTask ].state 	= _TOS_TASK_STATE_WAIT_;
		taskData[ tosData.indexCurrentTask ].delayTime 	= timeMs;
		callSchedule();
	}
}

void 		tosIniSchedule_v	( void )
{
	SysTick_Config(SystemCoreClock / 1000);			// Se fija a 1mS
	NVIC_SetPriority(PendSV_IRQn , (1 << __NVIC_PRIO_BITS) - 1 );
	iniIdleTask();
	tosData.idCurrentTask	= _TOS_NULL_ID_TASK_VALUE_;
	callSchedule();
}


/** @} doxygen end group definition */

/*==================[end of file]============================================*/
