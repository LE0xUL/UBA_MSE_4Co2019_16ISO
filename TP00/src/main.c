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
#define _DEFAULT_STACK_SIZE_ 	128					// Tamaño de palabra 32 bits
#define _B1_				 	_EDUCIAA_TEC_1_		// Tecla del evento B1
#define _B2_				 	_EDUCIAA_TEC_2_		// Tecla del evento B2
#define _NUM_BTN_				2
#define _DELAY_BTN_				10					// Tiempo de Antirebote

/*==================[internal data declaration]==============================*/

semaphoreHandle_t 	semLED;			// Controla el evento de encendido de LED
semaphoreHandle_t 	semUSB;			// Controla el evento de envio de datos por USB
semaphoreHandle_t 	semBTN;			// Controla el evento de boton

struct
{
	uint8_t 	led2Blink;
	uint8_t 	down1;
	uint8_t 	down2;
	uint8_t 	up1;
	uint8_t 	up2;
	uint32_t	t1;
	uint32_t	t2;
}datoSys;

typedef enum{ _ESPERA1DOWN_ , _ESPERA2DOWN_ , _ESPERA1UP_ , _ESPERA2UP_ } estadosMEF_t;

/** Estados de un Evento de Tecla */
typedef enum
{
	_BTN_STD_UPV_,			// EStado UP Verificado
	_BTN_STD_VDW_,			// EStado para Verificar DOWN
	_BTN_STD_DWV_,			// EStado DOWN Verificado
	_BTN_STD_VUP_			// EStado para Verificar UP
} btnState_e;

typedef struct
{
	uint8_t			tecla;
	btnState_e		state;
	uint32_t		timeEvent;			// En ticks (ms)
} strcBtn_t;

strcBtn_t			btn_arr[ _NUM_BTN_ ];
strcBtn_t			eventoBTN;

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware( void );

/*==================[internal data definition]===============================*/

char*	ledVerdeON_pStn 		= "Led Verde encendido: \r\n";
char*	ledAzulON_pStn			= "Led Azul encendido: \r\n";
char*	ledAmarilloON_pStn		= "Led Amarillo encendido: \r\n";
char*	ledRojoON_pStn			= "Led Rojo encendido: \r\n";
char*	tiempoEncendido_pStn	= "\tTiempo encendido: ";
char*	tiempoFlancosDw_pStn	= "\tTiempo entre flancos desencientes: ";
char*	tiempoFlancosUp_pStn	= "\tTiempo entre flancos ascendentes: ";

/*==================[external data definition]===============================*/

uint32_t	stackTaskLED[ _DEFAULT_STACK_SIZE_ ];	// Tarea que controla en encendido del LED
uint32_t	stackTaskUSB[ _DEFAULT_STACK_SIZE_ ];	// Tarea que controla el envio de datos via UART USB
uint32_t	stackTaskMEF[ _DEFAULT_STACK_SIZE_ ];	// Maquina de estados para el control del estado del sistema
uint32_t	stackTaskBTN[ _DEFAULT_STACK_SIZE_ ];	// Tarea que verifica el estado de los Botones

/*==================[internal functions definition]==========================*/

static void initHardware(void)
{
	Board_Init();
	SystemCoreClockUpdate();
	ciaaUARTInit();
}

void 	uartSendInt(uint32_t x)
{
	char 		num[20];
	int8_t		idxNum = 0;
	uint8_t		dato2Send;
	while(x)
	{
		num[ idxNum++ ] = x%10;
		x /= 10;
	}

	idxNum--;

	for( ; idxNum >= 0 ; idxNum-- )
	{
		dato2Send = num[ idxNum ] + 0x30;
		uartSendChar( dato2Send );
	}
}


/*==================[external functions definition]==========================*/

void* 		taskLED( void *arg )
{
	while (1)
	{
		tosSemaphoreTake( semLED );

		if     ( _B1_ == datoSys.down1 && _B2_ == datoSys.down2 && _B1_ == datoSys.up1 && _B2_ == datoSys.up2 )	datoSys.led2Blink = _EDUCIAA_LED_G_;
		else if( _B1_ == datoSys.down1 && _B2_ == datoSys.down2 && _B2_ == datoSys.up1 && _B1_ == datoSys.up2 )	datoSys.led2Blink = _EDUCIAA_LED_R_;
		else if( _B2_ == datoSys.down1 && _B1_ == datoSys.down2 && _B1_ == datoSys.up1 && _B2_ == datoSys.up2 )	datoSys.led2Blink = _EDUCIAA_LED_1_;	// En mi EDU-CIAA es el amarillo
		else if( _B2_ == datoSys.down1 && _B1_ == datoSys.down2 && _B2_ == datoSys.up1 && _B1_ == datoSys.up2 )	datoSys.led2Blink = _EDUCIAA_LED_B_;

		tosSemaphoreGive( semUSB );

		Board_LED_Toggle( datoSys.led2Blink );
		tosDelayMs_v( datoSys.t1 + datoSys.t2 );
		Board_LED_Toggle( datoSys.led2Blink );
	}
	return 0;
}


