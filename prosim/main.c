#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "context.h"
#include "process.h"

void* thread_function(void* arg) {
    int nodeID = *((int*)arg);
    process_simulate(nodeID);
    return NULL;
}

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

//    for (int i=0; i<NumNodes; i++) {
//        int r = pthread_create(&tid[i],
//                           NULL,
//                           process_simulate(i+1),
//                           NULL);
//    }

    for (int i = 0; i < NumNodes; i++) {
        int* nodeID = malloc(sizeof(int));
        if (nodeID == NULL) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }
        *nodeID = i + 1;  // Assuming nodeID starts from 1

        int rc = pthread_create(&tid[i], NULL, thread_function, nodeID);
    }
    //printf("create over \n");
    for (int i=0; i<NumNodes; i++){
        //printf("join loop %d\n",i);
        int r = pthread_join(tid[i],
                         NULL);
    }

    print_final_stats();
    //printf("join loop over\n");
    /* All the magic happens in here
     */


    /* Output the statistics for processes in order of amdmission.
     */
//    for (int i = 0; i < num_procs; i++) {
//        context_stats(procs[i], stdout);
//    }

    return 0;
}