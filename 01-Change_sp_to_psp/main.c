#include <stdint.h>

int func_add (int a, int b, int c, int d)
{
	return a + b + c + d;
}

__attribute__((naked)) void change_sp_to_psp(void)
{
	// Macro of PSP_START
	__asm volatile(".equ SRAM_END, (0x20000000 + (128 * 1024))");
	__asm volatile(".equ PSP_START, (SRAM_END - 152)");
	// Write PSP start to PSP reg
	__asm volatile("LDR R0,=PSP_START");
	__asm volatile("MSR PSP, R0");
	// Set CONTROL[1] to 1 to use PSP as current stack in Thread mode
	__asm volatile("MOV R0,#0x02");
	__asm volatile("MSR CONTROL,R0");  // [GDB]: `PSP` is copied to `SP`
	// Jump to `LR` (return address)
	__asm volatile("BX LR");           // [GDB]: Jump back to main()

}

void generate_exception(void)
{
	__asm volatile("SVC #0x2");        // [GDB]: Jump to SVC_Handler
}

int main(void)
{
	change_sp_to_psp();
	int ret;
	ret = func_add(1,2,3,4);
	/* printf("result = %d\n", ret); */
	generate_exception();
	for(;;);
}

void SVC_Handler(void)
{
	/* printf("In SVC Handler\n"); */
    for(;;);
}
