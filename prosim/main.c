#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
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
    //initialize the finished queue.
    finished_queue_init();

    //initialize threadNodes array
    process_threadNodes_init(NumNodes);

    //initialize the individual threadNodes memory
    for(int i = 1; i<= NumNodes; i++){
        process_init(quantum,i);
    }
    /* We use an array of pointers to contexts to track the processes.
     */
    context **procs  = calloc(num_procs, sizeof(context *));
//
//    process_init(quantum);

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

    pthread_t tid[NumNodes];


    //create
    for (int i = 0; i < NumNodes; i++) {
        int nodeID = i + 1;
        int rc = pthread_create(&tid[i], NULL, process_simulate, nodeID);
    }

    //join
    for (int i=0; i<NumNodes; i++){
        int r = pthread_join(tid[i],
                         NULL);
    }

    //print end stats
    print_final_stats();

    return 0;
}