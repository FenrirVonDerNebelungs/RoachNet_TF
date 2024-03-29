#pragma once
#ifndef TRAINSTAMPS_H
#define TRAINSTAMPS_H


#ifndef CONVOLHEX_H
#include "../HexedImg/ConvolHex.h"
#endif
#ifndef TRAINSTAMPSBASE_H
#include "TrainStampsBase.h"
#endif
#ifndef LUNA_H
#include "../Luna/Luna.h"
#endif

const int TRAINSTAMPS_stamp_eye_levels = 3;
float TRAINSTAMPS_lowest_eye_dim_padding_multiplier = 8.f;/*padding is for when image is moved when slid*/
float TRAINSTAMPS_eye_dim_extension_multiplier = 4.f;/*extension is for luna overlap*/
#define TRAINSTAMPS_NETTARGET_PREFIX_DIR "NNet"/*each of the dirs NNet1, NNet2 etc will contain the images and keys for the training stamps for one of the nnets*/
#define TRAINSTAMPS_PRENET_DIR "Pre" /*directory where the results of the smudge and sig increase with their correct keys are put*/
#define TRAINSTAMPS_DEBUGIMG_DIR "DebugImg"

struct s_trainStamps_pipes {
	s_HexPlate** eyeBase;
	s_HexPlateLayer** col;
	s_HexPlateLayer** luna;
	int N;/*length of arrays*/
	int N_max;/*length of arrays in memory*/
	int nnet_num;
};
namespace n_trainStamps_pipes {
	unsigned char init(int nnet_num, int array_len, s_trainStamps_pipes &sp);/*creates new sub objects but does not init them*/
	void          release(s_trainStamps_pipes& sp);

	unsigned char fill(s_HexPlate* hexedImg, s_HexPlateLayer* colPlates, s_HexPlateLayer* lunaPlates, s_trainStamps_pipes& sp);
	void          empty(s_trainStamps_pipes& sp);
}

class TrainStamps : public Base {
public:
	unsigned char init(
		float scale_r = 7.0, /*r scale of hexes at lowest level determines other dimensions*/
		float gaus_smudge_sigma_divisor = 2.f,
		float smudge_mult_factor_for_offset = 2.f,
		float smudge_angle = PI/12.f,
		int   num_bak_smudge=3,
		float exclusion_train_ang = 0.f,/*exclusion variables are set so that if stamp is within range of all it is excluded from the background set*/
		float exclusion_train_opening_DAng=PI/12.f /*stamps with similar angular curvature at the same rot angle are excluded from the background*/
	);

	unsigned char runOnStamp(int stamp_i);/*public for use with debug*/
	unsigned char run();

	void release();
protected:
	/*owned*/
	ConvolHex* m_Convol;/*takes the hexes from the eye that is overlayed on the stamp and does the gaus convol to fill the lowest hexes*/
	HexEye* m_genEye;/*generator of the eye*/
	s_HexEye* m_baseEye;/*the eye that lies on top of the current stamp image*/
	Img* m_baseImg;/*image with the corrrect dimensions for the base image under the eye*/
	ParseTxt* m_parse;/*object used to parse info*/
	/*this exists if doing debug*/
	CTargaImage* m_tga;
	/* emd of debug*/
	HexEyeImg* m_hexEyeImg;/*this is used to take values from the image and color the hexes for the base of the hexeye structs*/

	Stamp* m_genStamps;/*class that generates stamps*/
	GenPreImgs* m_genPreImgs;/*class that takes the stamps and sets them up for input into the NNet with an increas in signal and optional smudging of the background*/

	/******************************************/
	/***start of luna stuff***/

	s_ColWheel m_whiteColWheel;
	Col* m_genCol;/*generates color plate layers*/
	Luna* m_genLuna;
	s_Luna* m_lunaPat;
	
	/*all objects in processing of image*/
	s_HexEye* m_nnet_hexEye;
	s_ColPlateLayer* m_colPlates;/*really only one plate*/
	s_HexBasePlateLayer* m_lunaPlates;/*all the luna plates for each luna rot run on the color plate(of the colplatelayer)*/


	/***end of luna stuff***/
	/******************************************/

	float       m_scale_r;
	/*util vars filled at run time*/

    std::string m_stamp_file_name;
	int m_num_raw_stamps;
	std::string m_stamp_raw_dir;
	std::string m_stamp_current_key_dir;
	/*holds the corner keys read in from the stamp generated files*/
	s_rCornKey* m_cornKeys;/*cornkey is a larger version of stampkey*/
	/*                            */
	/*stamp keys selected with y's set for a particular 
	  NNet */
	s_stampKey* m_selKeys;
	s_stampsKey m_master_stamps_key;
	/*hold variables for the pre stage,
	  the stage where the back and signal
	  permutations have been populated and
	  background and signal have been equalized */
	s_stampsKey m_preMasterKey;
	s_stampKey* m_preKeys;
	Img** m_preImgs;
	int m_numPreStamps;
	/*                                          */

	unsigned char genRawStamps();/*after this is run the Raw directory is populated with the different L1 corner stamps at all different angles
								   currently the stampkey for all of these stamps indicates signal */
	unsigned char getCornKeys();
	void          releaseCornKeys();
	unsigned char genNNetKeyFiles(int stamp_NNET_num);/*assumes that each stamp in raw should have its own NNET*/
	void          releaseNNetKeyFiles();
	unsigned char genNNetStampSets();/* generates stampkey and stampskey files that reference the stamp images in the
											  raw directory but are setup 
											  needs genNNetKeyFiles to be run just before to set the directories*/

	/*helpers to genNNetKeyFiles*/
	int getCornKeyIndex(int Key_ID);
	bool goodBak(const s_rCornKey& bak_cand_key, const s_rCornKey signal_key);
	bool isSignal(const s_rCornKey& cand_key, const s_rCornKey signal_key);

	/*helpers to dump for debug */
	unsigned char dumpKeys(std::string& PathToFile, s_stampKey dkeys[], s_stampsKey& master_key, int numKeys);
	unsigned char dumpImgs(std::string& PathToFile, Img* dImgs[], int numImgs);

	/*********************************************************************/
	/*next part is running processing code on each of the stamp images
	  when this is finished the value 1 to 0 for intensity of each hex
	  representing a luna value and for all luna plates is dumped
	  these values go into the nnet trained by tensorflow */
	  /*debug*/
	s_trainStamps_pipes* m_stampPipes;/*used to save the hexed images after each stage this is deleted and
									 overwritten for each nnet*/

	unsigned char genPipeObjsForLuna();/*generates all the constructs that for each image run on a stamp are filled
									     to ultimately result in a set of plates with the luna values recorded in them*/
	void          releasePipeObjsForLuna();

	unsigned char runLunaOnEyes(int stamp_NNet_num);
	/*  debug */
	unsigned char genStampPipes(int stamp_NNet_num);/*copies base of m_nnet_hexEye, m_colPlates and m_LunaPlates into curent stampPipe*/
	void          releaseStampPipes();
	unsigned char genStampPipeForStamp();
	void          releaseStampsPipeForStamp();
	/********************************************************************/
	/*nnet values presumed comming from tensor flow are put back into
	  the trained nnets */
};
#endif