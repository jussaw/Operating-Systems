
/*****************************************************************************\
* Laboratory Exercises COMP 3500                                              *
* Author: Saad Biaz                                                           *
* Updated 6/5/2017 to distribute to students to redo Lab 1                    *
* Updated 5/9/2017 for COMP 3500 labs                                         *
* Date  : February 20, 2009                                                   *
\*****************************************************************************/

/*****************************************************************************\
*                             Global system headers                           *
\*****************************************************************************/

#include "common2.h"

/*****************************************************************************\
*                             Global data types                               *
\*****************************************************************************/

typedef enum {TAT,RT,CBT,THGT,WT,WTJQ} Metric;

/*****************************************************************************\
*                             Global definitions                              *
\*****************************************************************************/

//Policies, created by students.
#define NONE_POLICY     -1
#define OMAP_POLICY     0
#define PAGING_POLICY   1
#define BESTFIT_POLICY  2
#define WORSTFIT_POLICY 3

//Page Size of 2^8
#define PAGESIZE        256

//Created by Dr. Biaz.
#define FCFS            1
#define RR              3
#define MAXMETRICS      6
#define MAX_QUEUE_SIZE  10

/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/

//Not available

/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/

//Created by Dr. Biaz.
Quantity NumberofJobs[MAXMETRICS]; // Number of Jobs for which metric was collected
Average  SumMetrics[MAXMETRICS]; // Sum for each Metrics

//2^12
int  NumberOfAvailablePages = 4096;
int  NumberOfNotAvailablePages = 0;

//Linked List Structure
struct Node* head = NULL;

//Initialization
int memoryPolicy = WORSTFIT_POLICY;

/*****************************************************************************\
*                               Function prototypes                           *
\*****************************************************************************/

void                 ManageProcesses(void);
void                 NewJobIn(ProcessControlBlock whichProcess);
void                 BookKeeping(void);
Flag                 ManagementInitialization(void);
void                 LongtermScheduler(void);
void                 IO();
void                 CPUScheduler(Identifier whichPolicy);
ProcessControlBlock *SRTF();
void                 Dispatcher();
void                 checkForMissingPages();
void                 push(struct Node** headIn, int dataIn, int sizeIn);
void                 insert(struct Node* prevNode, int newData, int newSize);
void                 append(struct Node** headIn, int dataIn, int sizeIn);
int                  bestFitImplementation(struct Node *nodeIn, int dataIn, int sizeIn);
int                  worstFitImplementation(struct Node *nodeIn, int datain, int sizeIn);
void                 takeProcessOff(struct Node *nodeIn, int ident);
void                 cleanUpList(struct Node *nodeIn);
void                 removeProcess(struct Node *nodeIn, int ident);
void                 printList(struct Node *nodeIn);

/***************************************************************************
 *                    Implementation of Doubly Linked list                 *
 ***************************************************************************/
 
// Defining the Node struct
struct Node {
	//Pointer to "next" node in DLL
	struct Node *next;
	//Pointer to "previous" node in DLL
	struct Node *prev;
	//Data in node
	int data;
	//Optional, track size of new nodes
	int size;
};

/* Function to add a new node the beginning of the doubly linked list.
Inputs are a pointer to a pointer to a the head node of a list, int data
for the new node and int size for the new node. Used same implementation 
from GeeksForGeeks.org.*/
void push(struct Node** headIn, int dataIn, int sizeIn) {
    // Create and allocate the new node.
    struct Node* newNode = (struct Node*) malloc(sizeof(struct Node));

    // Copy the data and size from the input to the new node.
    newNode->data = dataIn;
    newNode->size = sizeIn;

    /* Make the new node point its next pointer to the head and the previous
    pointer to NULL. */
    newNode->next = (*headIn);
    newNode->prev = NULL;

    // If there is a head node, make its previous pointer point to the new node.
    if((*headIn) !=  NULL){
      (*headIn)->prev = newNode ;
    }

    // Change the head reference to point to the new node
    (*headIn) = newNode;
}

