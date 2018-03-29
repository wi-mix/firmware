/*
 * dispensing.c

 *
 *  Created on: Mar 26, 2018
 *      Author: vanselow
 */
#include "../models/models.h"
#include "../pwm/pwm.h"
#include "../adc/adc.h"
#include "../tasks.h"

#include <ucos_ii.h>
#include <cpu.h>
#define TASK_STACK_SIZE 4096

#define POUR_TASK_NUM 3
CPU_STK PourTaskStk[POUR_TASK_NUM][TASK_STACK_SIZE];
OS_EVENT * pour_semaphore;
OS_EVENT * simultaneous_pour_semaphore;
OS_EVENT * simultaneous_done_semaphore;
extern OS_EVENT * controller_semaphore;
// For ordered dispensing
void PourTask (void *p_arg);

void InitPourTask(pour_command command){
	INT8U os_err;
	uint8_t task_priority = POUR_TASK_PRIORITY + command.canister_index;
	printf("Initializing pouring task for canister: %d\r\n", command.canister_index);
	os_err = OSTaskCreateExt((void (*)(void *)) PourTask,   /* Create the start task.                               */
							 (void          * ) (void *)&command,
							 (OS_STK        * )&PourTaskStk[command.canister_index][TASK_STACK_SIZE - 1],
							 (INT8U           ) task_priority,
							 (INT16U          ) task_priority,  // reuse prio for ID
							 (OS_STK        * )&PourTaskStk[0],
							 (INT32U          ) TASK_STACK_SIZE,
							 (void          * )0,
							 (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

	if (os_err != OS_ERR_NONE)
	{
		printf("Unable to start Motor Task"); /* Handle error. */
	}
}
// For simultaneous dispensing
void start_dispense_simultaneous(dispensing_ingredient * ingredients)
{
	uint8_t err = 0;
	simultaneous_pour_semaphore = OSSemCreate(3);
	simultaneous_done_semaphore = OSSemCreate(0);
	for(int i = 0; i < 3; i++)
	{
		pour_command command = {.isOrdered = 0,
								.target_volume = getTargetVolume(i, ingredients[i].amount),
								.canister_index = i};
		InitPourTask(command);
	}
	OSSemPend(simultaneous_done_semaphore, 0, &err);
	OSSemDel(simultaneous_pour_semaphore, OS_DEL_ALWAYS, &err);
	OSSemDel(simultaneous_done_semaphore, OS_DEL_ALWAYS, &err);
	OSSemPost(controller_semaphore);
}

void startDispenseOrdered(dispensing_ingredient *ingredients)
{
	pour_semaphore = OSSemCreate(1);
	uint8_t err = 0;
	for(int i = 0; i < 3; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			if(ingredients[j].order == i)
			{
				// Grab 'pouring' semaphore
				OSSemPend(pour_semaphore,0,&err);
				pour_command pour_command = {.isOrdered = 1,
											 .target_volume = getTargetVolume(j, ingredients[j].amount),
											 .canister_index = j};
				InitPourTask(pour_command);
				break;
			}
		}
	}
	OSSemPend(pour_semaphore,0,&err);
	OSSemDel(pour_semaphore, OS_DEL_ALWAYS, &err);
	OSSemPost(controller_semaphore);
}


void PourTask (void *p_arg) {
	uint8_t err = 0;
	pour_command command = *(pour_command*)p_arg;

	motor_command run_command = {.state=RUN, .motor_num=command.canister_index};
	motor_command stop_command ={.state=STOP, .motor_num=command.canister_index};

	if(!command.isOrdered)
	{
		OSSemPend(simultaneous_pour_semaphore, 0, &err);
	}

	double increments[3] = {.1,0.05,0};

	printf("The target volume for canister %d is %d\r\n", command.canister_index, command.target_volume);
	int amount = getCurrentVolume(command.canister_index) - command.target_volume;
	for(int i = 0; i < sizeof(increments)/sizeof(double); i++)
	{

		MotorTask(run_command);

		while(command.target_volume <= getCurrentVolume(command.canister_index) - (amount * increments[i]))
		{
			printf("The current volume for canister %d is %d\r\n", command.canister_index, getCurrentVolume(command.canister_index));

			OSTimeDlyHMSM(0, 0, 0, 100); // Check every 100ms
		}
		MotorTask(stop_command);
		OSTimeDlyHMSM(0, 0, 0, 250); // Wait 1s for levels to stabilize, then pour again
	}
	printf("Finished pouring canister %d\r\n", command.canister_index);
	MotorTask(stop_command);
	if(command.isOrdered)
	{
		OSSemPost(pour_semaphore);
	}
	else
	{
		OS_SEM_DATA data;
		OSSemPost(simultaneous_pour_semaphore);
		OSSemQuery(simultaneous_pour_semaphore, &data);
		if(data.OSCnt == 3)
		{
			OSSemPost(simultaneous_done_semaphore);
		}
	}
    OSTaskDel(OS_PRIO_SELF);
}



