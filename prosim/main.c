#include <stdio.h>
#include <stdlib.h>
#include "context.h"
#include "process.h"

int main() {
    int num_procs;
    int quantum;
    int NumNodes;

    /* Read in the header of the process description with minimal validation
     */
    if (scanf("%d %d %d", &num_procs, &quantum, &NumNodes) < 2) {
        fprintf(stderr, "Bad input, expecting number of process and quantum size\n");
        return -1;
    }

    /* We use an array of pointers to contexts to track the processes.
     */
    context **procs  = calloc(num_procs, sizeof(context *));

    process_init(quantum);

    /* Load and admit each process, if an error occurs, we just give up.
     */
    for (int i = 0; i < num_procs; i++) {
        procs[i] = context_load(stdin);
        if (!procs[i]) {
            fprintf(stderr, "Bad input, could not load program description\n");
            return -1;
        }
        process_admit(procs[i]);
    }

    /* All the magic happens in here
     */
    process_simulate();

    /* Output the statistics for processes in order of amdmission.
     */
    for (int i = 0; i < num_procs; i++) {
        context_stats(procs[i], stdout);
    }

    return 0;
}