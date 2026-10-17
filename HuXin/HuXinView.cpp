
// HuXinView.cpp: CHuXinView 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "HuXin.h"
#endif

#include "HuXinDoc.h"
#include "HuXinView.h"

#include <math.h>
#include <GL/glu.h>
#pragma comment(lib, "glu32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHuXinView

IMPLEMENT_DYNCREATE(CHuXinView, CView)

BEGIN_MESSAGE_MAP(CHuXinView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_FILE_OPEN, &CHuXinView::OnFileOpen)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_ANIMATION_TWIST, &CHuXinView::OnAnimationTwist)
	ON_COMMAND(ID_ANIMATION_TAPER, &CHuXinView::OnAnimationTaper)
	ON_COMMAND(ID_ANIMATION_BEND, &CHuXinView::OnAnimationBend)
	ON_COMMAND(ID_BeRevoked, &CHuXinView::OnBerevoked)
	ON_COMMAND(ID_ANIMATION_FFD, &CHuXinView::OnAnimationFfd)
	ON_COMMAND(ID_ANIMATION_SQUASHSTRETCH, &CHuXinView::OnAnimationSquashstretch)
END_MESSAGE_MAP()

// CHuXinView 构造/析构

CHuXinView::CHuXinView() noexcept
{
	// TODO: 在此处添加构造代码
	this->m_GLPixelIndex = 0;
	this->m_hGLContext = NULL;

	m_read = 0;

	m_xrotation = 0.0;
	m_yrotation = 0.0;

	m_leftButtonDown = FALSE;

	numvertices = 0;			/* number of vertices in model */
	vertices = NULL;			/* array of vertices  */
	BeRevoked = FALSE;
	m_deformMode = DEFORM_NONE;
	m_deformDragging = false;
	m_ffdSelectedPoint = -1;
	m_ffdSelectedDepth = 0.5f;

}

CHuXinView::~CHuXinView()
{
}

BOOL CHuXinView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);    //窗口风格


	return CView::PreCreateWindow(cs);
}

// CHuXinView 绘图

void CHuXinView::OnDraw(CDC* pDC)
{
	CHuXinDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(m_xrotation, 1.0, 0.0, 0.0);
	glRotatef(m_yrotation, 0.0, 1.0, 0.0);


	if (m_read == 1)
		m_obj.DrawOBJ();
	else if (m_read == 2)
		rw.DrawObject();

	if (m_deformMode == DEFORM_FFD && !m_ffdPoints.empty())
		DrawFFDControlCage();

	SwapBuffers(pDC->m_hDC);
}


// CHuXinView 打印

BOOL CHuXinView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CHuXinView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CHuXinView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

BOOL CHuXinView::SetWindowPixelFormat(HDC hDC)
{
	PIXELFORMATDESCRIPTOR  pixelDesc = { sizeof(PIXELFORMATDESCRIPTOR), 1,
																			   PFD_DRAW_TO_WINDOW |
																			   PFD_SUPPORT_OPENGL |
																			   PFD_DOUBLEBUFFER,
																			   PFD_TYPE_RGBA,   24,  0,0,0,0,0,0,  0,  0,  0,
																			   0,0,0,0,  32,   0,  0, 0,  0,  0,0,0 };
	m_GLPixelIndex = ChoosePixelFormat(hDC, &pixelDesc);

	if (m_GLPixelIndex == 0)
	{
		return FALSE;
	}

	if (SetPixelFormat(hDC, m_GLPixelIndex, &pixelDesc) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CHuXinView::CreateViewGLContext(HDC hDC)
{
	m_hGLContext = wglCreateContext(hDC);
	if (m_hGLContext == NULL) //创建失败
	{
		return FALSE;
	}

	if (wglMakeCurrent(hDC, m_hGLContext) == FALSE)
	{//选为当前RC失败
		return FALSE;
	}
	return TRUE;
}


// CHuXinView 诊断

#ifdef _DEBUG
void CHuXinView::AssertValid() const
{
	CView::AssertValid();
}

void CHuXinView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CHuXinDoc* CHuXinView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHuXinDoc)));
	return (CHuXinDoc*)m_pDocument;
}
#endif //_DEBUG


// CHuXinView 消息处理程序

