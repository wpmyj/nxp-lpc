
// SvrChildFrm.h : CSvrChildFrame 类的接口
//

#pragma once

#include "SplitterWndExt.h"
class CSvrChildFrame : public CBCGPMDIChildWnd
{
	DECLARE_DYNCREATE(CSvrChildFrame)
public:
	CSvrChildFrame();

// 属性
public:
	CSplitterWndExt  m_wndSplitter1;
//	CBCGPSplitterWnd  m_wndSplitter2;
// 操作
public:

// 重写
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void ActivateFrame(int nCmdShow);

// 实现
public:
	virtual ~CSvrChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
};
