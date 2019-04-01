/* Copyleft 2019, Leonardo Urrego
 *
 */
	.syntax unified
	.text
	.global PendSV_Handler

	.extern millis
	.extern	getNextSP

	.thumb_func

PendSV_Handler:
	PUSH	{r4-r11,lr}

	MRS		r0,msp		// Carga el argumento para la función
	BL		getNextSP	// Llama a función getNextSP(msp)
	MSR		msp,r0		// mov sp,r0  || mov r13,r0	// R0 = Valor devuelto por getNextSP

	POP		{r4-r11,lr}
return:
	BX lr