int CHuXinView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	HWND hWnd = this->GetSafeHwnd();
	HDC hDC = ::GetDC(hWnd);

	if (this->SetWindowPixelFormat(hDC) == FALSE)
	{
		return 0;
	}

	if (this->CreateViewGLContext(hDC) == FALSE)
	{
		return 0;
	}


	//////////////
	glewInit();
	glDrawBuffer(GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	/////////////

	/////////////////////
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	float light_ambient[] = { 0.1,0.1,0.1,1 };
	float light_diffuse[] = { 1,1,1,1 };
	float light_specular[] = { 1,1,1,1 };
	float light_position[] = { 1,1,1,0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	////////////////////
	 
	return 0;
}

void CHuXinView::OnDestroy()
{
	CView::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	if (wglGetCurrentContext() != NULL)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (this->m_hGLContext != NULL)
	{
		wglDeleteContext(this->m_hGLContext);
		this->m_hGLContext = NULL;
	}
	delete[]vertices;
}

void CHuXinView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	glViewport(0, 0, cx, cy);
	GLdouble aspect;
	if (cy == 0)
	{
		aspect = (GLdouble)cx;
	}
	else
	{
		aspect = (GLdouble)cx / (GLdouble)cy;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-3.0 * aspect, 3.0 * aspect, -3.0, 3.0, -13.0, 13.0);
	Invalidate();

}

BOOL CHuXinView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	//return CView::OnEraseBkgnd(pDC);

	return TRUE;
}

void CHuXinView::OnFileOpen()
{
	// TODO: 在此添加命令处理程序代码

	CString filename;  TCHAR strFilter[] = _T("OBJ file(*.obj)|*.obj|ply file(*.ply)|*.ply||");
	CFileDialog fDlg(TRUE, _T("OBJ"), _T("*.obj"), OFN_HIDEREADONLY |
		OFN_OVERWRITEPROMPT, strFilter);
	TCHAR strTitle[] = _T("Open Model file...");
	fDlg.m_ofn.lpstrTitle = strTitle;

	if (fDlg.DoModal() == IDOK)
	{
		filename = fDlg.GetPathName();

		if (filename.Right(3) == "OBJ" || filename.Right(3) == "obj")
		{
			//获取缓冲区的大小，并申请空间，缓冲区大小是按字节计算的  
			int len = WideCharToMultiByte(CP_ACP, 0, filename.GetBuffer(filename.GetLength()),
				filename.GetLength(), NULL, 0, NULL, NULL);
			char* buffer = new char[len + 1];
			WideCharToMultiByte(CP_ACP, 0, filename.GetBuffer(filename.GetLength()),
				filename.GetLength(), buffer, len, NULL, NULL);
			buffer[len] = '\0';
			m_obj.readOBJ(buffer);
			delete[]buffer;
			m_read = 1;

			///////////////////////////////////////
			delete[]vertices;
			numvertices = m_obj.Obj_model->numvertices;
			vertices = new GLfloat[numvertices * 3 + 3];
			BeRevoked = FALSE;


		}
		
		else if (filename.Right(3) == "PLY" || filename.Right(3) == "ply") 
		{
			//获取缓冲区的大小，并申请空间，缓冲区大小是按字节计算的  
			int len = WideCharToMultiByte(CP_ACP, 0, filename.GetBuffer(filename.GetLength()), filename.GetLength(), NULL, 0, NULL, NULL);
			char* buffer = new char[len + 1];
			WideCharToMultiByte(CP_ACP, 0, filename.GetBuffer(filename.GetLength()), filename.GetLength(), buffer, len, NULL, NULL);
			buffer[len] = '\0';

			/////////////////////////////////////////
			int nvf[2];
			ReadPlyInCore rpic;

			rpic.get_nverts_nfaces(buffer, nvf);  //取得模型的顶点数、面数
			rw.init(nvf[0], nvf[1]);

			FILE* infile = fopen(buffer, "rb");
			if (!infile)
			{
				fprintf(stderr, "Error:  Couldn't open file, %s, for reading\n", buffer);
				exit(0);
			}

			rpic.read_ply_incore(infile, &rw);
			if (infile) fclose(infile);


			rw.bb_center();
			rw.bb_normolize();
			/////////////////////////////////////////

			//删除缓冲区 
			delete[]buffer;
			m_read = 2;

		}
	}

	Invalidate(TRUE);


}

