#include "NonPreemptive.h"

//Global variables
Task_t tasks[TASKS_N];
Scheduler_t scheduler;


// ************************Functions definitions ************************
long milliseconds(void)
{
    return clock() / (CLOCKS_PER_SEC / 1000);
}

/// Scheduler_Init
/// <summary>
/// The function should initialize the hscheduler structure with the values passed as parameters
/// the number of tasks to use, the tick value use a base time, the address of the array of task TCBs
/// and the amount of time the scheduler should run.
/// </summary>
/// <param name="hscheduler"></param>

void Scheduler_Init(Scheduler_t* hscheduler)
{
    hscheduler->tasksNum = TASKS_N;
    hscheduler->TaskList = &tasks;
    hscheduler->BaseTick = TICK_VAL;
    hscheduler->timeout = TIMEOUT;
    hscheduler->tasksCount = 0;
}
/// <summary>
/// The function will set the task TCB with the following parameters, 
/// the address of the function to hold the init routine for the given task and the 
/// address for the actual routine that will run as the task, plus the periodicity in milliseconds
/// of the task to register, if the task does not have an init routine a NULL parameter should be 
/// accepted by the function, another thing to validate is the Periodicity should not be less than 
/// the tick value and always be multiple.The function shall return a Task ID which will be a number 
/// from 1 to n task registered if the operation was a success, otherwise, it will return zero.
/// </summary>
unsigned char Scheduler_RegisterTask(Scheduler_t* hscheduler, void (*InitPtr)(void), void (*TaskPtr)(void), unsigned long period)
{
   
    unsigned char returnVal;
    if (InitPtr != NULL)
    {
        hscheduler->TaskList[hscheduler->tasksCount].InitFunc = InitPtr;
    }
    else
    {
        hscheduler->TaskList[hscheduler->tasksCount].InitFunc = NULL;
    }
    
    hscheduler->TaskList[hscheduler->tasksCount].TickFct = TaskPtr;

    if (period % hscheduler->BaseTick == 0)
    {
        hscheduler->TaskList[hscheduler->tasksCount].period = period;
        hscheduler->TaskList[hscheduler->tasksCount].stop = 0;
        hscheduler->TaskList[hscheduler->tasksCount].elapsedTime = period;
        hscheduler->tasksCount = (hscheduler->tasksCount + 1) % (hscheduler->tasksNum);
        returnVal = hscheduler->tasksCount;
    }
    else
    {
        returnVal = 0;
    }

   
    return returnVal;
}
/// <summary>
/// By default after registering a task this will start running within the scheduler,
/// this function can prevent it from being dispatched. The second parameter indicates
/// the task to be stopped, which is a number from 1 to n task registered.Number zero 
/// is forbidden. The function will return a TRUE if the task was stopped otherwise 
/// returns FALSE
/// </summary>
unsigned char Scheduler_StopTask(Scheduler_t* hscheduler, unsigned char task)
{
    unsigned char returnVal;
    if ((task <= hscheduler->tasksNum) && (task > 0))
    {
        hscheduler->TaskList[task - 1].stop = 1;
        returnVal = 1;
    }
    else
    {
        returnVal = 0;
    }
    
    return returnVal;


}
/// <summary>
/// Once a task is stopped using the function Scheduler_StopTask , 
/// it can be active again using this function. The second parameter indicates the task
///  to be started, which is a number from 1 to n task registered.Number zero is forbidden.
/// The function will return a TRUE if the task was stopped otherwise returns FALSE
/// </summary>
unsigned char Scheduler_StartTask(Scheduler_t* hscheduler, unsigned char task) 
{
    unsigned char returnVal;
    if ((task <= hscheduler->tasksNum) && (task > 0))
    {
        hscheduler->TaskList[task - 1].stop = 1;
        returnVal = 1;
    }
    else
    {
        returnVal = 0;
    }
    return returnVal;
}

