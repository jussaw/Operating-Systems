
/*****************************************************************************\
* Laboratory Exercises COMP 3500                                              *
* Author: Saad Biaz                                                           *
* Updated 5/22/2017 to distribute to students to do Lab 1                     *
*                                                                             *
*                                                                             *
*                                                                             *
* Group Number: 1                                                             *
* Students that completed this Lab: Paul Chong    (phc0004@auburn.edu)        *
*   								Justin Sawyer (jts0047@auburn.edu)        *
\*****************************************************************************/

/*****************************************************************************\
*                             Global system headers                           *
\*****************************************************************************/
#include "common.h"

/*****************************************************************************\
*                             Global data types                               *
\*****************************************************************************/
typedef enum {TAT,RT,CBT,THGT,WT} Metric;

//No global data types besides the one provided above implemented.

/*****************************************************************************\
*                             Global definitions                              *
\*****************************************************************************/
#define MAX_QUEUE_SIZE 10 
#define FCFS            1 
#define SJF             2
#define RR              3 
#define MAXMETRICS      5 


/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/

//No global data structures implemented.

/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/
Quantity NumberofJobs[MAXMETRICS]; // Number of Jobs for which metric was collected
Average  SumMetrics[MAXMETRICS]; // Sum for each Metrics

//No global data besides the ones provided above implemented.


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
ProcessControlBlock *SJF_Scheduler();
ProcessControlBlock *FCFS_Scheduler();
ProcessControlBlock *RR_Scheduler();
void                 Dispatcher();


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


/* XXXXXXXXX Do Not Change IO() Routine XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
/***********************************************************************\
* Input : none                                                          *          
* Output: None                                                          *        
* Function:                                                             *
*    1) if CPU Burst done, then move process on CPU to Waiting Queue    *
*         otherwise (for RR) return Process to Ready Queue              *                           
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
 * Input : whichPolicy (1:FCFS, 2: SJF , and 3:RR)                      *      
 * Output: None                                                         * 
 * Function: Selects Process from Ready Queue and Puts it on Running Q. *
\***********************************************************************/
void CPUScheduler(Identifier whichPolicy) {
  ProcessControlBlock *selectedProcess;
  switch(whichPolicy){
    case FCFS : selectedProcess = FCFS_Scheduler();
      break;
    case SJF : selectedProcess = SJF_Scheduler();
      break;
    case RR   : selectedProcess = RR_Scheduler();
  }
  if (selectedProcess) {
    selectedProcess->state = RUNNING; // Process state becomes Running                                     
    EnqueueProcess(RUNNINGQUEUE, selectedProcess); // Put process in Running Queue                         
  }
}


/***********************************************************************\                                               
 * Input : None                                                         *                                               
 * Output: Pointer to the process based on First Come First Serve (FCFS)*
 * Function: Returns process control block based on FCFS                *                                                
\***********************************************************************/
ProcessControlBlock *FCFS_Scheduler() {
  /*Select Process based on FCFS 
  Implement code for FCFS*/ 

  //Dequeue'd Portion of Ready
  ProcessControlBlock *selectedProcess = (ProcessControlBlock *) DequeueProcess(READYQUEUE);

  //Stability purposes
  if (selectedProcess == NULL) {
    return NULL;
  }
  
  //Return the process for CPUScheduler to put into the running queue
  return(selectedProcess);
}