void CHuXinView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_deformMode == DEFORM_FFD && m_read == 1 && m_obj.Obj_model != NULL)
	{
		GLfloat depth = 0.5f;
		int picked = PickFFDControlPoint(point, &depth);
		if (picked >= 0)
		{
			m_ffdSelectedPoint = picked;
			m_ffdSelectedDepth = depth;
			m_deformDragging = true;
			SetCapture();
		}
	}
	else if (m_deformMode == DEFORM_SQUASH_STRETCH && m_read == 1 && m_obj.Obj_model != NULL && m_obj.Obj_model->numvertices > 0)
	{
		SaveDeformSnapshot(); m_deformStart = point; m_deformDragging = true; SetCapture();
	}
	else { m_leftButtonDown = TRUE; m_leftDownPos = point; }
	CView::OnLButtonDown(nFlags, point);
}

void CHuXinView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_deformDragging) { m_deformDragging = false; m_ffdSelectedPoint = -1; ReleaseCapture(); }
	// 注意：松开鼠标只结束当前拖动，不退出 FFD 模式。
	m_leftButtonDown = FALSE;
	CView::OnLButtonUp(nFlags, point);
}

void CHuXinView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_deformDragging && m_deformMode == DEFORM_FFD && m_ffdSelectedPoint >= 0)
	{
		DragFFDControlPoint(point);
		Invalidate(FALSE);
	}
	else if (m_deformDragging && m_deformMode == DEFORM_SQUASH_STRETCH)
	{
		float amount = (float)(m_deformStart.y - point.y) / 150.0f;
		amount = max(-0.75f, min(0.75f, amount));
		ApplySquashStretch(amount);
		Invalidate(FALSE);
	}
	else if (m_leftButtonDown)
	{
		m_yrotation += (point.x - m_leftDownPos.x) / 3.0f;
		m_xrotation += (point.y - m_leftDownPos.y) / 3.0f;
		m_leftDownPos = point; Invalidate(FALSE);
	}
	CView::OnMouseMove(nFlags, point);
}

void CHuXinView::SaveDeformSnapshot()
{
	if (m_read != 1 || m_obj.Obj_model == NULL) return;
	const int n = (int)m_obj.Obj_model->numvertices;
	m_deformOriginal.resize((n + 1) * 3);
	for (int i = 1; i <= n; ++i) for (int k = 0; k < 3; ++k)
		m_deformOriginal[3*i+k] = m_obj.Obj_model->vertices[3*i+k];
	if (vertices == NULL || numvertices != (GLuint)n)
	{
		delete[] vertices; numvertices = (GLuint)n; vertices = new GLfloat[(n + 1) * 3];
	}
	for (int i = 1; i <= n; ++i) for (int k = 0; k < 3; ++k)
		vertices[3*i+k] = m_deformOriginal[3*i+k];
	BeRevoked = TRUE;
}

void CHuXinView::ApplySquashStretch(float amount)
{
	if (m_read != 1 || m_obj.Obj_model == NULL || m_deformOriginal.empty()) return;
	const int n = (int)m_obj.Obj_model->numvertices;
	float minY = m_deformOriginal[4], maxY = minY;
	for (int i = 1; i <= n; ++i) { minY = min(minY, m_deformOriginal[3*i+1]); maxY = max(maxY, m_deformOriginal[3*i+1]); }
	const float centerY = (minY + maxY) * 0.5f;
	// y 缩放 s，x/z 缩放 1/sqrt(s)，缩放矩阵行列式为 1，保持体积不变
	const float scaleY = max(0.25f, min(4.0f, 1.0f + amount));
	const float scaleXZ = 1.0f / sqrtf(scaleY);
	for (int i = 1; i <= n; ++i) {
		float x=m_deformOriginal[3*i], y=m_deformOriginal[3*i+1], z=m_deformOriginal[3*i+2];
		m_obj.Obj_model->vertices[3*i] = x*scaleXZ;
		m_obj.Obj_model->vertices[3*i+1] = centerY+(y-centerY)*scaleY;
		m_obj.Obj_model->vertices[3*i+2] = z*scaleXZ;
	}
}

