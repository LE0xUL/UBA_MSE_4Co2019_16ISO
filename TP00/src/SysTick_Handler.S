/* Copyleft 2019, Leonardo Urrego
 *
 */
	.syntax unified
	.text
	.global SysTick_Handler

	.extern	getNextSP

	.thumb_func

SysTick_Handler:
	push	{r4-r11,lr}

	mrs		r0,msp		// Carga el argumento para la función
	bl		getNextSP	// Llama a función getNextSP(msp)
	msr		msp,r0		// mov sp,r0  || mov r13,r0	// R0 = Valor devuelto por getNextSP

	pop		{r4-r11,lr}
return:
	bx lr
