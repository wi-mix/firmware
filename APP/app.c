/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*                          (c) Copyright 2009-2014; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                          APPLICATION CODE
*
*                                            CYCLONE V SOC
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : JBL
* Modifications	: Nancy Minderman nancy.minderman@ualberta.ca, Brendan Bruner bbruner@ualberta.ca
* 				  Changes to this project include scatter file changes and BSP changes for port from
* 				  Cyclone V dev kit board to DE1-SoC
*********************************************************************************************************
* Note(s)       : none.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <app_cfg.h>
#include  <lib_mem.h>

#include  <bsp.h>
#include  <bsp_int.h>
#include  <bsp_os.h>
#include  <cpu_cache.h>

#include  <cpu.h>
#include  <cpu_core.h>

#include  <os.h>
#include  <hps.h>
#include  <socal.h>
#include  <hwlib.h>

#include "command_controller/command_controller.h"
#include "models/models.h"
#include <alt_i2c.h>

#include "basic_io.h"
#include "adc.h"
#include "seven_seg.h"
#include "pwm.h"
#include "timer.h"


// Compute absolute address of any slave component attached to lightweight bridge
// base is address of component in QSYS window
// This computation only works for slave components attached to the lightweight bridge
// base should be ranged checked from 0x0 - 0x1fffff

#define FPGA_TO_HPS_LW_ADDR(base)  ((void *) (((char *)  (ALT_LWFPGASLVS_ADDR))+ (base)))

#define ADC_TASK_PRIO 		2
#define MOTOR_TASK_PRIO 	3
#define WATCHDOG_TASK_PRIO 	1

#define TASK_STACK_SIZE 4096

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

CPU_STK WatchdogTaskStk[TASK_STACK_SIZE];
CPU_STK ADCTaskStk[TASK_STACK_SIZE];
CPU_STK MotorTaskStk[TASK_STACK_SIZE];


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  WatchdogTask(void *p_arg);
static  void  ADCTask(void *p_arg);
static  void  MotorTask(void *p_arg);

void ADCTaskInit(INT8U task_priority);
void MotorTaskInit(INT8U task_priority);
void WatchdogTaskInit(INT8U task_priority);

void MotorTimerISRHandler(CPU_INT32U cpu_id);

#define ISR_MOTOR PWM1_BASE
#define MOTOR_SPEED PWM_MAX - PWM_INC

bool pouring = false;
bool poured = true;

#define REQUIRED_VOLUME 100
int target_volume = 0;
int tick = 0;
bool paused = false;
#define MAX_TICK 250

command_controller wimix_controller;
/*
*********************************************************************************************************
*                                               main()
*
* Description : Entry point for C code.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : (1) It is assumed that your code will call main() once you have performed all necessary
*                   initialisation.
*********************************************************************************************************
*/

int main ()
{
    BSP_WatchDog_Reset();                                       /* Reset the watchdog as soon as possible.              */

                                                                /* Scatter loading is complete. Now the caches can be activated.*/
    BSP_BranchPredictorEn();                                    /* Enable branch prediction.                            */
    BSP_L2C310Config();                                         /* Configure the L2 cache controller.                   */
    BSP_CachesEn();                                             /* Enable L1 I&D caches + L2 unified cache.             */



    CPU_Init();

    Mem_Init();

    BSP_Init();

    ADCInit();

    OSInit();

     InitHPSTimerInterrupt(200000, MotorTimerISRHandler);



    ADCTaskInit(ADC_TASK_PRIO);
    // MotorTaskInit(MOTOR_TASK_PRIO);


    WatchdogTaskInit(WATCHDOG_TASK_PRIO);

    CPU_IntEn();

    OSStart();

}