void CHuXinView::ApplyFFD(float /*amount*/)
{
	// FFD 由 3x3x3 控制笼驱动，顶点始终从未变形的 m_deformOriginal 计算，避免累积误差。
	if (m_read != 1 || m_obj.Obj_model == NULL || m_deformOriginal.empty() || m_ffdPoints.size() != 27) return;
	const int n = (int)m_obj.Obj_model->numvertices;
	for (int v = 1; v <= n; ++v)
	{
		const float x = m_deformOriginal[3*v], y = m_deformOriginal[3*v+1], z = m_deformOriginal[3*v+2];
		float u = (x-m_ffdMinX)/(m_ffdMaxX-m_ffdMinX);
		float w = (y-m_ffdMinY)/(m_ffdMaxY-m_ffdMinY);
		float t = (z-m_ffdMinZ)/(m_ffdMaxZ-m_ffdMinZ);
		u=max(0.0f,min(1.0f,u)); w=max(0.0f,min(1.0f,w)); t=max(0.0f,min(1.0f,t));
		float bu[3]={(1-u)*(1-u),2*u*(1-u),u*u};
		float bw[3]={(1-w)*(1-w),2*w*(1-w),w*w};
		float bt[3]={(1-t)*(1-t),2*t*(1-t),t*t};
		float px=0,py=0,pz=0;
		for(int i=0;i<3;++i) for(int j=0;j<3;++j) for(int k=0;k<3;++k)
		{
			const FFDControlPoint& cp=m_ffdPoints[(i*3+j)*3+k];
			float weight=bu[i]*bw[j]*bt[k];
			px+=cp.x*weight; py+=cp.y*weight; pz+=cp.z*weight;
		}
		m_obj.Obj_model->vertices[3*v]=px;
		m_obj.Obj_model->vertices[3*v+1]=py;
		m_obj.Obj_model->vertices[3*v+2]=pz;
	}
}

void CHuXinView::InitFFDControlCage()
{
	if (m_read != 1 || m_obj.Obj_model == NULL || m_obj.Obj_model->numvertices == 0) return;
	SaveDeformSnapshot();
	const int n=(int)m_obj.Obj_model->numvertices;
	m_ffdMinX=m_deformOriginal[3]; m_ffdMaxX=m_ffdMinX;
	m_ffdMinY=m_deformOriginal[4]; m_ffdMaxY=m_ffdMinY;
	m_ffdMinZ=m_deformOriginal[5]; m_ffdMaxZ=m_ffdMinZ;
	for(int v=1;v<=n;++v) {
		m_ffdMinX=min(m_ffdMinX,m_deformOriginal[3*v]); m_ffdMaxX=max(m_ffdMaxX,m_deformOriginal[3*v]);
		m_ffdMinY=min(m_ffdMinY,m_deformOriginal[3*v+1]); m_ffdMaxY=max(m_ffdMaxY,m_deformOriginal[3*v+1]);
		m_ffdMinZ=min(m_ffdMinZ,m_deformOriginal[3*v+2]); m_ffdMaxZ=max(m_ffdMaxZ,m_deformOriginal[3*v+2]);
	}
	// 防止扁平模型出现除零；控制笼在该轴上仍提供一个很薄的厚度。
	if(m_ffdMaxX-m_ffdMinX<1e-5f){m_ffdMinX-=0.01f;m_ffdMaxX+=0.01f;}
	if(m_ffdMaxY-m_ffdMinY<1e-5f){m_ffdMinY-=0.01f;m_ffdMaxY+=0.01f;}
	if(m_ffdMaxZ-m_ffdMinZ<1e-5f){m_ffdMinZ-=0.01f;m_ffdMaxZ+=0.01f;}
	m_ffdPoints.clear(); m_ffdPoints.reserve(27);
	for(int i=0;i<3;++i) for(int j=0;j<3;++j) for(int k=0;k<3;++k) {
		FFDControlPoint cp;
		cp.x=m_ffdMinX+(m_ffdMaxX-m_ffdMinX)*i*0.5f;
		cp.y=m_ffdMinY+(m_ffdMaxY-m_ffdMinY)*j*0.5f;
		cp.z=m_ffdMinZ+(m_ffdMaxZ-m_ffdMinZ)*k*0.5f;
		m_ffdPoints.push_back(cp);
	}
	m_ffdSelectedPoint=-1;
}

