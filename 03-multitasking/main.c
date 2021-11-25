#include <stdint.h>

#define MAX_TASKS 4
#define HSI_CLOCK         16000000U
#define SYSTICK_TIM_CLOCK HSI_CLOCK
#define TICK_HZ 1000
uint32_t task_handlers[MAX_TASKS];
uint32_t psp_of_tasks[MAX_TASKS] = {T1_STACK_START,
                                    T2_STACK_START,
									T3_STACK_START,
									T4_STACK_START};
uint8_t current_task = 1; //task1 is first task

void enable_processor_faults (void)
{
	// Enable all configurable exceptions : usage fault, mem manage fault and bus fault

	uint32_t *pSHCSR = (uint32_t*)0xE000ED24;

	*pSHCSR |= ( 1 << 16); //mem manage
	*pSHCSR |= ( 1 << 17); //bus fault
	*pSHCSR |= ( 1 << 18); //usage fault

}

void init_tasks_stack(void)
{
	uint32_t *pPSP;
	for(int i = 0; i < MAX_TASKS; i++) // Create dummy stack frame
	{
		pPSP = (uint32_t*) psp_of_tasks[i];
		pPSP--; // xPSR
		*pPSP = 0x01000000;

		pPSP--; // PC
		*pPSP = task_handler[i];

		pPSP--; // LR
		*pPSP = 0xFFFFFFFD;

		// R0 ~ R12 init as 0
		for(int j = 0; j < 13; j++) {
			pPSP--;
			*pPSP = 0;
		}

		// Preserve value of each PSP
		psp_of_tasks[i] = (uint_32_t)pPSP;
	}
}


void init_systick_timer(uint32_t tick_hz)
{
	uint32_t *pSRVR = (uint32_t*)0xE00E014;
	uint32_t *pSCSR = (uint32_t*)0xE00E010;

	uint32_t count_value = (SYSTICK_TIM_CLOCK / tich_hz) - 1;

	// Clear the value of SRVR
	*pSRVR &= ~(0x00FFFFFF);
	// Load the value into SRVR
	*pSRVR |= count_value;

	// do some settings
	*pSCSR |= (1 << 1); // Enable SysTick exception requeset
	*pSCSR |= (1 << 2); // Use processor clock resourc
	*pSCSR |= (1 << 0); // Enbale the counter
}

uint32_t get_psp_value(void)
{
    return psp_of_tasks[current_task]; // returnd by R0
}

__attribute__((naked)) switch_sp_to_psp(void)
{
    // 1. initialize the PSP the with Task1 stack start address
    //    get the value of psp of current task
    __asm volatile("PUSH {LR}"); // 下個 function call 會覆蓋掉 switch_sp_to_psp 回到 main 的 LR, 所以先存下來
    __asm volatile("BL get_psp_value"); // branch and link (return)
    __asm volatile("MSR PSP,R0"); // initialize PSP
    __asm volatile("POP {LR}"); // 拿回 switch_sp_to_psp 的 LR
    // 2. change SP to PSP using CONTROL
    __asm volatile("MOV R0,#0x02");
    __asm volatile("MSR CONTROL,R0");
    __asm volatile("BX LR"); // 跳回 main, 也就是 LR 會被放到 PC
}

void task1_handler(void)
{
    for(;;);
}

void task2_handler(void)
{
    for(;;);
}

void task3_handler(void)
{
    for(;;);
}

void task4_handler(void)
{
    for(;;);
}

int main(void)
{
    enable_processor_faults();

    int_scheduler_stack(SCHED_STACK_START);

    task_handler[0] = (uint32_t)task1_handler;
    task_handler[1] = (uint32_t)task2_handler;
    task_handler[2] = (uint32_t)task3_handler;
    task_handler[3] = (uint32_t)task4_handler;

    init_tasks_stack();

    init_systick_timer(TICK_HZ);

    switch_sp_to_psp();

    task1_handler();

	for(;;);
}

// Implement the fault handlers
void HardFault_Handler(void)
{
	while(1);
}


void MemManage_Handler(void)
{
	while(1);
}

void BusFault_Handler(void)
{
	while(1);
}
