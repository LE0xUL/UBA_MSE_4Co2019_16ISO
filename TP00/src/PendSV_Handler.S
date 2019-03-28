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

	LDR  R1, =millis	// read address of millis into R1
	LDR  R0, [R1]		// load value of millis
	ADD  R0, R0, #1		// millis++
	STR  R0, [R1]		// store new value into millis

	MRS		r0,msp		// Carga el argumento para la función
	BL		getNextSP	// Llama a función getNextSP(msp)
	MSR		msp,r0		// mov sp,r0  || mov r13,r0	// R0 = Valor devuelto por getNextSP

	POP		{r4-r11,lr}
return:
	BX lr
