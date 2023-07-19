#include <time.h>
#include <stdio.h>


/// ************************ Defines ************************
#define TASKS_N 2
#define TICK_VAL 200
# define TIMEOUT 6000

#define P_TASK1 200
#define P_TASK2 1000

#define TIMESRUN1 7
#define TIMESRUN2 6

#define N_TIMERS 2
#define TIMER_ON 1
#define TIMER_OFF 0

#define SUCCESS 1
#define NOT_REGISTERED 0


// ************************ Typedefs ************************
typedef struct TCB {
    unsigned long period;      // Rate at which the task should tick
    unsigned long elapsedTime; // Time since task's last tick
    char stop;
    void (*TickFct)(void);     // Function to call for task's tick
    void (*InitFunc)(void); //Function to hold the init routine
}Task_t;

typedef struct _Timer_TypeDef
{
    unsigned long Timeout; // timer timeout to decrement and reload when the timer is re-started 
    unsigned long Count; // actual timer decrement count 
    unsigned char StartFlag; //flag to start timer count 
    void(*callbackPtr)(void); //pointer to callback function function 
} Timer_t;

typedef struct Scheduler
{
    unsigned char tasksNum; //the number of tasks to use
    unsigned long BaseTick; //the time base in ms
    unsigned long timeout; //the amount of time the scheduler should run
    unsigned char tasksCount; //internal task counter
    Task_t* TaskList; //the address of the array of task TCBs
    unsigned char timers; //number of software timer to use
    Timer_t* timerPtr; //Pointer to buffer timer 
    unsigned char timerCount; //internal task counter

}Scheduler_t;


// ************************ Function declaration ************************

// ---- Scheduler functions -----
void Scheduler_Init(Scheduler_t* hscheduler);
unsigned char Scheduler_RegisterTask(Scheduler_t* hscheduler, void (*InitPtr)(void), void (*TaskPtr)(void), unsigned long period);
unsigned char Scheduler_StopTask(Scheduler_t* hscheduler, unsigned char task);
unsigned char Scheduler_StartTask(Scheduler_t* hscheduler, unsigned char task);
unsigned char Scheduler_PeriodTask(Scheduler_t* hscheduler, unsigned char task, unsigned long period);
void Init_200ms(void);
void Init_1000ms(void);
void Task_200ms(void);
void Task_1000ms(void);
void Scheduler_Start(Scheduler_t* hscheduler);

// ---- Timer functions -----
unsigned char Scheduler_RegisterTimer(Scheduler_t* hscheduler, unsigned long Timeout, void (*CallbackPtr)(void));
unsigned long Scheduler_GetTimer(Scheduler_t* hscheduler, unsigned char Timer);
unsigned char Scheduler_ReloadTimer(Scheduler_t* hscheduler, unsigned char Timer, unsigned long Timeout);
unsigned char Scheduler_StartTimer(Scheduler_t* hscheduler, unsigned char Timer);
unsigned char Scheduler_StopTimer(Scheduler_t* hscheduler, unsigned char Timer);