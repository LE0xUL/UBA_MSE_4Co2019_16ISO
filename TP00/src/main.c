/* Copyleft 2019, Leonardo Urrego
 *
 */

/** @brief This is a simple blink example.
 */

/** \addtogroup TP_16ISO_00 Cambio de contexto
 ** @{ */

/*==================[inclusions]=============================================*/

#include "main.h"
#include "board.h"

/*==================[macros and definitions]=================================*/

/** tamaño de pila para los threads */
#define _STACK_SIZE_ 128			// Tamaño de palabra 32 bits

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void);

/** @brief Inicializa pila de tarea
 *	@return puntero de pila
 */
static uint32_t* iniStackTask( uint32_t *stack , taskFunction_t functionName , void *argFunction );


/*==================[internal data definition]===============================*/


/*==================[external data definition]===============================*/

uint32_t	stackTask1[ _STACK_SIZE_ ];
uint32_t	stackTask2[ _STACK_SIZE_ ];

uint32_t	*sp1;
uint32_t	*sp2;

uint32_t	currentTask;

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);			// Se fija a 1mS
}

void taskVoid()
{
	while( 1 )
	{
		__WFI();
	}
}

static uint32_t* iniStackTask( uint32_t *stack , taskFunction_t functionName , void *argFunction )
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

	return ++stack;
}

/*==================[external functions definition]==========================*/

void* 		task1( void *arg )
{
	Board_LED_Toggle(LED);
	while (1)
		__WFI();
	return 0;
}

void* 		task2( void *arg )
{
	Board_LED_Toggle(LED);
	while (1)
		__WFI();
	return 0;
}

uint32_t*	getNextSP( uint32_t *currentSP )
{
	uint32_t *nextSP;
	switch( currentTask )
	{
		case 0:
			nextSP = sp1;
			currentTask = 1;
		break;

		case 1:
			sp1 = currentSP;
			nextSP = sp2;
			currentTask = 2;
		break;

		case 2:
			sp2 = currentSP;
			nextSP = sp1;
			currentTask = 1;
		break;

		default:
			while (1)
				__WFI();
	}

	return nextSP;
}

int main(void)
{
	sp1 = iniStackTask( &stackTask1[ _STACK_SIZE_ - 1 ] , task1 , (void *)0x11223344 );
	sp2 = iniStackTask( &stackTask2[ _STACK_SIZE_ - 1 ] , task2 , (void *)0x11223344 );

	initHardware();

	while (1)
	{
		__WFI();
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
