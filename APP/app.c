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


// Compute absolute address of any slave component attached to lightweight bridge
// base is address of component in QSYS window
// This computation only works for slave components attached to the lightweight bridge
// base should be ranged checked from 0x0 - 0x1fffff

#define FPGA_TO_HPS_LW_ADDR(base)  ((void *) (((char *)  (ALT_LWFPGASLVS_ADDR))+ (base)))

#define LEDR_ADD 0x00000000
#define LEDR_BASE FPGA_TO_HPS_LW_ADDR(LEDR_ADD)

#define SW_ADD 0x00001100
#define SW_BASE FPGA_TO_HPS_LW_ADDR(SW_ADD)

// ADC Read Addresses
#define ADC_CH0 0x00001200
#define ADC_CH0_BASE FPGA_TO_HPS_LW_ADDR(ADC_CH0)
#define ADC_CH1 0x00001204
#define ADC_CH1_BASE FPGA_TO_HPS_LW_ADDR(ADC_CH1)
#define ADC_CH2 0x00001208
#define ADC_CH2_BASE FPGA_TO_HPS_LW_ADDR(ADC_CH2)
#define ADC_CH3 0x0000120C
#define ADC_CH3_BASE FPGA_TO_HPS_LW_ADDR(ADC_CH3)
#define ADC_CH4 0x00001210
#define ADC_CH4_BASE FPGA_TO_HPS_LW_ADDR(ADC_CH4)
#define ADC_CH5 0x00001214
#define ADC_CH5_BASE FPGA_TO_HPS_LW_ADDR(ADC_CH5)
#define ADC_CH6 0x00001218
#define ADC_CH6_BASE FPGA_TO_HPS_LW_ADDR(ADC_CH6)
#define ADC_CH7 0x0000121C
#define ADC_CH7_BASE FPGA_TO_HPS_LW_ADDR(ADC_CH7)
// ADC Write Addresses
#define ADC_UP 0x00001200
#define ADC_UP_BASE FPGA_TO_HPS_LW_ADDR(ADC_UP)
#define ADC_AUTO 0x00001204
#define ADC_AUTO_BASE FPGA_TO_HPS_LW_ADDR(ADC_AUTO)

// Hex Locations
#define HEX0_ADD 0x00000100
#define HEX0_BASE FPGA_TO_HPS_LW_ADDR(HEX0_ADD)
#define HEX1_ADD 0x00000110
#define HEX1_BASE FPGA_TO_HPS_LW_ADDR(HEX1_ADD)
#define HEX2_ADD 0x00000120
#define HEX2_BASE FPGA_TO_HPS_LW_ADDR(HEX2_ADD)
#define HEX3_ADD 0x00000130
#define HEX3_BASE FPGA_TO_HPS_LW_ADDR(HEX3_ADD)
#define HEX4_ADD 0x00000140
#define HEX4_BASE FPGA_TO_HPS_LW_ADDR(HEX4_ADD)
#define HEX5_ADD 0x00000150
#define HEX5_BASE FPGA_TO_HPS_LW_ADDR(HEX5_ADD)

// PWM Locations
#define PWM1_ADD 0x00000200
#define PWM1_BASE FPGA_TO_HPS_LW_ADDR(PWM1_ADD)
#define PWM2_ADD 0x00000204
#define PWM2_BASE FPGA_TO_HPS_LW_ADDR(PWM2_ADD)
#define PWM3_ADD 0x00000208
#define PWM3_BASE FPGA_TO_HPS_LW_ADDR(PWM3_ADD)
#define PWM4_ADD 0x0000020C
#define PWM4_BASE FPGA_TO_HPS_LW_ADDR(PWM4_ADD)

// PWM Constants
#define PWM_MAX 625000
#define PWM_INC   6250

#define APP_TASK_PRIO 	0
#define ADC_TASK_PRIO 	1
#define MOTOR_TASK_PRIO 2

#define TASK_STACK_SIZE 4096

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