/// <summary>
/// The new periodicity shall be a multiple of the tick value otherwise won’t be 
/// affected by the new period. The second parameter indicates the task to be started, 
/// which is a number from 1 to n task registered.Number zero is forbidden.the function
///  will return a TRUE if the task was stopped otherwise returns FALSE.
/// </summary>
unsigned char Scheduler_PeriodTask(Scheduler_t* hscheduler, unsigned char task, unsigned long period)
{
    unsigned char returnVal;
    if (period % hscheduler->BaseTick == 0)
    {
        hscheduler->TaskList[task-1].period = period;
        returnVal = 1;
    }
    else
    {
        returnVal = 0;
    }
    return returnVal;
}
/// <summary>
/// Init and task funciton definitions
/// </summary>
/// <param name=""></param>
void Init_200ms(void)
{
    printf("Init task 200 millisecond\n");
}
void Init_1000ms(void)
{
    printf("Init task 1000 millisecond\n");
}
void Task_200ms(void)
{
    static int loop = 0;
    if (loop == TIMESRUN1)
    {
        Scheduler_StopTask(&scheduler, 1);
    }
    else
    {
        printf("This is a counter from task 200ms: %d\n", loop++);
    }
    
}
void Task_1000ms(void)
{
    static int loop = 0;
    if (loop == TIMESRUN2)
    {
        Scheduler_StopTask(&scheduler, 2);
    }
    else
    {
        printf("This is a counter from task 1000ms: %d\n", loop++);
    }
}

/// <summary>
/// This is the function in charge of running the task init functions one single time 
/// and actual run each registered task according to their periodicity in an infinite loop,
///  the function will never return at least something wrong happens, but this will be considered
///  a malfunction.A timer will need to accomplish this purpose, for practical reasons, 
/// we can use the clock functions and the milliseconds functions 
/// </summary>
/// <param name="hscheduler"></param>
void Scheduler_Start(Scheduler_t* hscheduler)
{
    // Timer count
    long x = 0;
    ///get the milliseconds for the first time
    unsigned long LastTick = milliseconds();
    unsigned long ActualTick;
    for (int i = 0; i < hscheduler->tasksNum; i++)
    {
        if (hscheduler->TaskList[i].InitFunc != NULL)
        {
            hscheduler->TaskList[i].InitFunc();
        }
    }


    while ((x*hscheduler->BaseTick) < hscheduler->timeout)
    {
        ActualTick = milliseconds();
        // Preguntamos si han pasado 500ms desde que obtuvimos el anterior valor
        if ((ActualTick - LastTick) >= hscheduler->BaseTick)
        {
            //Enter interruption
            for (int i = 0; i < hscheduler->tasksNum; i++)
            {
                if ((hscheduler->TaskList[i].elapsedTime >= hscheduler->TaskList[i].period) && (hscheduler->TaskList[i].stop == 0))
                {
                    //Execute task function
                    hscheduler->TaskList[i].TickFct();
                    hscheduler->TaskList[i].elapsedTime = 0;
                }
                hscheduler->TaskList[i].elapsedTime += hscheduler->BaseTick;
                
            }

            ///------------------------------------------------------------
            LastTick = milliseconds();//volvemos a obtener los ms actuales
            x++;
        }
    }

}

// ************************ Main ************************
int main(void) {

    //Init Scheduler with values defined in macros
    Scheduler_Init(&scheduler);

    unsigned char TaskID1;
    unsigned char TaskID2;
   
    //register two task with thier corresponding init fucntions and their periodicyt, 100ms and 200ms
    TaskID1 = Scheduler_RegisterTask(&scheduler, Init_200ms, Task_200ms, P_TASK1);
    TaskID2 = Scheduler_RegisterTask(&scheduler, Init_1000ms, Task_1000ms, P_TASK2);
    
    //run the scheduler for the mount of time stablished in Sche.timeout
    Scheduler_Start(&scheduler);

    return 0; 
    
}

