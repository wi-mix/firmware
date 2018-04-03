#include  <os.h>
#include <stdbool.h>
#include <stdlib.h>
#include "command_controller.h"
#include "../HWLIBS/alt_i2c.h"
#include "../models/models.h"
#include "../dispensing/dispensing.h"
#include "../i2c_driver/i2c_driver.h"
#include "../adc/adc.h"
#include "../tasks.h"

#define CMD_QUEUE_SIZE 1
#define CONTROLLER_STACK_SIZE 4096
#define DISPENSING_STACK_SIZE 4096

CPU_STK Controller_Task_Stack[CONTROLLER_STACK_SIZE];
CPU_STK Dispensing_Task_Stack[DISPENSING_STACK_SIZE];

OS_EVENT* cmd_queue;
void* cmd_msg_queue[CMD_QUEUE_SIZE];

static command_controller cmd_controller;
OS_EVENT * controller_semaphore;


/*
* Processes commands and launches dispensing or level reading depending on outcome
*/
void CommandProcessingTask(void *p_arg)
{
    command_t command;
    command_controller * ctrl = (command_controller *)p_arg;
    while(true)
    {
        //Use I2C to read new command from the PROWF
        read((void *)&command, sizeof(command_t)); 
        command_handler(ctrl, command);
    	//OSTimeDlyHMSM(0, 0, 1, 0);
    }
}

void command_handler(command_controller * controller, command_t command)
{
    recipe* my_recipe = (recipe *)malloc(sizeof(recipe));
    switch(command)
    {
        case READ_LEVELS:
            read_levels();
            break;
        case DISPENSE_REQUEST:
            write((void *)&(controller->state), sizeof(dispensing_status));
            if(controller->state == ACCEPTING)
            {
            	get_recipe(my_recipe);
				dispense(controller, my_recipe);
            }
            break;
        default:
            break;
    }
}

void get_recipe(recipe * my_recipe)
{
    read((void*)my_recipe, sizeof(recipe));
}

void read_levels(void)
{
    //Read levels from the dac into this array
    uint16_t levels[3] = { 0 };
    for(int i = 0; i < 3; i++)
    {
    	levels[i] = HTONS(getCurrentVolume(i));
    }
    write((void *)&(levels[0]), sizeof(levels));
}

void dispense(command_controller * controller, recipe * my_recipe)
{
	if(controller->state == DISPENSING)
	{
		return;
	}
    controller->state = DISPENSING;
    if(controller->current_recipe != NULL)
	{
    	free(controller->current_recipe);
	}

    controller->current_recipe = my_recipe;
	uint8_t os_err;
    os_err = OSTaskCreateExt((void (*)(void *)) DispensingTask,   /* Create the start task.                               */
                                     (void          * ) controller,
                                     (OS_STK        * )&Dispensing_Task_Stack[CONTROLLER_STACK_SIZE - 1],
                                     (INT8U           ) DISPENSE_TASK_PRIO,
                                     (INT16U          ) DISPENSE_TASK_PRIO,  // reuse prio for ID
                                     (OS_STK        * )&Dispensing_Task_Stack[0],
                                     (INT32U          ) CONTROLLER_STACK_SIZE,
                                     (void          * )0,
                                     (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

            if (os_err != OS_ERR_NONE) {
                ; /* Handle error. */
            }
}


void DispensingTask (void *p_arg)
{
	uint8_t os_err;
	OSSemPend(controller_semaphore, 0, &os_err);
    //Dereference controller
    command_controller * ctrl = (command_controller *)p_arg;

    recipe * my_recipe = ctrl->current_recipe;
    ctrl->state = DISPENSING;

    //Dispense liquid
    // If they're ordered, the task will dispense each item in turn
	if(my_recipe->ordered)
	{
		OrderedDispenseTaskInit(my_recipe);
	}// Otherwise spawn all threads simultaneously
	else
	{
		SimultaneousDispenseTaskInit(my_recipe);
	}
    //Update liquid levels

    //Unbusy
	OSSemPend(controller_semaphore, 0, &os_err);
    ctrl->state = ACCEPTING;
    OSSemPost(controller_semaphore);
    OSTaskDel(OS_PRIO_SELF);
}

command_controller * initialize_cmd_ctrl()
{
	uint8_t os_err;
	controller_semaphore = OSSemCreate(1);
	OSSemPend(controller_semaphore, 0, &os_err);
    cmd_controller.state = ACCEPTING;
    cmd_controller.dispense = &dispense;
    cmd_controller.command_handler = &command_handler;
    cmd_controller.current_recipe = NULL;
    cmd_queue = OSQCreate(cmd_msg_queue, CMD_QUEUE_SIZE);
    init_I2C2(&(cmd_controller.command_i2c));

    os_err = OSTaskCreateExt((void (*)(void *)) CommandProcessingTask,   /* Create the start task.                               */
							 (void          * ) &cmd_controller,
							 (OS_STK        * )&Controller_Task_Stack[CONTROLLER_STACK_SIZE - 1],
							 (INT8U           ) CMD_TASK_PRIO,
							 (INT16U          ) CMD_TASK_PRIO,  // reuse prio for ID
							 (OS_STK        * )&Controller_Task_Stack[0],
							 (INT32U          ) CONTROLLER_STACK_SIZE,
							 (void          * )0,
							 (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

	if (os_err != OS_ERR_NONE)
	{
		; /* Handle error. */
	}
	OSSemPost(controller_semaphore);
    return &cmd_controller;
}
