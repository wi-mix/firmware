/*
 * dispensing.h

 *
 *  Created on: Mar 26, 2018
 *      Author: vanselow
 */
#ifndef DISPENSING_H
#define DISPENSING_H

void OrderedDispenseTaskInit(recipe * dispensing_recipe);
void SimultaneousDispenseTaskInit(recipe * dispensing_recipe);
void SimultaneousDispenseTask (void *p_arg);
void OrderedDispenseTask (void *p_arg);

#endif // DISPENSING_H