/* Function to insert a a Node doubly linked list. Inputs are a pointer to
the prevNode, int dataIn, int sizeIn. Used same implementation from
GeeksForGeeks.org.*/
void insert(struct Node* prevNode, int newData, int newSize) {
    /* Check to make sure that the prevNode is NULL.
    If so, then exit cleanly. */
    if (prevNode == NULL){
        exit(0);
    }

    // Create and allocate the new node.
    struct Node* newNode =(struct Node*) malloc(sizeof(struct Node));

    // Copy the data and size from the input to the new node.
    newNode->data  = newData;
    newNode->size  = newSize;

    /* Point the next pointer of new node to what next node of prevNode 
    points to. */
    newNode->next = prevNode->next;

    // Point next node of prevNode to newNode.
    prevNode->next = newNode;

    // Point previous node of newNode to prevNode.
    newNode->prev = prevNode;

    // Change the previous node of the next node of newNode to newNode.
    if (newNode->next != NULL){
		
	  //Not entirely sure what this does. GeeksForGeeks implementation.
      newNode->next->prev = newNode;
    }
}

/* Function to append a new node to the end of the double linked list, given
that a reference to the head and an int is provided. Used same implementation
as GeeksForGeeks as usual. */
void append(struct Node** headIn, int dataIn, int sizeIn) {
    // Create and allocate the new node. The last node will be used later.
    struct Node* newNode = (struct Node*) malloc(sizeof(struct Node));
    struct Node *last = *headIn;

    // Copy the data and size from the input to the new node.
    newNode->data  = dataIn;
    newNode->size  = sizeIn;

    // Since newNode will be the last node, its next node will be NULL.
    newNode->next = NULL;

    /* Checks if the input list is empty by checking if headIn is NULL.
	If so then set the newNode as the head.*/
    if (*headIn == NULL){
      newNode->prev = NULL;
      *headIn = newNode;
      return;
    }

    /* Loop until we get to the last node in the linked list. On each iteration
    set last to the current node until we reach then end. */
    while (last->next != NULL){
      last = last->next;
    }

    // Sets the next node of the last to newNode
    last->next = newNode;

    // Sets the previous node of newNode to last
    newNode->prev = last;
    return;
}

void checkForMissingPages(){
  if(NumberOfAvailablePages + NumberOfNotAvailablePages != (1048576 / 256)){
    //printf(">>>>Error Pages have gotten lost somehow<<<<\nPages Available: %d Pages Taken: %d\n" , NumberOfAvailablePages, NumberOfNotAvailablePages);
    exit(0);
  }
}

/****************************************************************************
 *                   End of Doubly Linked List                              *
 ****************************************************************************/

int bestFitImplementation(struct Node *nodeIn, int dataIn, int sizeIn) {
  struct Node *checkIfBetter = nodeIn;
  
  //Variables to track best fit nodes
  int best = -1;
  int bestLocation = 0;
  
  //Indexes for while loops
  int counter = 0;
  int counter_2 = 0;
  
  //Safety check
  if(nodeIn == NULL) {
    return -1;
  }
  
  /* Find where to insert the new node and if the node is better,
  set the best node to that node's size and location instead. */
  while(checkIfBetter != NULL) {
    if(checkIfBetter->data == -1 
		// If size is bigger than or equal to new node
		&& (checkIfBetter->size >= sizeIn)
		
		// If better than best
		&& (checkIfBetter->size < best || best < 0)) {
			
		//Update best node if conditions check
        best = checkIfBetter->size;
        bestLocation = counter;
		
		//Continue incrementing if not
    } checkIfBetter = checkIfBetter->next;
    counter++;
  }
  
  // If while condition did not produce any results, exit with not found
  if(best == -1) {
    return -1;
  }
  
  /* Operations to ??? */
  else {
    while(nodeIn != NULL){
      if(counter_2 == bestLocation){
        if(nodeIn->size == sizeIn){
          nodeIn->size = sizeIn;
          nodeIn->data = dataIn;
          return 0;
        }
        else{
          insert(nodeIn, dataIn, sizeIn);
          nodeIn->size = (nodeIn->size - sizeIn);
          return 0;
        }
      }
      nodeIn = nodeIn->next;
      counter_2++;
    }
  }
  return 0;
}