void* 		taskUSB( void *arg )
{
	while (1)
	{
		tosSemaphoreTake( semUSB );

		switch ( datoSys.led2Blink )
		{
			case _EDUCIAA_LED_R_ : uartSendString( ledRojoON_pStn 		);	break;
			case _EDUCIAA_LED_G_ : uartSendString( ledVerdeON_pStn		);	break;
			case _EDUCIAA_LED_B_ : uartSendString( ledAzulON_pStn 		);	break;
			case _EDUCIAA_LED_1_ : uartSendString( ledAmarilloON_pStn 	);	break;
		}

		uartSendString( tiempoEncendido_pStn );
		uartSendInt( datoSys.t1 + datoSys.t2 );
		uartSendMS();

		uartSendString( tiempoFlancosDw_pStn );
		uartSendInt( datoSys.t1	);
		uartSendMS();

		uartSendString( tiempoFlancosUp_pStn );
		uartSendInt( datoSys.t2	);
		uartSendMS();
	}
	return 0;
}


void* 		taskBTN( void *arg )
{
	while (1)
	{
		for( uint8_t idxBtn = 0 ; idxBtn < _NUM_BTN_ ; idxBtn++ )
		{
			switch ( btn_arr[ idxBtn ].state )
			{
				case _BTN_STD_UPV_ :
					if( tecXPress( btn_arr[ idxBtn ].tecla ) )
					{
						btn_arr[ idxBtn ].timeEvent = tosGetTicks_ui32();
						btn_arr[ idxBtn ].state 	= _BTN_STD_VDW_;
					}
					break;

				case _BTN_STD_VDW_ :
					if( tecXPress( btn_arr[ idxBtn ].tecla ) )
					{
						btn_arr[ idxBtn ].state 	= _BTN_STD_DWV_;
						eventoBTN = btn_arr[ idxBtn ];
						tosSemaphoreGive( semBTN );
					}
					else
					{
						btn_arr[ idxBtn ].state 	= _BTN_STD_UPV_;
					}
					break;

				case _BTN_STD_DWV_ :
					if( !tecXPress( btn_arr[ idxBtn ].tecla ) )
					{
						btn_arr[ idxBtn ].timeEvent = tosGetTicks_ui32();
						btn_arr[ idxBtn ].state 	= _BTN_STD_VUP_;
					}
					break;

				case _BTN_STD_VUP_ :
					if( !tecXPress( btn_arr[ idxBtn ].tecla ) )
					{
						btn_arr[ idxBtn ].state 	= _BTN_STD_UPV_;
						eventoBTN = btn_arr[ idxBtn ];
						tosSemaphoreGive( semBTN );
					}
					else
					{
						btn_arr[ idxBtn ].state 	= _BTN_STD_DWV_;
					}
					break;
			}
		}
		tosDelayMs_v( _DELAY_BTN_ );
	}
	return 0;
}


void* 		taskMEF( void* arg )
{
	estadosMEF_t estadoMEF = _ESPERA1DOWN_;

	while(TRUE) {
		tosSemaphoreTake( semBTN );
		switch ( estadoMEF )
		{
			case _ESPERA1DOWN_:
				if( _BTN_STD_DWV_ == eventoBTN.state )
				{
					datoSys.down1 	= eventoBTN.tecla;
					datoSys.t1		= eventoBTN.timeEvent;
					estadoMEF 		= _ESPERA2DOWN_;
				}
			break;

			case _ESPERA2DOWN_:
				if( _BTN_STD_DWV_ == eventoBTN.state  &&  datoSys.down1 != eventoBTN.tecla)
				{
					datoSys.down2	= eventoBTN.tecla;
					datoSys.t1		= eventoBTN.timeEvent - datoSys.t1;
					estadoMEF 		= _ESPERA1UP_;
				}
				else
				{
					estadoMEF = _ESPERA1DOWN_;
				}
			break;

			case _ESPERA1UP_:
				if( _BTN_STD_UPV_ == eventoBTN.state )
				{
					datoSys.up1 	= eventoBTN.tecla;
					datoSys.t2		= eventoBTN.timeEvent;
					estadoMEF 		= _ESPERA2UP_;
				}
			break;

			case _ESPERA2UP_:
				if( _BTN_STD_UPV_ == eventoBTN.state  &&  datoSys.up1 != eventoBTN.tecla )
				{
					datoSys.up2 	= eventoBTN.tecla;
					datoSys.t2		= eventoBTN.timeEvent - datoSys.t2;
					estadoMEF = _ESPERA1DOWN_;
					tosSemaphoreGive( semLED );
				}
				else	// Si vuelve a presionar la misma que ya habia soltado, reinicia la medición
				{
					estadoMEF = _ESPERA1UP_;
				}
		}

	}
}


int main(void)
{
	initHardware();

	semLED = tosSemaphoreNewBin();
	semBTN = tosSemaphoreNewBin();
	semUSB = tosSemaphoreNewBin();

	// Ini array botones
	btn_arr[ 0 ].tecla = _B1_;
	btn_arr[ 1 ].tecla = _B2_;

	tosAddTask_ui32( &stackTaskUSB[ _DEFAULT_STACK_SIZE_ - 1 ] , taskUSB , (void *)0x11223344 , _TOS_TASK_PRIORITY_0_);
	tosAddTask_ui32( &stackTaskLED[ _DEFAULT_STACK_SIZE_ - 1 ] , taskLED , (void *)0x11223344 , _TOS_TASK_PRIORITY_0_);
	tosAddTask_ui32( &stackTaskMEF[ _DEFAULT_STACK_SIZE_ - 1 ] , taskMEF , (void *)0x11223344 , _TOS_TASK_PRIORITY_1_);
	tosAddTask_ui32( &stackTaskBTN[ _DEFAULT_STACK_SIZE_ - 1 ] , taskBTN , (void *)0x11223344 , _TOS_TASK_PRIORITY_2_);

	tosIniSchedule_v();

	while (1)
	{
		__WFI();
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