void CHuXinView::DrawFFDControlCage()
{
	GLboolean lighting=glIsEnabled(GL_LIGHTING);
	glDisable(GL_LIGHTING);
	glLineWidth(1.5f);
	glColor3f(1.0f,0.58f,0.08f);
	glBegin(GL_LINES);
	for(int i=0;i<3;++i) for(int j=0;j<3;++j) for(int k=0;k<3;++k) {
		int a=(i*3+j)*3+k;
		if(i<2){int b=((i+1)*3+j)*3+k; glVertex3f(m_ffdPoints[a].x,m_ffdPoints[a].y,m_ffdPoints[a].z);glVertex3f(m_ffdPoints[b].x,m_ffdPoints[b].y,m_ffdPoints[b].z);}
		if(j<2){int b=(i*3+j+1)*3+k; glVertex3f(m_ffdPoints[a].x,m_ffdPoints[a].y,m_ffdPoints[a].z);glVertex3f(m_ffdPoints[b].x,m_ffdPoints[b].y,m_ffdPoints[b].z);}
		if(k<2){int b=(i*3+j)*3+k+1; glVertex3f(m_ffdPoints[a].x,m_ffdPoints[a].y,m_ffdPoints[a].z);glVertex3f(m_ffdPoints[b].x,m_ffdPoints[b].y,m_ffdPoints[b].z);}
	}
	glEnd();
	glPointSize(9.0f);
	glBegin(GL_POINTS);
	for(int i=0;i<27;++i){ if(i==m_ffdSelectedPoint) glColor3f(1.0f,0.1f,0.1f); else glColor3f(1.0f,0.62f,0.08f); glVertex3f(m_ffdPoints[i].x,m_ffdPoints[i].y,m_ffdPoints[i].z); }
	glEnd();
	glPointSize(1.0f); glLineWidth(1.0f);
	if(lighting) glEnable(GL_LIGHTING);
}

int CHuXinView::PickFFDControlPoint(CPoint point, GLfloat* depthOut)
{
	if(m_ffdPoints.size()!=27) return -1;
	GLdouble model[16],proj[16]; GLint viewport[4];
	glGetDoublev(GL_MODELVIEW_MATRIX,model); glGetDoublev(GL_PROJECTION_MATRIX,proj); glGetIntegerv(GL_VIEWPORT,viewport);
	int best=-1; double bestDist=14.0*14.0; GLfloat bestDepth=0.5f;
	for(int i=0;i<27;++i){ GLdouble sx,sy,sz; const FFDControlPoint& cp=m_ffdPoints[i];
		if(gluProject(cp.x,cp.y,cp.z,model,proj,viewport,&sx,&sy,&sz)!=GL_TRUE) continue;
		double dx=sx-point.x, dy=(viewport[3]-sy)-point.y, d=dx*dx+dy*dy;
		if(d<bestDist){bestDist=d;best=i;bestDepth=(GLfloat)sz;}
	}
	if(best>=0 && depthOut) *depthOut=bestDepth;
	return best;
}

void CHuXinView::DragFFDControlPoint(CPoint point)
{
	if(m_ffdSelectedPoint<0 || m_ffdSelectedPoint>=27) return;
	GLdouble model[16],proj[16]; GLint viewport[4];
	glGetDoublev(GL_MODELVIEW_MATRIX,model); glGetDoublev(GL_PROJECTION_MATRIX,proj); glGetIntegerv(GL_VIEWPORT,viewport);
	GLdouble x,y,z;
	if(gluUnProject((GLdouble)point.x,(GLdouble)(viewport[3]-point.y),m_ffdSelectedDepth,model,proj,viewport,&x,&y,&z)==GL_TRUE){
		m_ffdPoints[m_ffdSelectedPoint].x=(GLfloat)x;
		m_ffdPoints[m_ffdSelectedPoint].y=(GLfloat)y;
		m_ffdPoints[m_ffdSelectedPoint].z=(GLfloat)z;
		ApplyFFD(0.0f);
	}
}

