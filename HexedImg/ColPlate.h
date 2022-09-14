#pragma once
#ifndef COLPLATE_H
#define COLPLATE_H
#ifndef HEXIMG_H
#include "../HexedImg/HexImg.h"
#endif

struct s_ColWheel {
	float pixMax;/* maximum value for the pixels usually 255 for 8 bits*/

	float Dhue; /*distance in cos (needs to be greater than 0) */
	float DI;   /* intensity */
	float DSat; /*  saturation */
	float HueFadeV; /*hue vector length at which the hue is considered to start to fade to a neutral return */
	float I_target;
	s_2pt Hue_target;/*rgb vector in colorwheel*/
	float Sat_target;

	float stepSteepness;

	/*red, green, blue vectors in colwheel space*/
	s_2pt Ur;
	s_2pt Ug; 
	s_2pt Ub;

	float m_DhueRes;/* 2 - m_Dhue */
};

class s_ColPlate : public s_HexBasePlate {
public:
	s_ColPlate();
	~s_ColPlate();

	s_ColWheel Col;
};

class ColPlate : public Base {
public:
	ColPlate();
	~ColPlate();

	unsigned char init();
	void          release();

	unsigned char spawn(
		s_HexBasePlate* hexedImg,
		s_ColPlate* colPlate,
		const s_ColWheel& Col
	);
	void          despawn(s_ColPlate* colPlate);
	
	void resetCol(s_ColPlate* colPlate, const s_ColWheel& Col);
protected:
	unsigned char setDownLinks(s_HexBasePlate* hexedImg, s_ColPlate* colPlate);
	void setColWheelUnitVectors(s_ColWheel& col);
};
namespace n_ColWheel {
	void clear(s_ColWheel& Col);
	void resetCol(s_ColWheel& col, const s_ColWheel& orig);/*resets only what changes when the col/sat/intensity val change but does not reset U vectors*/
	void copy(s_ColWheel& Col, const s_ColWheel& orig);
}
namespace n_ColPlate {
	bool run(s_HexBasePlate* hexedImg, s_ColPlate* colPlate, long plate_index);
}
#endif
