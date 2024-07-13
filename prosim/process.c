//
// Created by Alex Brodsky on 2023-05-07.
//

#include <malloc.h>
#include "process.h"
#include "prio_q.h"
#include <pthread.h>
//need separate queue for all thread
//need to store proc for each thread
//need to figure out how to simulate each thread
//need separate quantum and time for all of them
static prio_q_t *finished;
typedef struct threadNode{
    int n; //curr size;
    context **procs;
    int quantum;
    int time;
    prio_q_t *blocked;
    prio_q_t *ready;
    int next_proc_id;
} threadNode;
static threadNode *nodes;

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//static int time = 0;
//static ;

enum {
    PROC_NEW = 0,
    PROC_READY,
    PROC_RUNNING,
    PROC_BLOCKED,
    PROC_FINISHED
};

static char *states[] = {"new", "ready", "running", "blocked", "finished"};

//static int quantum;

/* Initialize the simulation
 * @params:
 *   quantum: the CPU quantum to use in the situation
 * @returns:
 *   returns 1
 */
extern void finished_queue_init(){
    finished = prio_q_new();
}
//function to initialize the array of threadNodes
extern int process_threadNodes_init(int numNodes){
    nodes = (threadNode *)calloc(numNodes + 1, sizeof(threadNode));
    if(nodes){
        return 1;
    } else {
        return 0;
    }
}


extern int process_init(int cpu_quantum, int nodeId) {
    /* Set up the queues and store the quantum
     * Assume the queues will be allocated
     */
    nodes[nodeId].n=0;
    nodes[nodeId].procs= calloc(100, sizeof(context *));
    nodes[nodeId].quantum = cpu_quantum;
    nodes[nodeId].time = 0;
    nodes[nodeId].blocked = prio_q_new();
    nodes[nodeId].ready = prio_q_new();
    nodes[nodeId].next_proc_id = 1;
    return 1;
}

/* Print state of process
 * @params:
 *   proc: process' context
 * @returns:
 *   none
 */ //AL

extern void print_final_stats(){
    while(!prio_q_empty(finished)){
        context *cur = prio_q_remove(finished);
        printf("| %5.5d | Proc %02d.%02d | Run %d, Block %d, Wait %d\n",nodes[cur->node].time, cur->node, cur->id, cur->doop_time,cur->block_time, cur->wait_time);
    }
}

static void print_process(context *proc, int time) {
    //pthread_mutex_lock(&lock);
    printf("[%02d] %5.5d: process %d %s\n" ,proc->node, time, proc->id, states[proc->state]);
    //pthread_mutex_lock(&lock);

}

/* Compute priority of process, depending on whether SJF or priority based scheduling is used
 * @params:
 *   proc: process' context
 * @returns:
 *   priority of process
 */
static int actual_priority(context *proc) {
    if (proc->priority < 0) {
        /* SJF means duration of current DOOP is the priority
         */
        return proc->duration;
    }
    return proc->priority;
}

/* Insert process into appropriate queue based on the primitive it is performing
 * @params:
 *   proc: process' context
 *   next_op: if true, current primitive is done, so move IP to next primitive.
 * @returns:
 *   none
 */
static void insert_in_queue(context *proc, int next_op) { //***//
    /* If current primitive is done, move to next
     */
    if (next_op) {
        context_next_op(proc);
        proc->duration = context_cur_duration(proc);
    }

    int op = context_cur_op(proc);

    /* 3 cases:
     * 1. If DOOP, process goes into ready queue
     * 2. If BLOCK, process goes into blocked queue
     * 3. If HALT, process is not queued
     */
    if (op == OP_DOOP) {
        proc->state = PROC_READY;
        prio_q_add(nodes[proc->node].ready, proc, actual_priority(proc));
        proc->wait_count++;
        proc->enqueue_time = nodes[proc->node].time;
    } else if (op == OP_BLOCK) {
        /* Use the duration field of the process to store their wake-up time.
         */
        proc->state = PROC_BLOCKED;
        proc->duration += nodes[proc->node].time;
        prio_q_add(nodes[proc->node].blocked, proc, proc->duration);
    } else {
        proc->state = PROC_FINISHED;
        pthread_mutex_lock(&lock);
        prio_q_add(finished, proc, nodes[proc->node].time*100*100 + proc->node*100 + proc->id);
        pthread_mutex_unlock(&lock);
    }
    //printf("cajnscjac");
    print_process(proc,nodes[proc->node].time);
}

/* Admit a process into the simulation
 * @params:
 *   proc: pointer to the program context of the process to be admitted
 * @returns:
 *   returns 1
 */
extern int process_admit(context *proc) {
    /* Use a static variable to assign each process a unique process id.
     */
    proc->id = nodes[proc->node].next_proc_id;
    nodes[proc->node].next_proc_id++;
    proc->state = PROC_NEW;
    //lock

    print_process(proc,nodes[proc->node].time);

    insert_in_queue(proc, 1);
    return 1;
}

/* Perform the simulation
 * @params:
 *   none
 * @returns:
 *   returns 1
 */
extern int process_simulate(int nodeID) {
    context *cur = NULL;
    int cpu_quantum;

    /* We can only stop when all processes are in the finished state
     * no processes are readdy, running, or blocked
     */
    while(!prio_q_empty(nodes[nodeID].ready) || !prio_q_empty(nodes[nodeID].blocked) || cur != NULL) {
        int preempt = 0;

        /* Step 1: Unblock processes
         * If any of the unblocked processes have higher priority than current running process
         *   we will need to preempt the current running process
         */
        while (!prio_q_empty(nodes[nodeID].blocked)) {
            /* We can stop ff process at head of queue should not be unblocked
             */
            context *proc = prio_q_peek(nodes[nodeID].blocked);
            if (proc->duration > nodes[nodeID].time) {
                break;
            }

            /* Move from blocked and reinsert into appropriate queue
             */
            prio_q_remove(nodes[nodeID].blocked);
            insert_in_queue(proc, 1);

            /* preemption is necessary if a process is running, and it has lower priority than
             * a newly unblocked ready process.
             */
            preempt |= cur != NULL && proc->state == PROC_READY &&
                    actual_priority(cur) > actual_priority(proc);
        }

        /* Step 2: Update current running process
         */
        if (cur != NULL) {
            cur->duration--;
            cpu_quantum--;

            /* Process stops running if it is preempted, has used up their quantum, or has completed its DOOP
             */
            if (cur->duration == 0 || cpu_quantum == 0 || preempt) {
                insert_in_queue(cur, cur->duration == 0);
                cur = NULL;
            }
        }

        /* Step 3: Select next ready process to run if none are running
         * Be sure to keep track of how long it waited in the ready queue
         */
        if (cur == NULL && !prio_q_empty(nodes[nodeID].ready)) {
            cur = prio_q_remove(nodes[nodeID].ready);
            cur->wait_time += nodes[nodeID].time - cur->enqueue_time;
            cpu_quantum = nodes[nodeID].quantum;//might need to give separate quantum
            cur->state = PROC_RUNNING;
            //pthread_mutex_lock(&lock);
            print_process(cur,nodes[nodeID].time);
            //pthread_mutex_unlock(&lock);
            //printf("vwfgwgwefg");
        }

        /* next clock tick
         */
        nodes[nodeID].time++;
    }


    //printf("close to return \n");
    return 1;
}