int worstFitImplementation(struct Node *nodeIn, int datain, int sizeIn){
  struct Node *iteratedNode = nodeIn;
  
  //Variables to track worst fit nodes
  int worst = 0;
  int worstLocation = 0;
  
  //Indexes for while loops
  int counter = 0;
  int counter_2 = 0;
  
  //Safety check
  if(nodeIn == NULL){
    return -1;
  }
  while(iteratedNode != NULL){
    if(iteratedNode->data == -1 && iteratedNode->size >= sizeIn
      && iteratedNode->size > worst)  {
        worst = iteratedNode->size;
        worstLocation = counter;
    }
    iteratedNode = iteratedNode->next;
    counter++;
  }
  counter_2 = 0;
  if(worst == -1){
    return -1;
  }
  else{
    while(nodeIn != NULL){
      if(counter_2 == worstLocation){
        if(nodeIn->size == sizeIn){
          nodeIn->size = sizeIn;
          nodeIn->data = datain;
          return 0;
        }
        else{
          insert(nodeIn, datain, sizeIn);
          nodeIn->size = (nodeIn->size - sizeIn);
          return 0;
        }
      }
      nodeIn = nodeIn->next;
      counter_2++;
    }
  }
  return 0;
}


void takeProcessOff(struct Node *nodeIn, int ident){
  while (nodeIn != NULL){
    if(nodeIn-> data == ident){
      nodeIn->data = -1;
      break;
    }
    nodeIn = nodeIn->next;
  }
}


void cleanUpList(struct Node *nodeIn){
  while (nodeIn != NULL && nodeIn->next != NULL){
    if(nodeIn->data == -1 && nodeIn->next->data == -1){
        nodeIn->size += nodeIn->next->size;
        nodeIn->next = nodeIn->next->next;
    }
    nodeIn = nodeIn->next;
  }
}


void removeProcess(struct Node *nodeIn, int ident){
  takeProcessOff(nodeIn, ident);
  cleanUpList(nodeIn);
}


void printList(struct Node *nodeIn){
    printf("%s\n", "List:");
    while (nodeIn != NULL)
    {
        printf("Data: %d Size: %d \n", nodeIn->data, nodeIn->size);
        //last = node;
        nodeIn = nodeIn->next;
    }
    printf("\n" );
}


/*****************************************************************************\
* function: main()                                                            *
* usage:    Create an artificial environment operating systems. The parent    *
*           process is the "Operating Systems" managing the processes using   *
*           the resources (CPU and Memory) of the system                      *
*******************************************************************************
* Inputs: ANSI flat C command line parameters                                 *
* Output: None                                                                *
*                                                                             *
* INITIALIZE PROGRAM ENVIRONMENT                                              *
* START CONTROL ROUTINE                                                       *
\*****************************************************************************/

int main (int argc, char **argv) {
   if (Initialization(argc,argv)){
     ManageProcesses();
   }
} /* end of main function */

/***********************************************************************\
* Input : none                                                          *
* Output: None                                                          *
* Function: Monitor Sources and process events (written by students)    *
\***********************************************************************/

void ManageProcesses(void){
  ManagementInitialization();
  while (1) {
    IO();
    CPUScheduler(PolicyNumber);
    Dispatcher();
  }
}

/***********************************************************************\
* Input : none                                                          *          
* Output: None                                                          *        
* Function:                                                             *
*    1) if CPU Burst done, then move process on CPU to Waiting Queue    *
*         otherwise (RR) return to rReady Queue                         *                           
*    2) scan Waiting Queue to find processes with complete I/O          *
*           and move them to Ready Queue                                *         
\***********************************************************************/
void IO() {
  ProcessControlBlock *currentProcess = DequeueProcess(RUNNINGQUEUE); 
  if (currentProcess){
    if (currentProcess->RemainingCpuBurstTime <= 0) { // Finished current CPU Burst
      currentProcess->TimeEnterWaiting = Now(); // Record when entered the waiting queue
      EnqueueProcess(WAITINGQUEUE, currentProcess); // Move to Waiting Queue
      currentProcess->TimeIOBurstDone = Now() + currentProcess->IOBurstTime; // Record when IO completes
      currentProcess->state = WAITING;
    } else { // Must return to Ready Queue                
      currentProcess->JobStartTime = Now();                                               
      EnqueueProcess(READYQUEUE, currentProcess); // Mobe back to Ready Queue
      currentProcess->state = READY; // Update PCB state 
    }
  }

  /* Scan Waiting Queue to find processes that got IOs  complete*/
  ProcessControlBlock *ProcessToMove;
  /* Scan Waiting List to find processes that got complete IOs */
  ProcessToMove = DequeueProcess(WAITINGQUEUE);
  if (ProcessToMove){
    Identifier IDFirstProcess =ProcessToMove->ProcessID;
    EnqueueProcess(WAITINGQUEUE,ProcessToMove);
    ProcessToMove = DequeueProcess(WAITINGQUEUE);
    while (ProcessToMove){
      if (Now()>=ProcessToMove->TimeIOBurstDone){
	ProcessToMove->RemainingCpuBurstTime = ProcessToMove->CpuBurstTime;
	ProcessToMove->JobStartTime = Now();
	EnqueueProcess(READYQUEUE,ProcessToMove);
      } else {
	EnqueueProcess(WAITINGQUEUE,ProcessToMove);
      }
      if (ProcessToMove->ProcessID == IDFirstProcess){
	break;
      }
      ProcessToMove =DequeueProcess(WAITINGQUEUE);
    } // while (ProcessToMove)
  } // if (ProcessToMove)
}

