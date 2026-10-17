
#ifndef _RAWMESH_H_
#define _RAWMESH_H_
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <float.h>
#include<cmath>


//仅包含一列顶点和一列三角形,用于接收ply文件
class RawMesh		
{
	typedef struct Vertex 
	{
		float coord[3];
	}	Vertex;

	typedef struct Face		// 只处理三角网格
	{
		int v[3];		// 三个顶点的指标
	}	Face;
	
	typedef struct Edge
	{
		int vert[2];		// 两个顶点的指标, 由小到大
	}	Edge;

private:
 

public:
	int nverts;
	int nfaces;
	int nedges;
	float bb[6];
	float center[3];

	Vertex *vlist;
	Face   *flist;
	Edge   *elist;
    Vertex *nlist;

	RawMesh()
	{
		vlist=NULL;
		flist=NULL;
		elist=NULL;
		nlist=NULL;
	}
	void init(int num_verts, int num_faces, int num_edges = 0 ) 
	{
		clear_vfe();
		nverts=num_verts;
		nfaces=num_faces;
		nedges=num_edges;
		elist= NULL;
		
		vlist = new Vertex[num_verts];
		flist = new Face[num_faces];
		nlist =new Vertex[num_verts];
		memset(nlist, 0, num_verts*3*sizeof(float));

		if(num_edges > 0)
			elist= new Edge[num_edges];

		if( !vlist || !flist || (num_edges && !elist ) )
		{
			std::cout<<"RawMesh(): memory allocation error!" <<std::endl;
			exit(-1);
		}
	}

	~RawMesh()
	{
		clear_vfe();
	}
	void clear_vfe()	//有时为了减少内存,手工把用不到的空间释放
	{
		if(vlist) 
		{
			delete[] vlist;
			vlist=NULL;
		}
		if(flist) 
		{
			delete[] flist;
			flist=NULL;
		}
		if(elist)
		{
			delete[] elist;
			elist=NULL;
		}
		if(nlist) 
		{
			delete[] nlist;
			nlist=NULL;
		}		
		
	}
	void bb_center()
	{
		bb[1]=bb[3]=bb[5]=FLT_MIN;
		bb[0]=bb[2]=bb[4]=FLT_MAX;
		for(int i=0;i<nverts;i++)
		{	
			
			if(vlist[i].coord[0]<bb[0]) bb[0]=vlist[i].coord[0];	
			if(vlist[i].coord[0]>bb[1]) bb[1]=vlist[i].coord[0];
			if(vlist[i].coord[1]<bb[2]) bb[2]=vlist[i].coord[1];	
			if(vlist[i].coord[1]>bb[3]) bb[3]=vlist[i].coord[1];
			if(vlist[i].coord[2]<bb[4]) bb[4]=vlist[i].coord[2];	
			if(vlist[i].coord[2]>bb[5]) bb[5]=vlist[i].coord[2];
		}
		center[0]=(bb[0]+bb[1])/2.0f;
		center[1]=(bb[2]+bb[3])/2.0f;
		center[2]=(bb[4]+bb[5])/2.0f;
	}
	
	void bb_normolize()
	{
		for(int i=0;i<nverts;i++)
		{
			vlist[i].coord[0]=(vlist[i].coord[0]-center[0])/(0.5*(bb[1]-bb[0]));
			vlist[i].coord[1]=(vlist[i].coord[1]-center[1])/(0.5*(bb[1]-bb[0]));
			vlist[i].coord[2]=(vlist[i].coord[2]-center[2])/(0.5*(bb[1]-bb[0]));
		}
		////////////////////////////
		center[0]=0;
		center[1]=0;
		center[2]=0;

		bb[1]=1;
		bb[0]=-1;
	}

	void unit_vector(float* v, float* n)		//在n中返回单位向量
	{
		float lenth=sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
		n[0]=v[0]/lenth;
		n[1]=v[1]/lenth;
		n[2]=v[2]/lenth;
	}

	void normal_for_v()
	{
		int i,j,k;
		Face f; Vertex v1,v2,v3,norm;
		float u[3],v[3];

	
		for(i=0;i<nfaces;i++)
		{
			f=flist[i]; 
			v1=vlist[f.v[0]];
			v2=vlist[f.v[1]];
			v3=vlist[f.v[2]];

			u[0]=v2.coord[0]-v1.coord[0];
			u[1]=v2.coord[1]-v1.coord[1];
			u[2]=v2.coord[2]-v1.coord[2];
			v[0]=v3.coord[0]-v1.coord[0];
			v[1]=v3.coord[1]-v1.coord[1];
			v[2]=v3.coord[2]-v1.coord[2];
			norm.coord[0]=u[1]*v[2]-u[2]*v[1];
			norm.coord[1]=u[2]*v[0]-u[0]*v[2];
			norm.coord[2]=u[0]*v[1]-u[1]*v[0];

			for (j=0;j<3;j++)
			{
				for(k=0;k<3;k++)
				{
					nlist[f.v[j]].coord[k] += (norm.coord[k]);
					//std::cout<<nlist[f.v[j]].coord[k]<<std::endl;
				}
			}

		}

		for(i=0;i<nverts;i++)
		{
			float v[3];
			for(int j=0;j<3;j++)
				v[j]=nlist[i].coord[j];
			unit_vector(v, nlist[i].coord);
		}
	}

	void DrawObject()
	{
		int i;
		int j;
	
		//一般要求该模型没有normal
		normal_for_v();

		glBegin (GL_TRIANGLES);
		for(i=0;i<nfaces;i++)
		{
			Face f=flist[i]; 
			for(j=0;j<3;j++)
			{
				glNormal3f (nlist[f.v[j]].coord[0],nlist[f.v[j]].coord[1],nlist[f.v[j]].coord[2]);
				glVertex3f (vlist[f.v[j]].coord[0],vlist[f.v[j]].coord[1],vlist[f.v[j]].coord[2]);
			}
		}
		glEnd ();

	}

};
#endif