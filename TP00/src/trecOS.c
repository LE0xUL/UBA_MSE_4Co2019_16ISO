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

volatile uint32_t millis;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

uint32_t*	pTosArrSP[ _TOS_NUM_TASK_ ];

uint32_t	currentTask;

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

void schedule()
{
	__ISB();
	__DSB();
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void SysTick_Handler()
{
	schedule();
}

uint32_t* iniStackTask( uint32_t *stack , taskFunction_t functionName , void *argFunction )
{
	*stack-- = 1 << 24;					// xPSR
	*stack-- = (uint32_t)functionName;	// PC
	*stack-- = (uint32_t)taskVoid;		// LR
	*stack-- = 0;						// R12
	*stack-- = 0;						// R3
	*stack-- = 0;						// R2
	*stack-- = 0;						// R1
	*stack-- = (uint32_t)argFunction;	// R0

	*stack-- = 0xFFFFFFF9;				// LR IRQ
	*stack-- = 0;						// R11
	*stack-- = 0;						// R10
	*stack-- = 0;						// R9
	*stack-- = 0;						// R8
	*stack-- = 0;						// R7
	*stack-- = 0;						// R6
	*stack-- = 0;						// R5
	*stack-- = 0;						// R4

	stack++;

	pTosArrSP[ currentTask++ ] = stack;

	return stack;
}

void delayMs( uint32_t timeMs )
{
	uint32_t timeActual = millis;
	while( millis - timeActual < timeMs)
		__WFI();
}


uint32_t*	getNextSP( uint32_t *currentSP )
{
	uint32_t *nextSP;
	switch( currentTask )
	{
		case 0:
			nextSP = pTosArrSP[ 0 ];
			currentTask = 1;
		break;

		case 1:
			pTosArrSP[ 0 ] = currentSP;
			nextSP = pTosArrSP[ 1 ];
			currentTask = 2;
		break;

		case 2:
			pTosArrSP[ 1 ] = currentSP;
			nextSP = pTosArrSP[ 2 ];
			currentTask = 3;
		break;

		case 3:
			pTosArrSP[ 2 ] = currentSP;
			nextSP = pTosArrSP[ 0 ];
			currentTask = 1;
		break;

		default:
			while (1)
				__WFI();
	}

	return nextSP;
}

void vTosIniOs ( void )
{
	millis			= 0;
	currentTask		= 0;
}

/*==================[external functions definition]==========================*/


/** @} doxygen end group definition */

/*==================[end of file]============================================*/