void CHuXinView::OnAnimationTwist()//扭曲
{
	// TODO: 在此添加命令处理程序代码
		//计算ymaxy,miny

	for (int i = 1; i <= m_obj.Obj_model->numvertices; i++)
	{
		vertices[3 * i] = m_obj.Obj_model->vertices[3 * i];
		vertices[3 * i + 1] = m_obj.Obj_model->vertices[3 * i + 1];
		vertices[3 * i + 2] = m_obj.Obj_model->vertices[3 * i + 2];

	}

	GLfloat maxy, miny;

	maxy = miny = m_obj.Obj_model->vertices[3 + 1];

	for (int i = 1; i <= m_obj.Obj_model->numvertices; i++)
	{
		if (maxy < m_obj.Obj_model->vertices[3 * i + 1])
			maxy = m_obj.Obj_model->vertices[3 * i + 1];
		if (miny > m_obj.Obj_model->vertices[3 * i + 1])
			miny = m_obj.Obj_model->vertices[3 * i + 1];
	}

	//扭曲
	float rotate_y;

	for (int i = 1; i <= m_obj.Obj_model->numvertices; i++)
	{
		float x = m_obj.Obj_model->vertices[3 * i + 0];
		float y = m_obj.Obj_model->vertices[3 * i + 1];
		float z = m_obj.Obj_model->vertices[3 * i + 2];

		rotate_y = 90 * (y - miny);

		rotate_y *= 3.14 / 180;

		m_obj.Obj_model->vertices[3 * i + 0] = x * cos(rotate_y) - z * sin(rotate_y);
		m_obj.Obj_model->vertices[3 * i + 1] = y;
		m_obj.Obj_model->vertices[3 * i + 2] = x * sin(rotate_y) + z * cos(rotate_y);

	}

	BeRevoked = TRUE;
	//刷新
	Invalidate(TRUE);

}

void CHuXinView::OnAnimationTaper()//锥化
{
	// TODO: 在此添加命令处理程序代码
		//计算ymaxy,miny
	for (int i = 1; i <= m_obj.Obj_model->numvertices; i++)
	{
		vertices[3 * i] = m_obj.Obj_model->vertices[3 * i];
		vertices[3 * i + 1] = m_obj.Obj_model->vertices[3 * i + 1];
		vertices[3 * i + 2] = m_obj.Obj_model->vertices[3 * i + 2];

	}

	GLfloat maxy, miny;

	maxy = miny = m_obj.Obj_model->vertices[3 + 1];

	for (int i = 1; i <= m_obj.Obj_model->numvertices; i++)
	{
		if (maxy < m_obj.Obj_model->vertices[3 * i + 1])
			maxy = m_obj.Obj_model->vertices[3 * i + 1];
		if (miny > m_obj.Obj_model->vertices[3 * i + 1])
			miny = m_obj.Obj_model->vertices[3 * i + 1];
	}

	//锥化
	float scale_y;

	for (int i = 1; i <= m_obj.Obj_model->numvertices; i++)
	{
		float x = m_obj.Obj_model->vertices[3 * i + 0];
		float y = m_obj.Obj_model->vertices[3 * i + 1];
		float z = m_obj.Obj_model->vertices[3 * i + 2];

		scale_y = (maxy - y) / (maxy - miny);

		m_obj.Obj_model->vertices[3 * i + 0] = x * scale_y;
		m_obj.Obj_model->vertices[3 * i + 1] = y;
		m_obj.Obj_model->vertices[3 * i + 2] = z * scale_y;

	}
	BeRevoked = TRUE;

	//刷新
	Invalidate(TRUE);

}

