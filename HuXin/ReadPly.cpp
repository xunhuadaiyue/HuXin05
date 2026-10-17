#include "pch.h"
#include"ReadPly.h"
//#include <iostream>

#define TRUE  1
#define FALSE 0 


void ReadPlyInCore::read_ply_incore(FILE* stdin_f, RawMesh* rm)
{
	const char *elem_names[] = { /* list of the kinds of elements in the user's object */
  "vertex", "face", "edge"
};

HPLY::PlyProperty vert_props[] = { /* list of property information for a vertex */
	{"x", PLY_FLOAT, PLY_DOUBLE, offsetof(HPLY::PlyVertex,coord[0]), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_DOUBLE, offsetof(HPLY::PlyVertex,coord[1]), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_DOUBLE, offsetof(HPLY::PlyVertex,coord[2]), 0, 0, 0, 0},
  {"face_indices", PLY_INT, PLY_INT, offsetof(HPLY::PlyVertex,faces),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(HPLY::PlyVertex,nfaces)},
  {"edge_indices", PLY_INT, PLY_INT, offsetof(HPLY::PlyVertex,edges),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(HPLY::PlyVertex,nedges)}
};

HPLY::PlyProperty face_props[] = { /* list of property information for a face */
	{"vertex_indices", PLY_INT, PLY_INT, offsetof(HPLY::PlyFace,verts),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(HPLY::PlyFace,nverts)},
  {"edge_indices", PLY_INT, PLY_INT, offsetof(HPLY::PlyFace,edges),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(HPLY::PlyFace,nedges)}
};

const HPLY::PlyProperty edge_props[] = { /* list of property information for an edge */
  {"vert1", PLY_INT, PLY_INT, offsetof(HPLY::PlyEdge,vert1),0,0,0,0},
  {"vert2", PLY_INT, PLY_INT, offsetof(HPLY::PlyEdge,vert2),0,0,0,0},
  {"face1", PLY_INT, PLY_INT, offsetof(HPLY::PlyEdge,face1),0,0,0,0},
  {"face2", PLY_INT, PLY_INT, offsetof(HPLY::PlyEdge,face2),0,0,0,0}
};


const char *type_names[] = {
"invalid",
"char", "short", "int",
"uchar", "ushort", "uint",
"float", "double",
};

const int ply_type_size[] = {
  0, 1, 2, 4, 1, 2, 4, 4, 8
};
 
		//***********************************
	int nverts,nfaces;//,nedges;
	//PlyVertex **vlist;
	//PlyFace **flist;
	//PlyEdge **edgelist;
	PlyOtherElems *other_elements = NULL;
	PlyOtherProp *vert_other,*face_other;
	int nelems;
	char **elist;
	int num_comments;
	char **comments;
	int num_obj_info;
	char **obj_info;
	static int file_type;
	 
	unsigned char has_vedges, has_vfaces, has_fverts, has_fedges;
	//unsigned char has_vert1, has_vert2, has_face1, has_face2;
	unsigned char has_x, has_y, has_z;
		//*********************************
 
	float s;
	int i,j,k;
  	PlyFile *ply;
  	int nprops;
  	int num_elems;
  	PlyProperty **plist;
  	char *elem_name;
  	float version;
	PlyVertex vert_temp;
/*
  	FILE *p_out=fopen(out_v_file,"w+b"); //输出的vertex文件
	FILE *p_out_a=fopen("vertex.b.asc ","w+t"); //输出的vertex文件,asc
	FILE *fp_f=fopen(out_f_file,"w+b");//输出的face文件
	FILE *fp_f_a=fopen("faces.b.asc","w+t");
	FILE *fp_v;
*/
#if 0
  	FILE *p_out_a=fopen("vertex.b.asc ","w+t"); //输出的vertex文件,asc
	FILE *fp_f_a=fopen("faces.b.asc","w+t");
#endif/*** Read in the original PLY object ***/ 
 
 
//*****************************
 
//+++++++++++++++++++++++++++++++
 
  	ply  = ply_read (stdin_f, &nelems, &elist);
  	
  	
  	ply_get_info (ply, &version, &file_type);
  	
  		
  	
  	for (i = 0; i < nelems; i++) 
  	{
  	
  	  	/* get the description of the first element */
  	  	elem_name = elist[i];
  	  	plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);
  	  	
  	  	if (equal_strings ("vertex", elem_name))
		{
  	
  	    	/* create a vertex list to hold all the vertices */
  	    	//ALLOCN(vlist, PlyVertex*, num_elems);
  	    	nverts = num_elems;
				
  	    	/* set up for getting vertex elements */
  	    	has_x = has_y = has_z = has_vfaces = has_vedges = FALSE;;
  	    	
  	    	for (j=0; j<nprops; j++)
  	    	{
				if (equal_strings("x", plist[j]->name))
				{
					ply_get_property (ply, elem_name, &vert_props[0]);  // x 
					has_x = TRUE;
				}
				else if (equal_strings("y", plist[j]->name))
				{
					ply_get_property (ply, elem_name, &vert_props[1]);  // y 
					has_y = TRUE;
				}
				else if (equal_strings("z", plist[j]->name))
				{
					ply_get_property (ply, elem_name, &vert_props[2]);  // z 
					has_z = TRUE;
				}
				if (equal_strings("face_indices", plist[j]->name))
				{
					ply_get_property (ply, elem_name, &vert_props[3]);
					has_vfaces = TRUE;
				}
				else if (equal_strings("edge_indices", plist[j]->name))
				{
					ply_get_property (ply, elem_name, &vert_props[4]);
					has_vedges = TRUE;
				}
      		}
      		vert_other = ply_get_other_properties (ply, elem_name,
								     offsetof(PlyVertex,other_props));
      		
      		//test for necessary properties 
      		if ((has_vfaces) || (has_vedges))
      		{
				fprintf(stderr, "Vertices already have face or edge indices\n");
				exit(12);
      		}
      		if (!((has_x) && (has_y) && (has_z)))
      		{
				fprintf(stderr, "Vertices must have x, y, and z coordinates\n");
				exit(12);
      		}
      		
      		
      		// grab all the vertex elements 
      		for (j = 0; j < num_elems; j++) 
			{
				ply_get_element(ply, (void *)&vert_temp);
				
				for(k=0;k<3;k++)
				{
					s=(float)vert_temp.coord[k];
					rm->vlist[j].coord[k]=s;
					//fwrite(&s,sizeof(float),1,p_out);
				}
			
				//fwrite(&j, sizeof(int),1,p_out);
#if 0				
				fprintf(p_out_a,"%f  %f  %f  %d\n", vert_temp.coord[0], vert_temp.coord[1], vert_temp.coord[2], j);
#endif        	
			}
#if 0		  	//fclose(p_out);
		  	fclose(p_out_a);
#endif
		}
    	else if (equal_strings ("face", elem_name)) 
    	{
 
      		/* create a list to hold all the face elements */
      		//ALLOCN(flist, PlyFace *, num_elems);
      		nfaces = num_elems;
      		
      		/* set up for getting face elements */
      		has_fverts = has_fedges = FALSE;
      		
      		for (j=0; j<nprops; j++)
      		{
			  	if (equal_strings("vertex_indices", plist[j]->name))
			  	{
			  	    ply_get_property (ply, elem_name, &face_props[0]);
			  	    has_fverts = TRUE;
			  	}
			  	else if (equal_strings("edge_indices", plist[j]->name))
			  	{
			  	    ply_get_property (ply, elem_name, &face_props[1]);
			  	    has_fedges = TRUE;
				}
      		}
      		face_other = ply_get_other_properties (ply, elem_name,
							     offsetof(PlyFace,other_props));
        	
      		/* test for necessary properties */
      		if (!has_fverts)
      		{
						fprintf(stderr,"PlyFaces must have vertex indices\n");
						exit(12);
      		}
      		if (has_fedges)
      		{
				fprintf(stderr,"PlyFaces already have edge indices\n");
				exit(12);
      		}
        	
      		/* grab all the face elements */
      		//fp_v=fopen(out_v_file,"rb");
			for (j = 0; j < num_elems; j++) 
			{
				PlyFace temp_f;
        		ply_get_element (ply, (void *)(&temp_f));
				
				
				//s=smallest( (temp_f.verts), ply, fp_v);//从三个顶点指标找到顶点，求最小的横坐标
				//fwrite(&s,sizeof(float),1,fp_f);
				//fwrite( (void*)temp_f.verts, 3*sizeof(int),1,fp_f);
				memcpy( rm->flist[j].v,  (void*)temp_f.verts, 3*sizeof(int));
#if 0
				fprintf(fp_f_a,"%d  %d  %d\n", temp_f.verts[0], temp_f.verts[1], temp_f.verts[2]);
#endif
				free(temp_f.verts);temp_f.verts=NULL;
			}
			 // fclose(fp_f);
#if 0
			  fclose(fp_f_a);
#endif		 // fclose(fp_v);
    	}
    	else if (equal_strings ("edge", elem_name)) 
    	{
    	
			fprintf(stderr, "Plyfile already has edges\n");
			exit(12);
			
    	}
    	else
    		other_elements = ply_get_other_element (ply, elem_name, num_elems);
  	}
  
  	comments = ply_get_comments (ply, &num_comments);
  	obj_info = ply_get_obj_info (ply, &num_obj_info);
 
  	//*******************************
  	//HJM: for debug 
  
  	// for(i=0;i<nfaces;i++)
	//  {if(i%10==0)fprintf(stderr," flist[%d]->: nverts=%d\tnedges=%d\tplane_eq[0]=%d\n",i,flist[i]->nverts,flist[i]->nedges,flist[i]->plane_eq[0]);}
	/*
  	for(i=0;i<nverts;i++)
  	{
		  if(1)//i<100)
			  printf("vlist[%d]->: coord(0)=%f\tcoord(1)=%f\tcoord(2)=%f\n",i,vlist[i]->coord[0],vlist[i]->coord[1],vlist[i]->coord[2]);
		  else break;
  	}
	 
  	for(i=0;i<nfaces;i++)
  	{
		  if(1)//i<100)
			  printf("flist[%d]->: vert(0)=%d\tvert(1)=%d\tvert(2)=%d\n",i,flist[i]->verts[0],flist[i]->verts[1],flist[i]->verts[2]);
		  else break;
  	}
 
	*/ //**************************************
  	ply_close (ply);
  	//see_f(1);
  	//exit(3);
 	// exit(3);//HJM: for debug
}