/***********************************************************************\    
 * Input : whichPolicy (1:FCFS, 2: SRTF, and 3:RR)                      *        
 * Output: None                                                         * 
 * Function: Selects Process from Ready Queue and Puts it on Running Q. *
\***********************************************************************/
void CPUScheduler(Identifier whichPolicy) {
  ProcessControlBlock *selectedProcess;
  if ((whichPolicy == FCFS) || (whichPolicy == RR)) {
    selectedProcess = DequeueProcess(READYQUEUE);
  } else{ // Shortest Remaining Time First 
    selectedProcess = SRTF();
  }
  if (selectedProcess) {
    selectedProcess->state = RUNNING; // Process state becomes Running                                     
    EnqueueProcess(RUNNINGQUEUE, selectedProcess); // Put process in Running Queue                         
  }
}

/***********************************************************************\                         
 * Input : None                                                         *                                     
 * Output: Pointer to the process with shortest remaining time (SRTF)   *                                     
 * Function: Returns process control block with SRTF                    *                                     
\***********************************************************************/
ProcessControlBlock *SRTF() {
  /* Select Process with Shortest Remaining Time*/
  ProcessControlBlock *selectedProcess, *currentProcess = DequeueProcess(READYQUEUE);
  selectedProcess = (ProcessControlBlock *) NULL;
  if (currentProcess){
    TimePeriod shortestRemainingTime = currentProcess->TotalJobDuration - currentProcess->TimeInCpu;
    Identifier IDFirstProcess =currentProcess->ProcessID;
    EnqueueProcess(READYQUEUE,currentProcess);
    currentProcess = DequeueProcess(READYQUEUE);
    while (currentProcess){
      if (shortestRemainingTime >= (currentProcess->TotalJobDuration - currentProcess->TimeInCpu)){
	EnqueueProcess(READYQUEUE,selectedProcess);
	selectedProcess = currentProcess;
	shortestRemainingTime = currentProcess->TotalJobDuration - currentProcess->TimeInCpu;
      } else {
	EnqueueProcess(READYQUEUE,currentProcess);
      }
      if (currentProcess->ProcessID == IDFirstProcess){
	break;
      }
      currentProcess =DequeueProcess(READYQUEUE);
    } // while (ProcessToMove)
  } // if (currentProcess)
  return(selectedProcess);
}

