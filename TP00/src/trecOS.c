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
	uint32_t			contadorTaskID;
	uint32_t			idCurrentTask;
	uint8_t				indexCurrentTask;
	semaphoreHandle_t	contadorSemphrID;
}tosData;

/*==================[internal functions declaration]=========================*/
/** @brief Inicializa pila de tarea
 *	@return puntero de pila
 */
static 	uint32_t* 	iniStackTask	( uint32_t *stack , taskFunction_t functionName , void *argFunction );
static	void 		iniIdleTask		( void );
static	void 		callSchedule	( void );
static 	void* 		idleTask 		( void* );
static	void		semphrTake		( semaphoreHandle_t ids );
static	semphrState_t	semphrGetState	( semaphoreHandle_t ids );

/*==================[internal data definition]===============================*/

static 	taskData_t		taskData[ _TOS_MAX_TASK_ ];
static 	semphrData_t	semphrData[ _TOS_MAX_USER_SEMPHR_ ];

static	uint32_t		stackIdleTask[ _TOS_IDLE_TASK_STACK_SIZE_ ];
static	uint8_t			roundRobinControl[ _TOS_TASK_PRIORITY_IDLE_ + 1 ];		// Almacena el index de la ultima tarea ejecutada por cada nivel de prioridad

volatile static	uint32_t ticksCount;
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
	ticksCount++;
	// Escanea las tareas en estado wait
	for( uint8_t i = 0 ; i < _TOS_MAX_TASK_ ; i++ )
		if( taskData[ i ].state == _TOS_TASK_STATE_WAIT_
			&& taskData[ i ].delayTime > 0
			&& --taskData[ i ].delayTime == 0 )
		{
			taskData[ i ].state = _TOS_TASK_STATE_READY_;
		}
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
	taskData[ _TOS_IDLE_TASK_ID_INDEX_ ].state 		= _TOS_TASK_STATE_READY_;
	taskData[ _TOS_IDLE_TASK_ID_INDEX_ ].priority 	= _TOS_TASK_PRIORITY_IDLE_;
}

void		semphrTake				( semaphoreHandle_t ids )
{
	for( uint8_t idx = 0 ; idx < _TOS_MAX_USER_SEMPHR_ ; idx++ )
	{
		if( ids == semphrData[ idx ].id )
		{
			semphrData[ idx ].state = _TOS_SEMPHR_TAKEN_;
		}
	}
}

void		semphrWaitFree			( semaphoreHandle_t ids )
{
	for( uint8_t idx = 0 ; idx < _TOS_MAX_USER_SEMPHR_ ; idx++ )
	{
		if( ids == semphrData[ idx ].id )
		{
			while( _TOS_SEMPHR_TAKEN_ == semphrData[ idx ].state )
			{
				semphrData[ idx ].taskIdWaiting = tosData.idCurrentTask;
				taskData[ tosData.indexCurrentTask ].state 	= _TOS_TASK_STATE_WAIT_;
				taskData[ tosData.indexCurrentTask ].delayTime 	= 0;
				callSchedule();
			}
			return;
		}
	}
}

semphrState_t	semphrGetState			( semaphoreHandle_t ids )
{
	for( uint8_t idx = 0 ; idx < _TOS_MAX_USER_SEMPHR_ ; idx++ )
		if( ids == semphrData[ idx ].id )
			return semphrData[ idx ].state;

	return _TOS_SEMPHR_TAKEN_;
}