void CHuXinView::OnAnimationBend()//弯曲
{
	// TODO: 在此添加命令处理程序代码
		//计算ymaxy,miny

	for (int i = 1; i <= m_obj.Obj_model->numvertices; i++)
	{
		vertices[3 * i] = m_obj.Obj_model->vertices[3 * i];
		vertices[3 * i + 1] = m_obj.Obj_model->vertices[3 * i + 1];
		vertices[3 * i + 2] = m_obj.Obj_model->vertices[3 * i + 2];

	}

	GLfloat maxy, miny;

	maxy = miny = m_obj.Obj_model->vertices[3 + 1];

	for (int i = 1; i <= m_obj.Obj_model->numvertices; i++)
	{
		if (maxy < m_obj.Obj_model->vertices[3 * i + 1])
			maxy = m_obj.Obj_model->vertices[3 * i + 1];
		if (miny > m_obj.Obj_model->vertices[3 * i + 1])
			miny = m_obj.Obj_model->vertices[3 * i + 1];
	}

	//弯曲
	////弯曲中心
	float bend_center_x = 2.0;


	for (int i = 1; i <= m_obj.Obj_model->numvertices; i++)
	{
		float x = m_obj.Obj_model->vertices[3 * i + 0];
		float y = m_obj.Obj_model->vertices[3 * i + 1];
		float z = m_obj.Obj_model->vertices[3 * i + 2];

		float bend_angle = 90 * (y - miny);
		bend_angle *= 3.14 / 180;
		////////////////////////////////////////////////
		if (y < miny)
		{
			m_obj.Obj_model->vertices[3 * i + 0] = x;
		}
		else if (y > maxy)
		{
			m_obj.Obj_model->vertices[3 * i + 0] = bend_center_x - (bend_center_x - x) * cos(bend_angle) + (y - maxy) * sin(bend_angle);
		}
		else
		{
			m_obj.Obj_model->vertices[3 * i + 0] = bend_center_x - (bend_center_x - x) * cos(bend_angle);
		}
		////////////////////////////////////////////////		
		if (y < miny)
		{
			m_obj.Obj_model->vertices[3 * i + 1] = y;
		}
		else if (y > maxy)
		{
			m_obj.Obj_model->vertices[3 * i + 1] = miny + (bend_center_x - x) * sin(bend_angle) + (y - maxy) * cos(bend_angle);
		}
		else
		{
			m_obj.Obj_model->vertices[3 * i + 1] = miny + (bend_center_x - x) * sin(bend_angle);
		}
		///////////////////////////////////////////////
		m_obj.Obj_model->vertices[3 * i + 2];

	}

	BeRevoked = TRUE;
	//刷新
	Invalidate(TRUE);
}



void CHuXinView::OnBerevoked()
{
	if (BeRevoked != FALSE && m_read == 1 && m_obj.Obj_model != NULL && vertices != NULL)
	{
		for (int i=1; i<=m_obj.Obj_model->numvertices && i<=(int)numvertices; ++i)
			for (int k=0;k<3;++k) m_obj.Obj_model->vertices[3*i+k]=vertices[3*i+k];
		BeRevoked=FALSE; m_deformMode=DEFORM_NONE; m_deformDragging=false;
	}
	Invalidate(FALSE);
}

void CHuXinView::OnAnimationFfd() // FFD 自由变形
{
	if (m_deformMode == DEFORM_FFD) { m_deformMode = DEFORM_NONE; m_deformDragging = false; m_ffdSelectedPoint = -1; if (GetCapture() == this) ReleaseCapture(); Invalidate(FALSE); return; }
	if (m_read != 1 || m_obj.Obj_model == NULL || m_obj.Obj_model->numvertices == 0) {
		AfxMessageBox(_T("请先打开一个 OBJ 模型，再使用 FFD 变形。")); return;
	}
	m_deformMode=DEFORM_FFD;
	InitFFDControlCage();
	Invalidate(FALSE);
	AfxMessageBox(_T("已进入 3×3×3 FFD 模式：拖动橙色控制点进行实时变形；松开鼠标后可继续选择其他控制点。再次点击 FFD 菜单退出。"));
}

void CHuXinView::OnAnimationSquashstretch() // 体积保持的挤压/拉伸
{
	if (m_read != 1 || m_obj.Obj_model == NULL || m_obj.Obj_model->numvertices == 0) {
		AfxMessageBox(_T("请先打开一个 OBJ 模型，再使用挤压/拉伸。")); return;
	}
	m_deformMode=DEFORM_SQUASH_STRETCH;
	AfxMessageBox(_T("已进入挤压/拉伸交互模式：按住鼠标左键上下拖动；松开鼠标结束本次拖动。"));
}
