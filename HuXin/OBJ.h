// OBJ.h: interface for the OBJ class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBJ_H__77F851CA_6E73_45CB_9F02_8472901AB536__INCLUDED_)
#define AFX_OBJ_H__77F851CA_6E73_45CB_9F02_8472901AB536__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "glm.h"

class OBJ  
{
public:
	OBJ();
	virtual ~OBJ();
	void readOBJ(char * filename);
    void DrawOBJ();

public:
	GLMmodel* Obj_model;
};

#endif // !defined(AFX_OBJ_H__77F851CA_6E73_45CB_9F02_8472901AB536__INCLUDED_)
