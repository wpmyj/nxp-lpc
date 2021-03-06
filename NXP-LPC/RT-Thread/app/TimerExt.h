/****************************************Copyright (c)**************************************************
**                                        
**                                      
**                                      
**
**                           
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名:  
**创   建   人:  
**最后修改日期: 
**描        述:   软件定时器,软时钟的处理，系统时间处理
**备        注 : 软件定时器最好不要用在循环中
**--------------历史版本信息----------------------------------------------------------------------------
** 创建人:  
** 版  本:  
** 日　期:  
** 描　述: 
**
**--------------当前版本修订------------------------------------------------------------------------------
** 修改人:
** 日　期:
** 描　述:  
**
**------------------------------------------------------------------------------------------------------
********************************************************************************************************/
#pragma once


#if !defined(TIMER_MGR_H)
#define TIMER_MGR_H

 
 


struct EXT_TIMER
{
	INT32U Count;		//时间计数器
	INT32U TimeOut;		//触发点
	INT8U  Enable; 	    //使能标志
	INT8U  Flag ;       //是否已经超时标志
} ;

typedef struct EXT_TIMER TimerExt;

void InitTimerMgr( void );

//使用接口  nTimerID 从1开始，必须 <= 32

//设置定时器
 
INT32U  SetTimer  (INT32U nTimerID, INT32U nTime);
#endif
// 关定时器
INT8U   KillTimer (INT32U nTimerID);

INT8U ResetTimer(INT32U nTimerID);

//
INT8U IsTimeTo(INT32U TimerID ); 
//在TICK中断里面调用
void   ExtTimerTick( void );


#endif 
