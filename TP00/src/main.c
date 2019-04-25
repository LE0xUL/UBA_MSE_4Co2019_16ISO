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
#include "ciaaUART.h"

/*==================[macros and definitions]=================================*/

/** tamaño de pila por defecto para los threads */
#define _DEFAULT_STACK_SIZE_ 128				// Tamaño de palabra 32 bits


/*==================[internal data declaration]==============================*/

uint32_t ticksDelay;

semaphoreHandle_t 	semaforoBinario;

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
uint32_t	stackTask4[ _DEFAULT_STACK_SIZE_ ];

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
	ciaaUARTInit();
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
		// Board_LED_Toggle(_LED_T2_);
		// tosDelayMs_v( _DELAY_T2_ );
		uint32_t duracionTeclaPresionada;
		uint32_t ticsIni;
		uint32_t ticsFin;
		uint8_t	 stdTeclas = (uint8_t)Buttons_GetStatus();
		if( tec2Press() )
		{
			ticsIni = tosGetTicks();
			// uartSend( CIAA_UART_USB , &stdTeclas , 1 );
			Board_LED_Set(_LED_T2_ , 1 );
			while( tec2Press() );
			ticsFin = tosGetTicks();
			duracionTeclaPresionada = ticsFin - ticsIni;
			ticksDelay = duracionTeclaPresionada;
			tosSemaphoreGive( semaforoBinario );
		}
		else
			Board_LED_Set(_LED_T2_ , 0 );

 	}
	return 0;
}

void* 		task3( void *arg )
{
	uint32_t idTask;
	while (1)
	{
		Board_LED_Toggle(_LED_T3_);
		idTask = tosAddTask_ui32( &stackTask1[ _DEFAULT_STACK_SIZE_ - 1 ] , task1 , (void *)0x11223344 , _TOS_TASK_PRIORITY_1_);
		tosDelayMs_v( _DELAY_T3_ );

		Board_LED_Toggle(_LED_T3_);
		tosTaskDelete_ui8( idTask );
		tosDelayMs_v( _DELAY_T3_ );
	}
	return 0;
}

void* 		task4( void *arg )
{
	uint8_t dato = 0x39;
	while (1)
	{
		// Board_LED_Set(_EDUCIAA_LED_R_ , 1 );
		tosSemaphoreTake( semaforoBinario );
		uartSendChar( dato );
		Board_LED_Toggle(_LED_T4_);
		tosDelayMs_v( ticksDelay );
		Board_LED_Toggle(_LED_T4_);
		tosDelayMs_v( ticksDelay );
	}
	return 0;
}


int main(void)
{
	initHardware();

	semaforoBinario = tosSemaphoreNewBin();

	// tosAddTask_ui32( &stackTask1[ _DEFAULT_STACK_SIZE_ - 1 ] , task1 , (void *)0x11223344 , _TOS_TASK_PRIORITY_1_);
	tosAddTask_ui32( &stackTask2[ _DEFAULT_STACK_SIZE_ - 1 ] , task2 , (void *)0x11223344 , _TOS_TASK_PRIORITY_2_);
	tosAddTask_ui32( &stackTask3[ _DEFAULT_STACK_SIZE_ - 1 ] , task3 , (void *)0x11223344 , _TOS_TASK_PRIORITY_0_);
	tosAddTask_ui32( &stackTask4[ _DEFAULT_STACK_SIZE_ - 1 ] , task4 , (void *)0x11223344 , _TOS_TASK_PRIORITY_0_);

	tosIniSchedule_v();

	while (1)
	{
		__WFI();
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
