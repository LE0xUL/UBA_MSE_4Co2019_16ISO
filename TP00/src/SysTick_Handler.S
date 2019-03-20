/* Copyleft 2019, Leonardo Urrego
 *
 */
	.syntax unified
	.text
	.global SysTick_Handler

	.extern	sp1

	.thumb_func

SysTick_Handler:
	push	{r4-r11}
	ldr		r0,=sp1
	ldr		r0,[r0]

	msr		msp,r0		// mov sp,r0  || mov r13,r0

	pop		{r4-r11}
return:
	bx lr