/***********************************************************************\  
 * Input : None                                                         *   
 * Output: None                                                         *   
 * Function:                                                            *
 *  1)If process in Running Queue needs computation, put it on CPU      *
 *              else move process from running queue to Exit Queue      *     
\***********************************************************************/
void Dispatcher() {
  double start;
  int pagesToBeReleased;
  ProcessControlBlock *processOnCPU = Queues[RUNNINGQUEUE].Tail; // Pick Process on CPU
  if (!processOnCPU) { // No Process in Running Queue, i.e., on CPU
    return;
  }
  if(processOnCPU->TimeInCpu == 0.0) { // First time this process gets the CPU
    SumMetrics[RT] += Now()- processOnCPU->JobArrivalTime;
    NumberofJobs[RT]++;
    processOnCPU->StartCpuTime = Now(); // Set StartCpuTime
  }
  if (processOnCPU->TimeInCpu >= processOnCPU-> TotalJobDuration) { // Process Complete
    if (memoryPolicy == OMAP_POLICY ){
        AvailableMemory += processOnCPU->MemoryRequested; //release memory being held by process
    }
    else if (memoryPolicy == PAGING_POLICY ){
        pagesToBeReleased = (processOnCPU->MemoryRequested / PAGESIZE); //Calculating the number of pages and rounds it up
        if(processOnCPU->MemoryRequested % PAGESIZE > 0){
          pagesToBeReleased++;
        }
        NumberOfAvailablePages += pagesToBeReleased;
        NumberOfNotAvailablePages -= pagesToBeReleased;
        checkForMissingPages();
    }
    else if (memoryPolicy == BESTFIT_POLICY || memoryPolicy == WORSTFIT_POLICY){
      AvailableMemory += processOnCPU->MemoryRequested;
      removeProcess(head, processOnCPU->ProcessID);
    }

    NumberofJobs[THGT]++;
    NumberofJobs[TAT]++;
    NumberofJobs[WT]++;
    NumberofJobs[CBT]++;

    printf(" >>>>>Process # %d complete, %d Processes Completed So Far<<<<<<\n",
    processOnCPU->ProcessID,NumberofJobs[THGT]);
    processOnCPU=DequeueProcess(RUNNINGQUEUE);
    EnqueueProcess(EXITQUEUE,processOnCPU);


    SumMetrics[TAT]     += Now() - processOnCPU->JobArrivalTime;
    SumMetrics[WT]      += processOnCPU->TimeInReadyQueue;


    // processOnCPU = DequeueProcess(EXITQUEUE);
    // XXX free(processOnCPU);

  }
  else { // Process still needs computing, out it on CPU
    TimePeriod CpuBurstTime = processOnCPU->CpuBurstTime;
    processOnCPU->TimeInReadyQueue += Now() - processOnCPU->JobStartTime;
    if (PolicyNumber == RR){
      CpuBurstTime = Quantum;
      if (processOnCPU->RemainingCpuBurstTime < Quantum)
       CpuBurstTime = processOnCPU->RemainingCpuBurstTime;
    }
    processOnCPU->RemainingCpuBurstTime -= CpuBurstTime;
    // SB_ 6/4 End Fixes RR
    TimePeriod StartExecution = Now();
    OnCPU(processOnCPU, CpuBurstTime); // SB_ 6/4 use CpuBurstTime instead of PCB-> CpuBurstTime
    processOnCPU->TimeInCpu += CpuBurstTime; // SB_ 6/4 use CpuBurstTime instead of PCB-> CpuBurstTimeu
    SumMetrics[CBT] += CpuBurstTime;
  }
}


/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This routine is run when a job is added to the Job Queue    *
\***********************************************************************/
void NewJobIn(ProcessControlBlock whichProcess){
  ProcessControlBlock *NewProcess;
  /* Add Job to the Job Queue */
  NewProcess = (ProcessControlBlock *) malloc(sizeof(ProcessControlBlock));
  memcpy(NewProcess,&whichProcess,sizeof(whichProcess));
  NewProcess->TimeInCpu = 0; // Fixes TUX error
  NewProcess->RemainingCpuBurstTime = NewProcess->CpuBurstTime; // SB_ 6/4 Fixes RR
  EnqueueProcess(JOBQUEUE,NewProcess);
  DisplayQueue("Job Queue in NewJobIn",JOBQUEUE);
  LongtermScheduler(); /* Job Admission  */
}


