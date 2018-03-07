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

#include "http_parser.h"


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

#define APP_TASK_PRIO 4
#define TASK_STACK_SIZE 4096
#define ADC_TASK_PRIO 5

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

CPU_STK AppTaskStartStk[TASK_STACK_SIZE];
CPU_STK ADCTaskStartStk[TASK_STACK_SIZE];


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart              (void        *p_arg);
static  void  ADCTaskStart              (void        *p_arg);


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
int bench(int iter_count, int silent);

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
    uint8_t count = 0;
    for(;;) {
        OSTimeDlyHMSM(0, 0, 10, 000);

    }

}

static  void  ADCTaskStart (void *p_arg) {
	int32_t raw = (alt_read_word(ADC_CH1_BASE));

    for(;;) {
    	BSP_WatchDog_Reset();
    	raw = (0xFFF & alt_read_word(ADC_CH1_BASE));
    	printf("Raw: %d\n", raw);
        OSTimeDlyHMSM(0, 0, 0, 100);
    }

}
