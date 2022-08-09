#pragma once
#ifndef STRUCTS_H
#define STRUCTS_H

#ifndef BASE_H
#include "Base.h"
#endif

class s_Node {
public:
	s_Node();
	~s_Node();

	virtual unsigned char init(int nNodes);
	virtual void release();
	unsigned char genSubNodes();
	void          releaseSubNodes();
	virtual void reset();

	virtual s_Node& operator=(const s_Node& other);/*the node pointers from nodes are copied but not owned by the copy
										   the size of nodes in memory must be equal for 
										   the node pointers to be copied */

	float x;
	float y;

	long thislink;
	s_Node** nodes;/*lower nodes*/
	int N;

	float o;/*used as colset flag for hex nodes*/

protected:


	int N_mem;/*lenght of nodes pointer array in memory*/
};

class s_Hex : public s_Node {
public:
	s_Hex();
	~s_Hex();

	unsigned char init();
	void          release();
	s_Hex& operator=(const s_Hex& other);

	long i;
	long j;

	s_Node*  web[6];/*web will always be initialized to 6 pointers*/

	float rgb[3];
};

class s_nNode : public s_Node {/*nnet node*/
public:
	s_nNode();
	~s_nNode();

	unsigned char init(int nNodes);
	void          release();
	void          reset();
	s_nNode& operator=(const s_nNode& other);

	s_Hex* hex;/*hex node this NNet node  is attached to if such is linked*/

	float* w;/*w will have the length of nodes, N */
	float b;
};

class s_Plate {
public:
	s_Plate();
	~s_Plate();

	virtual unsigned char init(long nNodes);
	virtual void          release();/*assumes that the plate owns its subnodes if not NULL*/

	inline virtual void set(long indx, s_Node* nd) { this->nodes[indx] = nd; }
	inline virtual s_Node* get(long indx) { return this->nodes[indx]; }

	s_Node** nodes;
	long    N;
protected:
	long N_mem;
	virtual void reset();
};
class s_HexPlate : public s_Plate{
public:
	s_HexPlate();
	~s_HexPlate();

	virtual unsigned char init(long nNodes);
	virtual void          release();/*assumes that the plate owns its subnodes if not NULL*/

	inline void set(long indx, s_Hex* nd) { this->nodes[indx] = (s_Node*)nd; }
	inline void set(long indx, s_Hex& nd) { this->set(indx,&nd); }
	inline s_Hex* get(long indx) { return (s_Hex*)this->nodes[indx]; }

	long height;
	long width;
	float Rhex;
	float RShex;
	float Shex;
	s_2pt hexU[6];
protected:
	virtual void reset();
};
class s_HexBasePlate : public s_HexPlate {
public:
	s_HexBasePlate();
	~s_HexBasePlate();

	unsigned char initRowStart(long rowN);
	void          releaseRowStart();

	long xyToHexi(const s_2pt& xy);
	/*for fast scan xy to plate loc for square plate configuration*/
	s_2pt* RowStart; /*xy position of the first hex in each row*/
	s_2pt_i* RowStart_is;/* index in the hex array of the row start, and number in row*/
	long   Row_N;
	float  Col_d;/*seperation between columns 2/3 * R */
	float  Row_d;/*seperation between rows 2*RS */
protected:
	void reset();
};

class s_HexPlateLayer {
	s_HexPlate** p;
	int N;
protected:
	int N_mem;
};
#endif