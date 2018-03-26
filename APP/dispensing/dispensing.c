/*
 * dispensing.c

 *
 *  Created on: Mar 26, 2018
 *      Author: vanselow
 */
#include "../models/models.h"
#include "../adc/adc.h"
#include <ucos_ii.h>
#include "../app.c"
#include <cpu.h>
#define TASK_STACK_SIZE 4096
#define POUR_TASK_PRIORITY 4
CPU_STK PourTaskStk[TASK_STACK_SIZE];
OS_EVENT * pour_semaphore;
// For ordered dispensing
void PourTask (void *p_arg);
void InitPourTask(int canister_index, int32_t target_volume, char isOrdered){
	INT8U os_err;
	uint8_t task_priority = POUR_TASK_PRIORITY + canister_index;
	pour_command pour_command = {.isOrdered=isOrdered, .target_volume=target_volume, .canister_index=canister_index};
	os_err = OSTaskCreateExt((void (*)(void *)) PourTask,   /* Create the start task.                               */
							 (void          * ) (void *)&pour_command,
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
	int32_t target_volume = 0;
	char isOrdered = 0;
	InitPourTask(canister_index,target_volume,isOrdered);
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
				char isOrdered =1;
				InitPourTask(j,target_volume,isOrdered);
				break;
			}
		}
	}

}


void PourTask (void *p_arg) {
	pour_command command = *(pour_command*)p_arg;
	while(command.target_volume<getCurrentVolume(command.canister_index)){
		motor_command motor_command = {.state=1, .motor_num=command.canister_index};
		MotorTask(motor_command);
	}
	if(command.isOrdered == 1){
		OSSemPost(pour_semaphore);
	}
}



