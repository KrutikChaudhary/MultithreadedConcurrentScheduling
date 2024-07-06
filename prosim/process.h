//
// Created by Alex Brodsky on 2023-05-07.
//

#ifndef PROSIM_PROCESS_H
#define PROSIM_PROCESS_H
#include "context.h"

/* Initialize the simulation
 * @params:
 *   quantum: the CPU quantum to use in the situation
 * @returns:
 *   returns 1
 */
extern int process_init(int cpu_quantum);

/* Admit a process into the simulation
 * @params:
 *   proc: pointer to the program context of the process to be admitted
 * @returns:
 *   returns 1
 */
extern int process_admit(context *proc);

/* Perform the simulation
 * @params:
 *   none
 * @returns:
 *   returns 1
 */
extern int process_simulate();

#endif //PROSIM_PROCESS_H
