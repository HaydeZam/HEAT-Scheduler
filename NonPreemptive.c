#include "NonPreemptive.h"

// ************************ Global variables ****************************
static Task_t tasks[TASKS_N];
static Scheduler_t scheduler;
static Timer_t timers[N_TIMERS];
static Scheduler_t Sche;
static unsigned char TimerID;


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
    hscheduler->timers = N_TIMERS;
    hscheduler->timerCount = 0;
    hscheduler->timerPtr = &timers;
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
void Callback(void)
{
    static int loop = 0;
    printf("This is a counter from timer callback: %d\n", loop++);
    Scheduler_StartTimer(&scheduler,TimerID);
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
        // Preguntamos si han pasado basetick desde que obtuvimos el anterior valor
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
            if (hscheduler->timerCount > 0)// There are registered timers
            {
                for (int i = 0; i < hscheduler->timerCount; i++)
                {
                    if (hscheduler->timerPtr[i].StartFlag == TIMER_ON)
                    {
                        hscheduler->timerPtr[i].Count = hscheduler->timerPtr[i].Count - hscheduler->BaseTick;;
                        if (hscheduler->timerPtr[i].Count == 0)
                        {
                            Scheduler_StopTimer(hscheduler, i + 1);
                        }
                    }
                    
                }
            }

            ///------------------------------------------------------------
            LastTick = milliseconds();//volvemos a obtener los ms actuales
            x++;
        }
    }

}

// ---- Timer functions -----

/// <summary>
/// The function will register a new software timer to be updated on every tick, the timer shall count 
/// from a timeout value down to zero, and once its count ended the timer will stop and execute the 
/// function callback registered(the third parameter is optional in case no callback is needed a NULL
/// shall be passed).To successfully register a timer the time in milliseconds should be larger than 
/// the actual tick and multiple.The function returns an ID which is a value from 1 to the number of 
/// timer registers in the scheduler, otherwise, it will return zero indicating the timer couldn't 
/// be registered.
/// </summary>
unsigned char Scheduler_RegisterTimer(Scheduler_t* hscheduler, unsigned long Timeout, void (*CallbackPtr)(void))
{
    unsigned char returnVal = 0;
    if (hscheduler->timerCount < hscheduler->timers) //There's space to register new timer
    {
        if ((Timeout % hscheduler->BaseTick == 0) && (Timeout > hscheduler->BaseTick)) //Time out is valid number
        {
            hscheduler->timerPtr[hscheduler->timerCount].callbackPtr = CallbackPtr;
            hscheduler->timerPtr[hscheduler->timerCount].Timeout = Timeout;
            hscheduler->timerPtr[hscheduler->timerCount].StartFlag = TIMER_OFF;
            hscheduler->timerCount++;
            returnVal = hscheduler->timerCount;

        }
    }
    return returnVal;
}
/// <summary>
/// The function will return the current timer pending time in milliseconds, in case the timer does not 
/// been registered the function returns a zero value
/// </summary>
unsigned long Scheduler_GetTimer(Scheduler_t* hscheduler, unsigned char Timer)
{
    unsigned long returnVal= 0;
    if ((Timer <= hscheduler->timerCount) && (Timer > 0)) // Timer exist
    {
        returnVal = hscheduler->timerPtr[Timer - 1].Count;
    }
    return returnVal;
}
/// <summary>
/// The timer will be reloaded with a new value in milliseconds it will also start the timer,
/// the function does not require the timer to stop first, but if the timer to reload has not
/// been registered no action will be taken and it will return a zero, otherwise return one as
/// an indication of success.
/// </summary>
unsigned char Scheduler_ReloadTimer(Scheduler_t* hscheduler, unsigned char Timer, unsigned long Timeout)
{
    unsigned char returnVal = NOT_REGISTERED;
    if ((Timer <= hscheduler->timerCount) && (Timer > 0)) // Timer exist
    {
        //Update timeout
        hscheduler->timerPtr[Timer - 1].Timeout = Timeout;

        //Reset count
        hscheduler->timerPtr[Timer - 1].Count = Timeout;

        // Start timer
        Scheduler_StartTimer(hscheduler,Timer);

        returnVal = SUCCESS;
    }
    return returnVal;
}
/// <summary>
/// By default the timer count does not start when the timer is registered, it is necessary to call this function,
/// the actual timer decrement count is carried out during each tick occurrence.The function will also serve as a
/// mechanism of restart the timer from its timeout.If the timer to start has not been registered no action will 
/// be taken and it will return a zero, otherwise return one as an indication of success.
/// </summary>
unsigned char Scheduler_StartTimer(Scheduler_t* hscheduler, unsigned char Timer)
{
    unsigned char returnVal = NOT_REGISTERED;

    if ((Timer <= hscheduler->timerCount) && (Timer > 0)) // Timer exist
    {
        //Reset count
        hscheduler->timerPtr[Timer - 1].Count = hscheduler->timerPtr[Timer - 1].Timeout;
        
        //Set start flag
        hscheduler->timerPtr[Timer - 1].StartFlag = TIMER_ON;

        returnVal = SUCCESS;
    }
    return returnVal;
}
/// <summary>
/// The function will indicate the timer should not be decremented during each tick occurrence. 
/// If the timer to stop has not been registered no action will be taken and it will return a zero,
/// otherwise return one as an indication of success.
/// /// </summary>
unsigned char Scheduler_StopTimer(Scheduler_t* hscheduler, unsigned char Timer)
{
    unsigned char returnVal = NOT_REGISTERED;

    if ((Timer <= hscheduler->timerCount) && (Timer > 0)) // Timer exist
    {
        //Turn off start flag
        hscheduler->timerPtr[Timer - 1].StartFlag = TIMER_OFF;
        if (hscheduler->timerPtr[Timer - 1].callbackPtr != NULL)
        {
            hscheduler->timerPtr[Timer - 1].callbackPtr();
        }

        returnVal = SUCCESS;
    }
    return returnVal;
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
    TimerID = Scheduler_RegisterTimer(&scheduler, 800u, Callback);
    Scheduler_StartTimer(&scheduler,TimerID);
    
    //run the scheduler for the mount of time stablished in Sche.timeout
    Scheduler_Start(&scheduler);

    return 0; 
    
}

