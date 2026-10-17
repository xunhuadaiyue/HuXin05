// OBJ.cpp: implementation of the OBJ class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "OBJ.h"
#include "glm.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

OBJ::OBJ()
{
	this->Obj_model = NULL;
}

OBJ::~OBJ()
{
	if(this->Obj_model != NULL)
		glmDelete(Obj_model);
}

void OBJ::readOBJ(char *filename)
{
	if(this->Obj_model!=NULL)
		glmDelete(this->Obj_model);
	
	
	this->Obj_model=glmReadOBJ(filename);
	glmUnitize(this->Obj_model);
	glmFacetNormals(this->Obj_model);
	glmVertexNormals(this->Obj_model, 90);	
	
  
}

void OBJ::DrawOBJ()
{
	glmDraw(this->Obj_model,GLM_SMOOTH | GLM_COLOR);
}



/////////////////////////////////////////////////////////////////////////////////
