void			setTaskState 		( uint32_t idt , taskState_t ts )
{
	for( uint8_t idx = 0 ; idx < _TOS_MAX_TASK_ ; idx++ )
	{
		if( idt == taskData[ idx ].id )
		{
			taskData[ idx ].state = ts;
			return;
		}
	}
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
		if( _TOS_TASK_STATE_RUN_ == taskData[ tosData.indexCurrentTask ].state )
			taskData[ tosData.indexCurrentTask ].state = _TOS_TASK_STATE_READY_;
	}

	// Busca la siguiente tarea en READY por orden de prioridad
	for( int pry = _TOS_TASK_PRIORITY_0_ ; pry <= _TOS_TASK_PRIORITY_IDLE_ ; pry++ )
	{
		int idx = roundRobinControl[ pry ] + 1 == _TOS_MAX_TASK_ ? _TOS_IDLE_TASK_ID_INDEX_ : roundRobinControl[ pry ] + 1 ;
		int endIndex = idx;

		do
		{
			if( taskData[ idx ].state == _TOS_TASK_STATE_READY_ &&
				taskData[ idx ].priority == pry )
			{
				tosData.indexCurrentTask = idx;
				nextSP = taskData[ idx ].pStack;
				tosData.idCurrentTask = taskData[ idx ].id;
				taskData[ idx ].state = _TOS_TASK_STATE_RUN_;
				roundRobinControl[ pry ] = idx;
				return nextSP;
			}

			idx = ++idx == _TOS_MAX_TASK_ ? _TOS_IDLE_TASK_ID_INDEX_ : idx ;
		} while ( idx != endIndex);
	}

	// Si no encuentra tarea, ejecuta la IdleTask (En teoria esto nunca se debe ejecutar pero se deja por seguridad)
	nextSP 										= taskData[ _TOS_IDLE_TASK_ID_INDEX_ ].pStack;
	tosData.idCurrentTask 						= _TOS_IDLE_TASK_ID_INDEX_;
	tosData.indexCurrentTask 					= _TOS_IDLE_TASK_ID_INDEX_;
	taskData[ _TOS_IDLE_TASK_ID_INDEX_ ].state 	= _TOS_TASK_STATE_RUN_;

	return nextSP;
}

// Retorna id de la tarea creada o 0 si no pudo crearla.
uint32_t	tosAddTask_ui32		( uint32_t *pStack , taskFunction_t functionName , void *argFunction , taskPriority_t priority)
{
	for( uint8_t i = 1 ; i < _TOS_MAX_TASK_ ; i++)
	{
		if( taskData[ i ].state == _TOS_TASK_STATE_VOID_ )
		{
			if ( tosData.contadorTaskID != _TOS_NULL_ID_TASK_VALUE_ )
				taskData[ i ].id = ++tosData.contadorTaskID;
			else
				return 0;

			taskData[ i ].pStack = iniStackTask( pStack , functionName , argFunction );

			taskData[ i ].state = _TOS_TASK_STATE_READY_;
			taskData[ i ].priority = priority;

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
			taskData[ i ].state = _TOS_TASK_STATE_VOID_;
			return 1;
		}
	}
	return 0;
}


void 		tosDelayMs_v		( uint32_t timeMs )
{
	if( timeMs )
	{
		taskData[ tosData.indexCurrentTask ].state 	= _TOS_TASK_STATE_WAIT_;
		taskData[ tosData.indexCurrentTask ].delayTime 	= timeMs;
		callSchedule();
	}
}

void 		tosIniSchedule_v	( void )
{
	SysTick_Config(SystemCoreClock / 1000);			// Se fija a 1mS
	NVIC_SetPriority(PendSV_IRQn , (1 << __NVIC_PRIO_BITS) - 1 );
	iniIdleTask();
	tosData.idCurrentTask = _TOS_NULL_ID_TASK_VALUE_;
	callSchedule();
}

uint32_t 	tosGetTicks_ui32	()
{
	return ticksCount;
}

semaphoreHandle_t	tosSemaphoreNewBin( void )
{
	for( uint8_t i = 0 ; i < _TOS_MAX_USER_SEMPHR_ ; i++)
	{
		if( 0 == semphrData[ i ].id )
		{
			if ( tosData.contadorSemphrID != _TOS_SEMPHR_MAX_ID_ )
				semphrData[ i ].id = ++tosData.contadorSemphrID;
			else
				return 0; // Si no hay ID disponible retorna 0

			semphrData[ i ].state = _TOS_SEMPHR_TAKEN_;
			return semphrData[ i ].id;
		}
	}
	return 0;		// Si no encuentra un slot disponible retorna 0
}

void		tosSemaphoreTake( semaphoreHandle_t ids )
{
	if( _TOS_SEMPHR_FREE_ == semphrGetState( ids ) )
	{
		semphrTake( ids );
	}
	else
	{
		semphrWaitFree( ids );
		semphrTake( ids );
	}
}

void 			tosSemaphoreGive( semaphoreHandle_t ids )
{
	for( uint8_t idx = 0 ; idx < _TOS_MAX_USER_SEMPHR_ ; idx++ )
		if( ids == semphrData[ idx ].id && _TOS_SEMPHR_TAKEN_ == semphrData[ idx ].state )
		{

			setTaskState( semphrData[ idx ].taskIdWaiting , _TOS_TASK_STATE_READY_ );
			semphrData[ idx ].state = _TOS_SEMPHR_FREE_;
			semphrData[ idx ].taskIdWaiting = 0;
			// callSchedule();
			return;
		}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