/***********************************************************************\                                                   
* Input : None                                                         *                                                    
* Output: None                                                         *                                                    
* Function:                                                            *
* 1) BookKeeping is called automatically when 250 arrived              *
* 2) Computes and display metrics: average turnaround  time, throughput*
*     average response time, average waiting time in ready queue,      *
*     and CPU Utilization                                              *                                                     
\***********************************************************************/
void BookKeeping(void){
  double end = Now(); // Total time for all processes to arrive
  Metric m;

  // Compute averages and final results
  if (NumberofJobs[TAT] > 0){
    SumMetrics[TAT] = SumMetrics[TAT]/ (Average) NumberofJobs[TAT];
  }
  if (NumberofJobs[RT] > 0){
    SumMetrics[RT] = SumMetrics[RT]/ (Average) NumberofJobs[RT];
  }
  SumMetrics[CBT] = SumMetrics[CBT]/ Now();

  if (NumberofJobs[WT] > 0){
    SumMetrics[WT] = SumMetrics[WT]/ (Average) NumberofJobs[WT];
  }

  if (NumberofJobs[WTJQ] > 0) {
    SumMetrics[WTJQ] = SumMetrics[WTJQ]/ (Average) NumberofJobs[WTJQ];
  }

  printf("\n********* Processes Management Numbers ******************************\n");
  printf("Policy Number = %d, Quantum = %.6f   Show = %d\n", PolicyNumber, Quantum, Show);
  printf("Number of Completed Processes = %d\n", NumberofJobs[THGT]);
  printf("ATAT=%f   ART=%f  CBT = %f  T=%f AWT=%f AWTJQ=%f\n", 
	 SumMetrics[TAT], SumMetrics[RT], SumMetrics[CBT], 
	 NumberofJobs[THGT]/Now(), SumMetrics[WT], SumMetrics[WTJQ]);
  exit(0);
}

/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: Decides which processes should be admitted in Ready Queue   *
*           If enough memory and within multiprogramming limit,         *
*           then move Process from Job Queue to Ready Queue             *
\***********************************************************************/
void LongtermScheduler(void){
  ProcessControlBlock *currentProcess = DequeueProcess(JOBQUEUE);
  while (currentProcess) {
    if(memoryPolicy == OMAP_POLICY ){
        if (AvailableMemory >= currentProcess->MemoryRequested){
          AvailableMemory -= currentProcess->MemoryRequested;
        }
        else{
          EnqueueProcess(JOBQUEUE, currentProcess);
          return;
        }
    }
    else if(memoryPolicy == PAGING_POLICY ){
        int pagesRequested = (currentProcess->MemoryRequested / PAGESIZE);
        if(currentProcess->MemoryRequested % PAGESIZE > 0){
          pagesRequested++;
        }
        if(pagesRequested <= NumberOfAvailablePages){
          NumberOfAvailablePages -= pagesRequested;
          NumberOfNotAvailablePages += pagesRequested;
          checkForMissingPages();
        }
        else{
          EnqueueProcess(JOBQUEUE, currentProcess);
          return;
        }
      }
    else if(memoryPolicy == BESTFIT_POLICY){
      if(AvailableMemory >= currentProcess->MemoryRequested){
          if(bestFitImplementation(head, currentProcess->ProcessID, currentProcess->MemoryRequested) == -1){
              push(&head, currentProcess->ProcessID, currentProcess->MemoryRequested);
            }
          if(head->data == -1){
            head = head->next;
            head->prev = NULL;
          }

          AvailableMemory-= currentProcess->MemoryRequested;
      }
      else{
        EnqueueProcess(JOBQUEUE, currentProcess);
        return;
      }
    }
    else if(memoryPolicy == WORSTFIT_POLICY){
      if(AvailableMemory >= currentProcess->MemoryRequested){
          if(worstFitImplementation(head, currentProcess->ProcessID, currentProcess->MemoryRequested) == -1){
              push(&head, currentProcess->ProcessID, currentProcess->MemoryRequested);
            }
          if(head->data == -1){
            head = head->next;
            head->prev = NULL;
          }

          AvailableMemory-= currentProcess->MemoryRequested;
      }
      else{
        EnqueueProcess(JOBQUEUE, currentProcess);
        return;
      }
    }
    currentProcess->TimeInJobQueue = Now() - currentProcess->JobArrivalTime;
    currentProcess->JobStartTime = Now();
    EnqueueProcess(READYQUEUE,currentProcess);
    currentProcess->state = READY;
    NumberofJobs[WTJQ]++;
    SumMetrics[WTJQ] += currentProcess->TimeInJobQueue;
    currentProcess = DequeueProcess(JOBQUEUE);
  }
}

/***********************************************************************\
* Input : None                                                          *
* Output: TRUE if Initialization successful                              *
\***********************************************************************/
Flag ManagementInitialization(void){
  Metric m;
  for (m = TAT; m < MAXMETRICS; m++){
     NumberofJobs[m] = 0;
     SumMetrics[m]   = 0.0;
  }
  return TRUE;
}