/***********************************************************************\                         
 * Input : None                                                         *                            
 * Output: Pointer to the process with smallest remaining time(SJF)     *                             
 * Function: Returns process control block with SJF                     *              
\***********************************************************************/
ProcessControlBlock *SJF_Scheduler() {
	/* Select Process with Shortest Remaining Time*/
	// Implement code for SJF

	ProcessControlBlock *selectedProcess = (ProcessControlBlock *) DequeueProcess(READYQUEUE);
	
	//Stability purposes
	if(selectedProcess == NULL) {
		return NULL;
	}  

	//Set pointer as condition
	if(selectedProcess) {

		//Same as selectedProcess
		ProcessControlBlock *otherProcess = DequeueProcess(READYQUEUE);

		//Pointer to link back at the end of the loop and exit
		ProcessControlBlock *originalProcess = selectedProcess;

		//Endless, need update.
		while(otherProcess) {

			// Check to see if otherProcess (process we are comparing with) is less than
			// the current selectedProcess (shortest process). If so then enqueue it onto the 
			// ready queue and set the selectedProcess to otherProess. If it is not less, then
			// enqueue otherProcess to the ready queue.
			if(otherProcess->RemainingCpuBurstTime < selectedProcess->RemainingCpuBurstTime) {
				EnqueueProcess(READYQUEUE, selectedProcess);

				//Condition for loop.
				selectedProcess = otherProcess;
			}

			//If not less than, enqueue this one instead.
			else {
				EnqueueProcess(READYQUEUE, otherProcess);
			}

      		// This if statement is to check to ensure we do not have an infinite loop.
			if(originalProcess->ProcessID == otherProcess->ProcessID) {

		        // If the current minimum process is not equal to otherProcess, then enqueue
		        // otherProcess in the ready queue.
				if(selectedProcess->ProcessID != otherProcess->ProcessID) {
					EnqueueProcess(READYQUEUE, otherProcess);
				}

				//Checked, safe
				break;
			}

			//Break out of endless
			otherProcess = DequeueProcess(READYQUEUE);
		}
	}
	return selectedProcess;
}


/***********************************************************************\                                               
 * Input : None                                                         *                                               
 * Output: Pointer to the process based on Round Robin (RR)             *                                               
 * Function: Returns process control block based on RR                  *                                              \
 \***********************************************************************/
