/*
 * dispensing.c

 *
 *  Created on: Mar 26, 2018
 *      Author: vanselow
 */
#include "../models/models.h"
#include "../adc/adc.h"
#include <ucos_ii.h>
#include <cpu.h>
#define TASK_STACK_SIZE 4096
#define POUR_TASK_PRIORITY 4
CPU_STK PourTaskStk[TASK_STACK_SIZE];
OS_EVENT * pour_semaphore;
// For ordered dispensing
void PourTask (void *p_arg);
void InitPourTask(pour_command command){
	INT8U os_err;
	uint8_t task_priority = POUR_TASK_PRIORITY + command.canister_index;

	os_err = OSTaskCreateExt((void (*)(void *)) PourTask,   /* Create the start task.                               */
							 (void          * ) (void *)&command,
							 (OS_STK        * )&PourTaskStk[TASK_STACK_SIZE - 1],
							 (INT8U           ) task_priority,
							 (INT16U          ) task_priority,  // reuse prio for ID
							 (OS_STK        * )&PourTaskStk[0],
							 (INT32U          ) TASK_STACK_SIZE,
							 (void          * )0,
							 (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

	if (os_err != OS_ERR_NONE) {
		printf("Unable to start Motor Task"); /* Handle error. */
	}
}
// For simultaneous dispensing
void startDispenseSingle(dispensing_ingredient ingredient, int canister_index)
{
	pour_command command = {.isOrdered = 0, .target_volume = ingredient.amount, .canister_index = canister_index};
	InitPourTask(command);
}
void startDispenseOrdered(dispensing_ingredient *ingredients)
{
	pour_semaphore = OSSemCreate(0);
	uint8_t err =0;
	for(int i =0; i< 3;i++)
	{
		for(int j=0;i<3;i++){
			if(ingredients[j].order == i){
				// Grab 'pouring' semaphore
				OSSemPend(pour_semaphore,0,&err);
				pour_command pour_command = {.isOrdered=1, .target_volume=ingredients[j].amount, .canister_index=j};
				InitPourTask(pour_command);
				break;
			}
		}
	}

}


void PourTask (void *p_arg) {
	pour_command command = *(pour_command*)p_arg;
	motor_command run_command = {.state=1, .motor_num=command.canister_index};
	motor_command stop_command ={.state=0, .motor_num=command.canister_index};
	MotorTask(run_command);
	double increments[4] = {0.8,0.9,0.95,1};
	for(int i =0;i<sizeof(increments)/sizeof(double);i++)
	{
		while(command.target_volume * increments[i]<=getCurrentVolume(command.canister_index)){
				OSTimeDlyHMSM(0, 0, 0, 100); // Check every 100ms
		}
		MotorTask(stop_command);
		OSTimeDlyHMSM(0, 0, 1, 0); // Wait 1s for levels to stabilize, then pour again
	}

	MotorTask(stop_command);
	if(command.isOrdered == 1){
		OSSemPost(pour_semaphore);
	}
    OSTaskDel(OS_PRIO_SELF);
}



