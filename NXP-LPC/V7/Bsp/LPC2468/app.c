/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2006; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                             EXAMPLE CODE
*
*                                             NXP LPC2468
*                                                on the
*                                  Embedded Artists LPC2468 OEM Board
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : Brian Nagel
*********************************************************************************************************
*/

#include <includes.h>
#include <LPC24xx.H>
#include "smartarm2300.h"
#include "TimerExt.h"
//#include "LedMsgMgr.h"
#include "heap_mem.h"
#include "Blinky.h"

/*
*********************************************************************************************************
*                                           #DEFINE CONSTANTS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_STK          AppTaskStartStk[APP_TASK_START_STK_SIZE];

/*
*********************************************************************************************************
*                                         LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void            AppTaskStart                (void *p_arg); 



/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary 68HC12 and C initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

int  main (void)
{
    CPU_INT08U  err;


    BSP_IntDisAll();                                            /* Disable all interrupts until we are ready to accept them */

    OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel"              */

    OSTaskCreateExt(AppTaskStart,                               /* Create the start task                                    */
                    (void *)0,
                    (OS_STK *)&AppTaskStartStk[APP_TASK_START_STK_SIZE - 1],
                    APP_TASK_START_PRIO,
                    APP_TASK_START_PRIO,
                    (OS_STK *)&AppTaskStartStk[0],
                    APP_TASK_START_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#if OS_TASK_NAME_SIZE > 13
    OSTaskNameSet(APP_TASK_START_PRIO, "Start Task", &err);
#endif

    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II)       */
}
 

 
 
/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppStartTask()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#else
extern int __bss_end;
#endif

static  void  AppTaskStart (void *p_arg)
{
    void           *msg;
	INT8U   nLedState = 0;
	INT16U nTimerID = 0;
    

    (void)p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                                 */

#ifdef __CC_ARM
// 	rt_system_heap_init((void*)(&Image$$RW_IRAM1$$ZI$$Limit ), (void*)0x40010000) ;
#else
	rt_system_heap_init((void*)&__bss_end, (void*)0x40010000);
#endif


#if OS_TASK_STAT_EN > 0
    OSStatInit();                                               /* Determine CPU capacity                                   */
#endif

	eth_system_device_init();
//	OSTimeDly(10) ; 
	rt_hw_eth_init();
//	OSTimeDlyHMSM(0,0,10,0);
	InitTimerMgr(  );
//	InitLedMsgMgr( );
//	nTimerID = SetTimer(nTimerID,500);
	InitLed();
	SetLedBlinking(0,5,5);
    
	while (DEF_TRUE)
	{ 
		OSTimeDly(100) ; 
	}
} 