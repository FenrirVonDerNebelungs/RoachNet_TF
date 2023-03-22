#pragma once
#ifndef RENDERBASE_H
#define RENDERBASE_H
#ifndef STRUCTS_H
#include "../Base/Structs.h"
#endif
#ifndef IMG_H
#include "../Base/Img.h"
#endif
class RenderBase : public Base {
public:
	RenderBase();
	~RenderBase();

	unsigned char spawnHexPlateImg(s_HexBasePlate* plt, Img* iimg);/*image pointer must already point to an object
									                             if image is of 0 width and height it is assumed it is not initialized 
																 and it will be initialized to the dimensions of the plate
																 if the image is already initalized it must be of the same dimensions of the 
																 plate inorder to not return an error */
	void despawnHexPlateImg(Img* img);
protected:
	bool m_flag_doGridOverlay;
	s_rgb m_grid_col;
	Img* m_hex_mask;/*mask of hex image from plate rounded to bigger*/
	s_2pt_i m_hex_mask_center;

	bool IsImgInit(Img* iimg);
	bool IsImgDimMatch(s_HexPlate* plt, Img* iimg);
	unsigned char InitMask_for_hexes_of_HexPlate(s_HexPlate* plt);
	unsigned char InitImg_for_HexPlate(s_HexPlate* plt, Img* iimg);
	unsigned char RenderHexPlate_to_Img(s_HexPlate* plt, Img* iimg);
	unsigned char RenderHexGrid(s_HexPlate* plt, Img* iimg);

	/*helpers*/
	unsigned char InitMask_for_hex_dim(float r, float rs);/*r is the long radius rs is the direct short side out radius*/
	unsigned char FillMask_for_U_of_HexPlate(s_HexPlate* plt);
	bool do_FillBetweenUs(const s_2pt& pt, const s_2pt& U0, const s_2pt& U1, float rs);/*pt is given from center of U's*/
};
#endif
