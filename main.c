#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

#include<stdio.h>
#include "main.h"
#include "led.h"

uint8_t current_task = 1; 
uint32_t g_tick_count = 0;

void idle_task(void);
void task1_handler(void);
void task2_handler(void);
void task3_handler(void);
void task4_handler(void);

/* This is a task control block carries private information of each task */
typedef struct 
{
   uint32_t psp_value;
   uint32_t block_count;
   uint8_t current_state;
   void (*task_handler)(void);
} TCB_t;

TCB_t user_tasks[MAX_TASKS];

void enable_processor_faults(void)
{
    uint32_t *pSHCSR = (uint32_t*)0xE000ED24;
    *pSHCSR |= (1 << 16);
    *pSHCSR |= (1 << 17);
    *pSHCSR |= (1 << 18);
}

__attribute__((naked))void init_scheduler_stack(uint32_t sched_top_of_stack)
{
    __asm volatile("MSR MSP,%0"::"r"(sched_top_of_stack):);
    __asm volatile("BX LR");
}

void init_tasks_stack(void) 
{
    user_tasks[0].current_state = TASK_READY_STATE;
    user_tasks[1].current_state = TASK_READY_STATE;
    user_tasks[2].current_state = TASK_READY_STATE;
    user_tasks[3].current_state = TASK_READY_STATE;
    user_tasks[4].current_state = TASK_READY_STATE;

    user_tasks[0].psp_value = IDLE_STACK_START;
    user_tasks[1].psp_value = T1_STACK_START;
    user_tasks[2].psp_value = T2_STACK_START;
    user_tasks[3].psp_value = T3_STACK_START;
    user_tasks[4].psp_value = T4_STACK_START;

    user_tasks[0].task_handler = idle_task;
    user_tasks[1].task_handler = task1_handler;
    user_tasks[2].task_handler = task2_handler;
    user_tasks[3].task_handler = task3_handler;
    user_tasks[4].task_handler = task4_handler;

    // fill in private stacks
    for(int i = 0; i < MAX_TASKS; i++) {
        uint32_t *pPSP = (uint32_t*) user_tasks[i].psp_value;
        pPSP--; // xPSR
        *pPSP = DUMMY_XPSR;
        pPSP--; // PC
        *pPSP = (uint32_t) user_tasks[i].task_handler;
        pPSP--; // LR
        *pPSP = 0xFFFFFFFD;
        // R0 ~ R12
        for(int j = 0; j < 13; j++) {
            pPSP--;
            *pPSP = 0;
        }
        user_tasks[i].psp_value = (uint32_t)pPSP;
    }
}

void init_systick_timer(uint32_t tick_hz)
{
    uint32_t *pSRVR = (uint32_t*)0xE000E014;
    uint32_t *pSCSR = (uint32_t*)0xE000E010;

    uint32_t count_value = (SYSTICK_TIM_CLK / tick_hz) - 1;

    *pSRVR &= ~(0x00FFFFFF); // clear SRVR
    *pSRVR |= count_value;   // load value into SRVR

    *pSCSR |= (1 << 1); // enable SysTick exception request
    *pSCSR |= (1 << 2); // indecate the clock source, processor clock source

    *pSCSR |= (1 << 0); // enable SysTick
}

void save_psp_value(uint32_t current_psp_value)
{
    user_tasks[current_task].psp_value = current_psp_value;
}

uint32_t get_psp_value(void) 
{
    return user_tasks[current_task].psp_value;
}

__attribute__((naked)) void switch_sp_to_psp(void) 
{
    // 1. initialize the PSP with TASK1 stack start address
    __asm volatile ("PUSH {LR}"); // preserve LR of main() 
    __asm volatile ("BL get_psp_value");
    __asm volatile ("MSR PSP, R0"); 
    __asm volatile ("POP {LR}"); // retrieve LR of main()
    // 2. change SP to PSP using CONTROL register
    __asm volatile ("MOV R0,#0x02");
    __asm volatile ("MSR CONTROL,R0");
    __asm volatile ("BX LR");
}

void schedule(void)
{
    // pend the pendSV exception
    uint32_t *pICSR = (uint32_t*)0xE000ED04;
    *pICSR |= (1 << 28);
}

void task_delay(uint32_t tick_count)
{
    INTERRUPT_DISABLE();
    if(current_task) // if NOT idle task
    {
        user_tasks[current_task].block_count   
            = g_tick_count + tick_count;
        user_tasks[current_task].current_state 
            = TASK_BLOCKED_STATE;
        schedule();
    }
    INTERRUPT_ENABLE();
}

void idle_task(void) {
    while(1);
}

void task1_handler(void) {
    while(1) {
        led_on(LED_GREEN);
        task_delay(1000);
        led_off(LED_GREEN);
        task_delay(1000);
    } 
}

void task2_handler(void) {
    while(1) {
        led_on(LED_ORANGE);
        task_delay(500);
        led_off(LED_ORANGE);
        task_delay(500);
    } 
}

void task3_handler(void) {
    while(1) {
        led_on(LED_BLUE);
        task_delay(250);
        led_off(LED_BLUE);
        task_delay(250);
    } 
}

void task4_handler(void) {
    while(1) {
        led_on(LED_RED);
        task_delay(125);
        led_off(LED_RED);
        task_delay(125);
    } 
}

int main(void) 
{
    enable_processor_faults();

    init_scheduler_stack(SCHED_STACK_START);

    init_tasks_stack();

    led_init_all();

    init_systick_timer(TICK_HZ); // 1000Hz 

    switch_sp_to_psp();

    task1_handler();

    for(;;);
}

void SysTick_Handler(void) // this will run every 1ms
{
    g_tick_count++;
    // For each unblock_tasks 
    // if a blocked task has reached tick count
    // then make it ready
    for(int i = 1; i< MAX_TASKS; i++)
      if(user_tasks[i].current_state != TASK_READY_STATE)
        if(user_tasks[i].block_count == g_tick_count)
          user_tasks[i].current_state = TASK_READY_STATE;
    // pendSV
    schedule();
}

void update_next_task(void) // round robin 
{                           // decide next non-blocking task for every 1ms
    int state = TASK_BLOCKED_STATE;
    for(int i = 0; i < MAX_TASKS; i++) {
        current_task = (current_task + 1 != MAX_TASKS)? current_task + 1 : 0;
        state = user_tasks[current_task].current_state;
        if((state == TASK_READY_STATE) && (current_task != 0))
            break; // Ready non-idle task found 
    }
    if(state != TASK_READY_STATE)
        current_task = 0;
}

/* Implementation of context switch */
__attribute__((naked)) void PendSV_Handler(void)
{
    //      preserve LR
    __asm volatile ("PUSH {LR}");

    // 1. Save context of current task
    //      Get current task's PSP value
    __asm volatile ("MRS R0,PSP");
    //      Using that PSP value to save SF2
    __asm volatile ("STMDB R0!,{R4-R11}");
    //      Save the current task's PSP
    __asm volatile ("BL save_psp_value"); // PSP passed by R0 

    // 2. Decide next task to run
    __asm volatile ("BL update_next_task");

    // 3. Retrive context of next task
    //      Get next task's PSP value
    __asm volatile ("BL get_psp_value");
    //      Using that PSP value to retrieve SF2
    __asm volatile ("LDMIA R0!,{R4-R11}");
    //      Update PSP and exit
    __asm volatile ("MSR PSP,R0");

    //      retrieve LR
    __asm volatile ("POP {LR}");
    __asm volatile ("BX LR");
}

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