ProcessControlBlock *RR_Scheduler() {
  /* Select Process based on RR*/
  // Implement code for RR

  //Dequeue'd portion of ready queue
  ProcessControlBlock *selectedProcess = (ProcessControlBlock *) DequeueProcess(READYQUEUE);
  
  //Stability purposes
  if (selectedProcess == NULL) {
    return NULL;
  }

  //Return the process for CPUScheduler to put into the running queue
  //when Round Robin policy is entered.
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
  //Dequeue'd portion of the ready queue
  ProcessControlBlock *currentProcess = (ProcessControlBlock *) DequeueProcess(RUNNINGQUEUE);
  double start;

  //Stability purposes
  if (currentProcess == NULL) {
    return;
  }

  //if(First time this process gets the CPU  ) { /                  
  //  Update for this process the field StartCpuTime (in the PCB) 
  //}
  if (currentProcess->TimeInCpu == 0) {

    //Update one response time job because the process is first
    //getting the CPU and RT = FirstTime - Arrival.
    NumberofJobs[RT]++;

    //Similarly, update CBT once to note that the process has
    //gotten its first entry.
    NumberofJobs[CBT]++;

    //Updated as instructed.
    currentProcess->StartCpuTime = Now();

    //FirstTime (SystemT) - Arrival.
    SumMetrics[RT] += Now() - currentProcess->JobArrivalTime;
  }

  //if (Process totally Complete) { //            
  //  remove process from running queue
  //  add process to exit queue                                   
  //}
  if (currentProcess->TimeInCpu >= currentProcess->TotalJobDuration) {

    //Update one Wait Time job as process is completed.
    NumberofJobs[WT]++;

    //Similarly, process completed so thus throughput complete.
    NumberofJobs[THGT]++;

    //Change state to DONE and the exit time to current system time since
    //the process is to be moved to the exit queue.
    currentProcess->state = DONE;
    currentProcess->JobExitTime = Now();

    //Calculate turnaround time as learned previously,
    //Turnaround Time = Completed - Arrival.
    SumMetrics[TAT] += (currentProcess->JobExitTime - currentProcess->JobArrivalTime);

    //**FIXED incorrect metrics**
    //Calculate wait time as learned previously,
    //Wait Time = Time spent in ready queue. Order here does not necessarily matter
    //as all need to be eliminated to calculate the time portions in READYQUEUE.
    //DisplayQueue function not needed.
    SumMetrics[WT] += (currentProcess->JobExitTime - currentProcess->TimeInJobQueue
                                                   - currentProcess->JobArrivalTime 
                                                   - currentProcess->TimeInCpu 
                                                   - currentProcess->TimeInWaitQueue);
    
    //Remove process from the running queue and add to the exit queue as instructed.
    DequeueProcess(RUNNINGQUEUE);
    EnqueueProcess(EXITQUEUE, currentProcess);
  }

  // Process still needs computing 
  //else {
  else {

  	//Determine CpuBurstTime: the length of the CPU burst needed 
  	//by the process (depends on whether RR or no)
    if (PolicyNumber == RR) {

      //If Round Robin, only allow the CPU to burst at what the provided
      //quantum is.
      currentProcess->CpuBurstTime = Quantum;
    }

  	//Update the field RemainingCpuBurstTime in the PCB
    if (currentProcess->CpuBurstTime > currentProcess->TotalJobDuration 
                                     - currentProcess->TimeInCpu) {

      //Ensure that the CPU bursts until the job is finished.
    	currentProcess->CpuBurstTime = currentProcess->TotalJobDuration - currentProcess->TimeInCpu;
    }

  	//Put the process on the CPU for CpuBurstTime using the function 
  	//OnCPU(processOnCPU, CpuBurstTime) where processOnCPU is a pointer 
  	//to the process running
    OnCPU(currentProcess, currentProcess->CpuBurstTime);

  	//Update the field TimeInCPU
  	//processOnCPU->TimeInCpu += CpuBurstTime; // SB_ 6/4 use CpuBurstTime 
  	//instead of PCB-> CpuBurs\tTimeu                                                                                            
    currentProcess->TimeInCpu += currentProcess->CpuBurstTime;

  	//SumMetrics[CBT] += CpuBurstTime;
    SumMetrics[CBT] += currentProcess->CpuBurstTime;

    //Must put the process back into the running queue, as it was
    //taken out in the beginning.
    EnqueueProcess(RUNNINGQUEUE, currentProcess);
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
  printf("\n********* Processes Managemenent Numbers ******************************\n");
  printf("Policy Number = %d, Quantum = %.6f   Show = %d\n", PolicyNumber, Quantum, Show);
  printf("Number of Completed Processes = %d\n", NumberofJobs[THGT]);
  printf("ATAT=%f   ART=%f  CBT = %f  T=%f AWT=%f\n",

  //Average Turnaround Time = Turnaround Time divided by N number of 
  //turnaround time jobs.
  SumMetrics[TAT]/NumberofJobs[THGT],

  //Average Response Time = Response Time divided by N number of response
  //time jobs.
  SumMetrics[RT]/NumberofJobs[RT], 

  //Average CPU Utilization = CBT divided by current system time. 
  //Much more effective than trying to calculate the time manually.
  SumMetrics[CBT]/end,

  //Average Throughput = Throughput divided by current system time.
  //As above, much more effective and reliable with TUX.
  NumberofJobs[THGT]/end, 

  //Average Wait Time = Wait Time in the ready queue divided by 
  //N number of wait time jobs.
  SumMetrics[WT]/NumberofJobs[WT]);

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
    currentProcess->TimeInJobQueue = Now() - currentProcess->JobArrivalTime; // Set TimeInJobQueue
    currentProcess->JobStartTime = Now(); // Set JobStartTime
    EnqueueProcess(READYQUEUE,currentProcess); // Place process in Ready Queue
    currentProcess->state = READY; // Update process state
    currentProcess = DequeueProcess(JOBQUEUE);
  }
}


/***********************************************************************\
* Input : None                                                          *
* Output: TRUE if Intialization successful                              *
\***********************************************************************/
Flag ManagementInitialization(void){
  Metric m;
  for (m = TAT; m < MAXMETRICS; m++){
     NumberofJobs[m] = 0;
     SumMetrics[m]   = 0.0;
  }
  return TRUE;
}