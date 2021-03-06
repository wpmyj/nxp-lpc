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
**描        述:  
**
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
#ifndef GLOBAL_DEF_H
#define	GLOBAL_DEF_H

#include "LcdDriver.h"

//坐标位置
#ifndef LcdPos
#	define LcdPos INT16U
#endif

//定义颜色
#ifndef COLOR
#	define COLOR INT8U
#endif

//定义画笔
#ifndef PEN
#	define PEN INT8U
#endif

#ifndef ALIGN
#	define ALIGN INT8U
#endif

//画笔颜色
typedef enum GUI_COLOR
{
	COLOR_BK	=0,		//背景色
	COLOR_FR	=1		//前景色
}GUI_COLOR;

// 画笔模式
typedef enum GUI_PEN
{
	PEN_COPY,		// 直接copy样子
	PEN_NOT,		// NOT
	PEN_OR,			// OR
	PEN_XOR,        // XOR
	PEN_AND			//AND
}GUI_PEN;

// 对齐方式
typedef enum GUI_ALIGN
{
	ALIGN_LEFT,     //左对齐
	ALIGN_CENTER,   //居中对齐
	ALIGN_RIGHT     //右对齐
}GUI_ALIGN;


//定义光标位置数据数据结构
typedef struct LCD_POS
{
	LcdPos Row;			//行 
	LcdPos Col;			//列
} LCD_POS;


typedef enum LCD_EN_FONT
{
	LCD_EN_FONT_8_8,     // 英文字符大小8×8
	LCD_EN_FONT_8_16     // 英文字符大小8×16
}LCD_EN_FONT;


//根据 X 坐标位置，计算出缓冲区相应的索引
#define GetXIndex(x) (x)
//Y的索引
#define GetYIndex(y)  (((LcdPos)(y))>>3)
// Y点在八位中的位
#define GetYBitPos(y) (((INT8U)(y)) & 0x07)
// Y点在八位中的Mask 0x10形式
#define GetYBitMask(y) (1<<GetYBitPos(y))

/******************
1. 由于液晶的编排方式的列方式，如下图所示
|||||||||||...
|||||||||||...
|||||||||||...
|||||||||||...
122 * 32点阵的液晶，按列划分，上图一个竖线表示一个字节，控制时是以这个字节为对象的。
因此字节的划分为122*4个字节
*******************/
//计算显示缓冲区长宽
#define GetLCDBufferWidth(ignore) LCD_WIDTH
#define GetLCDBufferHeight(ignore) (LCD_HEIGHT>>3)  // (32/8)=4

//得到液晶的长宽
#define GetLCDWidth(ignore) LCD_WIDTH
#define GetLCDHeight(ignore) LCD_HEIGHT

//定义字符串最大长度
#define STRING_MAXLEN 256

//汉字库索引启示地址
#define CNZI_FIRST_INDEX 10000


typedef INT8U LcdBuffer[GetLCDBufferWidth()][GetLCDBufferHeight()];

typedef struct tagLCDRECT
{
	LcdPos    left;
	LcdPos    top;
	LcdPos    right;
	LcdPos    bottom;
} LCDRECT, *LPLCDRECT;

typedef const LCDRECT* LPCLCDRECT;    // pointer to read/only RECT

typedef struct tagLCDPOINT
{
	LcdPos  x;
	LcdPos  y;
} LCDPOINT, *LPLCDPOINT;

typedef const LCDPOINT* LPCLCDPOINT;

typedef struct tagLCDSIZE
{
	LcdPos     cx;
	LcdPos     cy;
} LCDSIZE, *LPLCDSIZE;

typedef const LCDSIZE* LPCLCDSIZE;
//
extern void NormalizeRect(LPLCDRECT  pRect);

//定义光标位置数据数据结构
typedef struct CLcdCursor
{
	LcdPos Row;			//行 
	LcdPos Col;			//列
} CLcdCursor;


//定义菜单链接
typedef struct  CMenuData
{
	INT16U		nMenuID;          //菜单ID号
	INT16U		nResourceID;      //菜单项名字
	INT32U		nItemData;		  //菜单附加项
}CMenuData;

//按键处理
typedef  void  (*fnOnKeyDown)( INT16U keyCode);
//消息处理虚函数
typedef  void   (*fnOnMessage)(INT32U msg, INT32U param);
//初始化窗口
typedef  void    (*fnInit)();
//退出时处理
typedef  void	(*fnDestory)();
//消息通知
typedef  void	(*fnOnNotify)(INT32U nf);


typedef  void    (*fnCreate)(void);



extern const INT32U TenMultiple[];
 

#endif