static void WatchdogTaskInit(INT8U task_priority){
    INT8U os_err;

    os_err = OSTaskCreateExt((void (*)(void *)) WatchdogTask,   /* Create the start task.                               */
                             (void          * ) 0,
                             (OS_STK        * )&WatchdogTaskStk[TASK_STACK_SIZE - 1],
                             (INT8U           ) task_priority,
                             (INT16U          ) task_priority,  // reuse prio for ID
                             (OS_STK        * )&WatchdogTaskStk[0],
                             (INT32U          ) TASK_STACK_SIZE,
                             (void          * )0,
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if (os_err != OS_ERR_NONE) {
        ; /* Handle error. */
    }
}
/*
*********************************************************************************************************
*                                           App_TaskStart()
*
* Description : Startup task example code.
*
* Arguments   : p_arg       Argument passed by 'OSTaskCreate()'.
*
* Returns     : none.
*
* Created by  : main().
*
* Notes       : (1) The ticker MUST be initialised AFTER multitasking has started.
*********************************************************************************************************
*/
static  void  WatchdogTask (void *p_arg) {

    BSP_OS_TmrTickInit(OS_TICKS_PER_SEC);                       /* Configure and enable OS tick interrupt.              */

    for(;;) {
        BSP_WatchDog_Reset();                                   /* Reset the watchdog.                                  */

        OSTimeDlyHMSM(0, 0, 0, 500);
        BSP_LED_On();

        OSTimeDlyHMSM(0, 0, 0, 500);
        BSP_LED_Off();
    }

}

void ADCTaskInit(INT8U task_priority){
    INT8U os_err;

    os_err = OSTaskCreateExt((void (*)(void *)) ADCTask,   /* Create the start task.                               */
                             (void          * ) 0,
                             (OS_STK        * )&ADCTaskStk[TASK_STACK_SIZE - 1],
                             (INT8U           ) task_priority,
                             (INT16U          ) task_priority,  // reuse prio for ID
                             (OS_STK        * )&ADCTaskStk[0],
                             (INT32U          ) TASK_STACK_SIZE,
                             (void          * )0,
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if (os_err != OS_ERR_NONE) {
        printf("Unable to start ADC Task"); /* Handle error. */
    }
}

static void ADCTask(void *p_arg) {
    for(;;) {
    	uint32_t cmd = 0x3FF & alt_read_word(SW_BASE);
    	uint32_t chan_num = 0x0FF & cmd;
    	int32_t* channel = 0;
    	channel = getADCChannel(chan_num);
    	if((cmd & 0x300) == 0){
			int32_t raw = (0xFFF & alt_read_word(channel));
			printf("Channel %u, Raw: %d, Cur: %d, Target: %d\n", chan_num, raw, calculateVolume(raw, 3), target_volume);
    	}

        OSTimeDlyHMSM(0, 0, 0, 100);
    }

}

void MotorTaskInit(INT8U task_priority){
    INT8U os_err;

    os_err = OSTaskCreateExt((void (*)(void *)) MotorTask,   /* Create the start task.                               */
                             (void          * ) 0,
                             (OS_STK        * )&MotorTaskStk[TASK_STACK_SIZE - 1],
                             (INT8U           ) task_priority,
                             (INT16U          ) task_priority,  // reuse prio for ID
                             (OS_STK        * )&MotorTaskStk[0],
                             (INT32U          ) TASK_STACK_SIZE,
                             (void          * )0,
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if (os_err != OS_ERR_NONE) {
        printf("Unable to start Motor Task"); /* Handle error. */
    }
}

static void  MotorTask(void *p_arg) {
    for(;;) {
    	uint32_t cmd = 0x3FF & alt_read_word(SW_BASE);
    	uint32_t motor_num = (cmd & 0x300) >> 8;
    	void* motor = PWM1_BASE;
    	uint32_t increment = 0x0FF & cmd;
    	if(motor_num != 0){
    		switch(motor_num){
				case 1: motor = PWM1_BASE; break;
				case 2: motor = PWM2_BASE; break;
				case 3: motor = PWM3_BASE; break;
				case 4: motor = PWM4_BASE; break;
				default: motor = PWM1_BASE; break;
    		}
    		alt_write_word(motor, increment * PWM_INC);
    		printf("PWM%d: %d/%d\n", motor_num, increment * PWM_INC, PWM_MAX);
    	}
        OSTimeDlyHMSM(0, 0, 1, 0);
    }
}

void MotorTimerISRHandler(CPU_INT32U cpu_id) {
	// Do stuff
	if(pouring == true){
		int32_t volume = getCurrentVolume(2);
		if(paused){
    		alt_write_word(ISR_MOTOR, 0);
		} else {
    		alt_write_word(ISR_MOTOR, MOTOR_SPEED);
		}
		if(tick < MAX_TICK){
			tick++;
		} else {
			tick = 0;
			paused = !paused;
		}
		if(volume <= target_volume){
			pouring = false;
    		alt_write_word(ISR_MOTOR, 0);
		}
	} else {
		sw_read();
		int8_t toggle = sw_get_bit_val(9);
		if(toggle == 1 && poured == false){
			target_volume = getCurrentVolume(2) - REQUIRED_VOLUME;
			pouring = true;
			poured = true;
			paused = false;
			tick = 0;
		} else if(toggle == 0 && poured == true){
			poured = false;
		}
	}

	// READ EOI Reg to clear interrupt (PAGE 23-10/23-11 of Cyclone V Hard Processor System
	// Technical Reference Manual
	volatile int32_t status = ARM_OSCL_TIMER_0_REG_EOI;
}