CPU_STK AppTaskStartStk[TASK_STACK_SIZE];
CPU_STK ADCTaskStartStk[TASK_STACK_SIZE];
CPU_STK MotorTaskStartStk[TASK_STACK_SIZE];


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart              (void        *p_arg);
static  void  ADCTaskStart              (void        *p_arg);
static  void  MotorTaskStart            (void        *p_arg);


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
    INT8U os_err;

    BSP_WatchDog_Reset();                                       /* Reset the watchdog as soon as possible.              */

                                                                /* Scatter loading is complete. Now the caches can be activated.*/
    BSP_BranchPredictorEn();                                    /* Enable branch prediction.                            */
    BSP_L2C310Config();                                         /* Configure the L2 cache controller.                   */
    BSP_CachesEn();                                             /* Enable L1 I&D caches + L2 unified cache.             */


    alt_write_word(ADC_AUTO_BASE, 1);

    CPU_Init();

    Mem_Init();

    BSP_Init();


    OSInit();




    os_err = OSTaskCreateExt((void (*)(void *)) AppTaskStart,   /* Create the start task.                               */
                             (void          * ) 0,
                             (OS_STK        * )&AppTaskStartStk[TASK_STACK_SIZE - 1],
                             (INT8U           ) APP_TASK_PRIO,
                             (INT16U          ) APP_TASK_PRIO,  // reuse prio for ID
                             (OS_STK        * )&AppTaskStartStk[0],
                             (INT32U          ) TASK_STACK_SIZE,
                             (void          * )0,
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if (os_err != OS_ERR_NONE) {
        ; /* Handle error. */
    }

    os_err = OSTaskCreateExt((void (*)(void *)) ADCTaskStart,   /* Create the start task.                               */
                             (void          * ) 0,
                             (OS_STK        * )&ADCTaskStartStk[TASK_STACK_SIZE - 1],
                             (INT8U           ) ADC_TASK_PRIO,
                             (INT16U          ) ADC_TASK_PRIO,  // reuse prio for ID
                             (OS_STK        * )&ADCTaskStartStk[0],
                             (INT32U          ) TASK_STACK_SIZE,
                             (void          * )0,
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if (os_err != OS_ERR_NONE) {
        ; /* Handle error. */
    }

    os_err = OSTaskCreateExt((void (*)(void *)) MotorTaskStart,   /* Create the start task.                               */
                             (void          * ) 0,
                             (OS_STK        * )&MotorTaskStartStk[TASK_STACK_SIZE - 1],
                             (INT8U           ) MOTOR_TASK_PRIO,
                             (INT16U          ) MOTOR_TASK_PRIO,  // reuse prio for ID
                             (OS_STK        * )&MotorTaskStartStk[0],
                             (INT32U          ) TASK_STACK_SIZE,
                             (void          * )0,
                             (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

    if (os_err != OS_ERR_NONE) {
        ; /* Handle error. */
    }

    CPU_IntEn();

    OSStart();

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

void display7Seg(uint8_t top, uint16_t bottom){
    alt_write_word(HEX0_BASE, bottom);
    alt_write_word(HEX1_BASE, bottom>>4);
    alt_write_word(HEX2_BASE, bottom>>8);
    alt_write_word(HEX3_BASE, bottom>>12);
    alt_write_word(HEX4_BASE, top);
    alt_write_word(HEX5_BASE, top>>4);
}

int32_t min = 0;
int32_t max = 0;
float minf = 0;
float maxf = 0;

static  void  AppTaskStart (void *p_arg) {

    BSP_OS_TmrTickInit(OS_TICKS_PER_SEC);                       /* Configure and enable OS tick interrupt.              */

    for(;;) {
        BSP_WatchDog_Reset();                                   /* Reset the watchdog.                                  */

        OSTimeDlyHMSM(0, 0, 0, 500);
        BSP_LED_On();

        OSTimeDlyHMSM(0, 0, 0, 500);
        BSP_LED_Off();
    }

}

static  void  ADCTaskStart (void *p_arg) {
    for(;;) {
    	uint32_t cmd = 0x3FF & alt_read_word(SW_BASE);
    	uint32_t chan_num = 0x0FF & cmd;
    	void* channel = 0;

    	switch(chan_num) {
			case 0: channel = ADC_CH0_BASE; break;
			case 1: channel = ADC_CH1_BASE; break;
			case 2: channel = ADC_CH2_BASE; break;
			case 3: channel = ADC_CH3_BASE; break;
			case 4: channel = ADC_CH4_BASE; break;
			case 5: channel = ADC_CH5_BASE; break;
			case 6: channel = ADC_CH6_BASE; break;
			case 7: channel = ADC_CH7_BASE; break;
			default: channel = ADC_CH0_BASE; break;
    	}
    	if((cmd & 0x300) == 0){
			int32_t raw = (0xFFF & alt_read_word(channel));
			printf("Channel %u: %d\n", chan_num, raw);
    	}
        OSTimeDlyHMSM(0, 0, 0, 100);
    }

}

static void  MotorTaskStart (void *p_arg) {
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
