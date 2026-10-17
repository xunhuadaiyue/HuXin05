#ifndef _READPLYINCORE_H_
#define _READPLYINCORE_H_

#include "hply.h"
#include "RawMesh.h"

//#pragma comment (lib, "EsortHPly.lib") 

class ReadPlyInCore : public HPLY
{
	//RawMesh* rm;
public:
	void read_ply_incore(FILE* stdin_f, RawMesh* rm);
	//ReadPlyInCore(RawMesh* rm1):rm(rm1){}

};
#endif
