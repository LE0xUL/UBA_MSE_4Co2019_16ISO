/* Copyleft 2019, Leonardo Urrego
 *
 */

/** @brief This is a simple blink example.
 */

/** \addtogroup TP_16ISO_00 Cambio de contexto
 ** @{ */

/*==================[inclusions]=============================================*/

#include "board.h"
#include "trecOS.h"
#include "main.h"

/*==================[macros and definitions]=================================*/

/** tamaño de pila por defecto para los threads */
#define _DEFAULT_STACK_SIZE_ 128				// Tamaño de palabra 32 bits


/*==================[internal data declaration]==============================*/

volatile uint32_t millis;

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware( void );


/*==================[internal data definition]===============================*/


/*==================[external data definition]===============================*/

uint32_t	stackTask1[ _DEFAULT_STACK_SIZE_ ];
uint32_t	stackTask2[ _DEFAULT_STACK_SIZE_ ];
uint32_t	stackTask3[ _DEFAULT_STACK_SIZE_ ];

uint32_t	*sp1;
uint32_t	*sp2;
uint32_t	*sp3;

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
}

/*==================[external functions definition]==========================*/

void* 		task1( void *arg )
{
	while (1)
	{
		Board_LED_Toggle(_LED_T1_);
		tosDelayMs_v( _DELAY_T1_ );
	}
	return 0;
}

void* 		task2( void *arg )
{
	while (1)
	{
		Board_LED_Toggle(_LED_T2_);
		tosDelayMs_v( _DELAY_T2_ );
	}
	return 0;
}

void* 		task3( void *arg )
{
	while (1)
	{
		Board_LED_Toggle(_LED_T3_);
		tosDelayMs_v( _DELAY_T3_ );
	}
	return 0;
}

int main(void)
{
	initHardware();

	tosIniOs_v();

	tosAddTask_ui32( &stackTask1[ _DEFAULT_STACK_SIZE_ - 1 ] , task1 , (void *)0x11223344 , _TOS_TASK_PRIORITY_0_);
	tosAddTask_ui32( &stackTask2[ _DEFAULT_STACK_SIZE_ - 1 ] , task2 , (void *)0x11223344 , _TOS_TASK_PRIORITY_0_);
	tosAddTask_ui32( &stackTask3[ _DEFAULT_STACK_SIZE_ - 1 ] , task3 , (void *)0x11223344 , _TOS_TASK_PRIORITY_0_);

	tosIniSchedule_v();

	while (1)
	{
		__WFI();
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
