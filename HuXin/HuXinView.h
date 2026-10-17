
// HuXinView.h: CHuXinView 类的接口
//

#pragma once
#include "OBJ.h"
#include "math.h"
#include "ReadPly.h"
#include <vector>

struct FFDControlPoint { GLfloat x, y, z; };

class CHuXinView : public CView
{
protected: // 仅从序列化创建
	CHuXinView() noexcept;
	DECLARE_DYNCREATE(CHuXinView)

	int m_GLPixelIndex;
	HGLRC m_hGLContext;

	OBJ  m_obj;
	int  m_read; //1-obj, 2-ply
	RawMesh rw;

	float m_xrotation;
	float m_yrotation;

	BOOL m_leftButtonDown;
	CPoint m_leftDownPos;


	//撤销
	GLuint   numvertices;			
	GLfloat* vertices;			
	BOOL BeRevoked;

	// 交互式变形：选择菜单命令后按住鼠标左键拖动
	enum DeformMode { DEFORM_NONE = 0, DEFORM_SQUASH_STRETCH, DEFORM_FFD };
	DeformMode m_deformMode;
	CPoint m_deformStart;
	std::vector<GLfloat> m_deformOriginal;
	bool m_deformDragging;
	// 3x3x3 FFD 控制笼；索引为 (i*3+j)*3+k
	std::vector<FFDControlPoint> m_ffdPoints;
	int m_ffdSelectedPoint;
	GLfloat m_ffdSelectedDepth;
	GLfloat m_ffdMinX, m_ffdMaxX, m_ffdMinY, m_ffdMaxY, m_ffdMinZ, m_ffdMaxZ;
	void InitFFDControlCage();
	void DrawFFDControlCage();
	int PickFFDControlPoint(CPoint point, GLfloat* depthOut = NULL);
	void DragFFDControlPoint(CPoint point);
	void SaveDeformSnapshot();
	void ApplySquashStretch(float amount);
	void ApplyFFD(float amount);


// 特性
public:
	CHuXinDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	BOOL SetWindowPixelFormat(HDC hDC);
	BOOL CreateViewGLContext(HDC hDC);


// 实现
public:
	virtual ~CHuXinView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFileOpen();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnAnimationTwist();
	afx_msg void OnAnimationTaper();
	afx_msg void OnAnimationBend();
	afx_msg void OnBerevoked();
	afx_msg void OnAnimationFfd();
	afx_msg void OnAnimationSquashstretch();
};

#ifndef _DEBUG  // HuXinView.cpp 中的调试版本
inline CHuXinDoc* CHuXinView::GetDocument() const
   { return reinterpret_cast<CHuXinDoc*>(m_pDocument); }
#endif

