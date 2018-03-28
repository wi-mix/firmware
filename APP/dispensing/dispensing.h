/*
 * dispensing.h

 *
 *  Created on: Mar 26, 2018
 *      Author: vanselow
 */
#ifndef DISPENSING_H
#define DISPENSING_H

void startDispenseOrdered(dispensing_ingredient * ingredients);
void start_dispense_simultaneous(dispensing_ingredient * ingredients);
void startDispenseSingle(dispensing_ingredient ingredient, int canister_index);
void PourTask (void *p_arg);

#endif // DISPENSING_H
