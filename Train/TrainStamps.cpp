#include "TrainStamps.h"
unsigned char n_trainStamps_pipes::init(int nnet_num, int array_len, s_trainStamps_pipes& sp) {
	if (array_len < 1)
		return ECODE_ABORT;
	sp.eyeBase = new s_HexPlate* [array_len];
	sp.col = new s_HexPlateLayer* [array_len];
	sp.luna = new s_HexPlateLayer* [array_len];
	for (int ii = 0; ii < array_len; ii++) {
		sp.eyeBase[ii] = NULL;
		sp.col[ii] = NULL;
		sp.luna[ii] = NULL;
	}
	sp.N = 0;
	sp.N_max = array_len;
	sp.nnet_num = nnet_num;
	return ECODE_OK;
}
unsigned char n_trainStamps_pipes::fill(s_HexPlate* hexedImg, s_HexPlateLayer* colPlates, s_HexPlateLayer* lunaPlates, s_trainStamps_pipes& sp) {
	if (sp.N >= sp.N_max)
		return ECODE_ABORT;
	unsigned char errc = ECODE_OK;
	sp.eyeBase[sp.N] = new s_HexPlate;
	errc=sp.eyeBase[sp.N]->init(hexedImg);
	if (errc != ECODE_OK)
		return ECODE_FAIL;
	sp.col[sp.N] = new s_HexPlateLayer;
	errc=sp.col[sp.N]->init(colPlates);
	if (errc != ECODE_OK)
		return ECODE_FAIL;
	sp.luna[sp.N] = new s_HexPlateLayer;
	errc=sp.luna[sp.N]->init(lunaPlates);
	if (errc != ECODE_OK)
		return ECODE_FAIL;

	return ECODE_OK;
}

unsigned char TrainStamps::init(float scale_r, float gaus_smudge_sigma_divisor, float smudge_mult_factor_for_offset, float smudge_angle, int num_bak_smudge, float exclusion_train_ang, float exclusion_train_opening_DAng) {
	m_scale_r = scale_r;
	n_stampsKey::clear(m_master_stamps_key);
	m_stamp_file_name = CTARGAIMAGE_IMGFILEPRE;
	m_stamp_raw_dir = STAMP_DUMPBASE_DIR;
	m_stamp_raw_dir += "/";
	m_stamp_raw_dir +=(STAMP_ROUNDCORN_DIR);
	m_stamp_raw_dir += "/";
	m_genEye = new HexEye;
	if (Err(m_genEye->init(scale_r, TRAINSTAMPS_stamp_eye_levels)))
		return ECODE_FAIL;
	m_baseEye = new s_HexEye;
	if (Err(m_genEye->spawn(m_baseEye)))
		return ECODE_FAIL;
	m_Convol = new ConvolHex;
	/*find the lowest level of the eye*/
	s_HexPlate* lowest_eye_level_hexes = m_baseEye->getBottom();
	/*find the pointer to the nodes in the lowest eye plate*/
	s_Node** lowest_hex_eye_nodes = lowest_eye_level_hexes->getNodes();
	if (Err(m_Convol->init(NULL, lowest_hex_eye_nodes, scale_r)))/*image will need to be set with update*/
		return ECODE_FAIL;
	m_genStamps = new Stamp;
	/*find the image dimension of the lowest hexes, which is the same as the dim of the eye*/
	float lowest_eye_dim = (m_baseEye->height >= m_baseEye->width) ? (float)m_baseEye->height : (float)m_baseEye->width;
	float padded_eye_dim = lowest_eye_dim + TRAINSTAMPS_lowest_eye_dim_padding_multiplier * scale_r + TRAINSTAMPS_eye_dim_extension_multiplier*scale_r;
	m_baseImg = new Img;
	if (Err(m_baseImg->init(padded_eye_dim, padded_eye_dim, 3L)))
		return ECODE_FAIL;
	if (Err(m_genStamps->init(padded_eye_dim, scale_r)))
		return ECODE_FAIL;
	m_genPreImgs = new GenPreImgs;
	/*m_genPreImgs will be initalized and released for every NNet*/
	m_parse = new ParseTxt;
	/*m_parse will be initialized and released for every NNet*/
	m_master_stamps_key.r = scale_r;
	m_master_stamps_key.Dim = lowest_eye_dim;
	m_master_stamps_key.maxDim = padded_eye_dim;
	m_master_stamps_key.sig_bak_rotang_jitter = smudge_angle;
	m_master_stamps_key.smudge_factor = smudge_mult_factor_for_offset;
	m_master_stamps_key.N_bak_smudge = num_bak_smudge;
	m_master_stamps_key.N_sig_smudge = 0;
	m_master_stamps_key.smudge_sigma_divisor = gaus_smudge_sigma_divisor;
	/*start of debug*/
	m_tga = new CTargaImage;
	if (Err(m_tga->Init()))
		return ECODE_FAIL;
	/* end of  debug*/
	m_hexEyeImg = new HexEyeImg;
	s_2pt eyeCenter = { padded_eye_dim / 2.f, padded_eye_dim / 2.f };
	if(Err(m_hexEyeImg->init(m_baseImg, eyeCenter, m_genEye)))
		return ECODE_FAIL;

	/*start of stuff used for luna */
	m_whiteColWheel.pixMax = 255.f;
	m_whiteColWheel.Dhue = 1.f;
	m_whiteColWheel.DI = 1.f;
	m_whiteColWheel.DSat = 0.f;
	m_whiteColWheel.HueFadeV = 0.5f;
	m_whiteColWheel.I_target = 1.f;
	m_whiteColWheel.Hue_target.x0 = 1.f;
	m_whiteColWheel.Hue_target.x1 = 0.f;
	m_whiteColWheel.Sat_target = 0.f;

	m_genCol = new Col;
	int number_of_colors_and_color_plate_layers = 1;
	if (Err(m_genCol->init(number_of_colors_and_color_plate_layers)))
		return ECODE_FAIL;
	if (Err(m_genCol->addCol(&m_whiteColWheel)))
		return ECODE_FAIL;
	m_genLuna = new Luna;
	if (Err(m_genLuna->init(m_scale_r)))/*use one color plate by default*/
		return ECODE_FAIL;
	m_lunaPat = new s_Luna;
	if (Err(m_genLuna->spawn(m_lunaPat)))
		return ECODE_FAIL;

	return ECODE_OK;
}
unsigned char TrainStamps::genRawStamps() {
	unsigned char err= m_genStamps->run();
	m_num_raw_stamps = m_genStamps->getNumberOfStamps();
	return err;
}
unsigned char TrainStamps::getCornKeys() {
	if (m_num_raw_stamps < 1)
		return ECODE_FAIL;
	if (m_cornKeys != NULL)
		return ECODE_FAIL;
	/*parse the key file to read in the data keys for each stamp*/
	m_cornKeys = new s_rCornKey[m_num_raw_stamps];
	m_parse->init();
	std::string pathToInKeyFile(m_stamp_raw_dir);
	pathToInKeyFile += STAMP_KEY;
	m_parse->setInFile(pathToInKeyFile);
	s_datLine* datLines = new s_datLine[m_num_raw_stamps];
	int num_lines_read = m_parse->readCSV(datLines, m_num_raw_stamps);
	m_parse->release();
	if (num_lines_read != m_num_raw_stamps)
		return ECODE_FAIL;
	int len_read = 0;
	for (int ii = 0; ii < m_num_raw_stamps; ii++) {
		len_read = n_rCornKey::datLineToKey(datLines[ii], m_cornKeys[ii]);
		if (len_read < 1)
			return ECODE_FAIL;
	}
	return ECODE_OK;
}
void TrainStamps::releaseCornKeys() {
	if (m_cornKeys != NULL)
		delete[] m_cornKeys;
	m_cornKeys = NULL;
}
unsigned char TrainStamps::genNNetKeyFiles(int stamp_NNET_num) {
	if (m_num_raw_stamps < 1)
		return ECODE_FAIL;
	/*setup the directory*/
	m_stamp_current_key_dir = m_stamp_raw_dir;
	m_stamp_current_key_dir=(TRAINSTAMPS_NETTARGET_PREFIX_DIR);
	std::string net_num(to_string(stamp_NNET_num));
	m_stamp_current_key_dir += net_num;
	m_stamp_current_key_dir += "/";
	/*                    */
	m_selKeys = new s_stampKey[m_num_raw_stamps];


	int corn_index = getCornKeyIndex(stamp_NNET_num);
	if (corn_index < 0)
		return ECODE_FAIL;
	s_rCornKey NNet_key;
	/*add the signal at the start*/
	n_rCornKey::copy(NNet_key, m_cornKeys[corn_index]);
	int num_bak_stamps = 0;
	int num_sig_stamps = 0;
	int num_tot_stamps = 0;
	for (int i = 0; i < m_num_raw_stamps; i++) {
		if (goodBak(m_cornKeys[i], NNet_key)) {
			n_stampKey::copy(m_selKeys[num_tot_stamps], m_cornKeys[i].key);	/*copy the cornKeys into just stampKeys*/
			m_selKeys[num_tot_stamps].y = -1.f;
			m_selKeys[num_tot_stamps].train = 1;
			num_bak_stamps++;
			num_tot_stamps++;
		}
		else if (isSignal(m_cornKeys[i], NNet_key)) {
			n_stampKey::copy(m_selKeys[num_tot_stamps], m_cornKeys[i].key);	/*copy the cornKeys into just stampKeys*/
			m_selKeys[num_tot_stamps].y = 1.f;
			m_selKeys[num_tot_stamps].train = 1;
			num_sig_stamps++;
			num_tot_stamps++;
		}
	}
	if (num_sig_stamps < 1)
		return ECODE_FAIL;
	/*setup the master key*/
	m_master_stamps_key.N = num_tot_stamps;
	m_master_stamps_key.N_sig = num_sig_stamps;
	m_master_stamps_key.N_bak = num_bak_stamps;
	/*now write the files*/
	if (Err(dumpKeys(m_stamp_current_key_dir, m_selKeys, m_master_stamps_key, num_tot_stamps)))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char TrainStamps::genNNetStampSets() {
	std::string dump_file_dir(m_stamp_current_key_dir); /*this directory is already set to NNet1, NNet2 etc for selected target stamp*/
	dump_file_dir += "/";
	dump_file_dir += TRAINSTAMPS_PRENET_DIR;
	dump_file_dir += "/";

	unsigned char err =  m_genPreImgs->init(m_stamp_raw_dir, m_stamp_current_key_dir);
	if (Err(err))
		return ECODE_FAIL;
	m_numPreStamps = m_genPreImgs->getN();
	m_preImgs = new Img*[m_numPreStamps];
	/*image init will take placein the genPreImage*/
	m_preKeys = new s_stampKey[m_numPreStamps];
	for (int i = 0; i < m_numPreStamps; i++) {
		n_stampKey::clear(m_preKeys[i]);/*shouldn't really be necessry since other key is copied into this one but good standard practice*/
		m_preImgs[i] = new Img;
		bool errcode = m_genPreImgs->spawn(m_preImgs[i], m_preKeys[i]);
		if (!errcode)
			break;/*probably won't be reached*/
	}

	/*fill the master key*/
	n_stampsKey::copy(m_preMasterKey, m_master_stamps_key);
	m_preMasterKey.N_bak_smudge = 1;/*after smudge don't need to smudge further*/
	m_preMasterKey.N_sig_smudge = 1;
	m_preMasterKey.N = m_numPreStamps;
	m_preMasterKey.N_sig = m_genPreImgs->getNSig();
	m_preMasterKey.N_bak = m_genPreImgs->getNBak();

	if (Err(dumpKeys(dump_file_dir, m_preKeys, m_preMasterKey, m_numPreStamps)))
		return ECODE_FAIL;
	std::string debugImg_file_dir(dump_file_dir);
	debugImg_file_dir += "/";
	debugImg_file_dir += TRAINSTAMPS_DEBUGIMG_DIR;
	debugImg_file_dir += "/";
	if (Err(dumpImgs(debugImg_file_dir, m_preImgs, m_numPreStamps)))
		return ECODE_FAIL;

	m_genPreImgs->release();
	return ECODE_OK;
}

int TrainStamps::getCornKeyIndex(int Key_ID) {
	int found_index = -1;
	for(int i=0; i<m_num_raw_stamps; i++)
		if (m_cornKeys[i].key.ID == Key_ID) {
			found_index = i;
			break;
		}
	return found_index;
}

unsigned char TrainStamps::dumpKeys(std::string& PathToFile, s_stampKey dkeys[], s_stampsKey& master_key, int numKeys) {
	s_datLine* stamp_key_out_dlines = new s_datLine[numKeys];
	for (int i = 0; i < numKeys; i++) {
		n_stampKey::dumpToDatLine(dkeys[i], stamp_key_out_dlines[i]);
	}
	/*write all the stamp keys*/
	m_parse->init();
	std::string stampKey_file_dir(PathToFile);
	stampKey_file_dir += "/";
	stampKey_file_dir += STAMP_KEY;
	m_parse->setOutFile(stampKey_file_dir);
	m_parse->writeCSV(stamp_key_out_dlines, numKeys);
	m_parse->release();
	delete[] stamp_key_out_dlines;

	/*write out the master key*/
	s_datLine one_dline[1];
	n_stampsKey::dumpToDatLine(master_key, one_dline[0]);
	std::string masterKey_file_dir(PathToFile);
	masterKey_file_dir += "/";
	masterKey_file_dir += STAMP_MASTER_KEY;
	m_parse->init();
	m_parse->setOutFile(masterKey_file_dir);
	m_parse->writeCSV(one_dline, 1);
	m_parse->release();

	return ECODE_OK;
}
unsigned char TrainStamps::dumpImgs(std::string& PathToFile, Img* dImgs[], int numImgs) {
	for (int i = 0; i < numImgs; i++){
		std::string file_name(PathToFile);
		file_name += CTARGAIMAGE_IMGFILEPRE;
		std::string net_num(to_string(i));
		file_name += net_num;
		file_name += CTARGAIMAGE_IMGFILESUF;
		m_tga->Open(dImgs[i]->getImg(), dImgs[i]->getWidth(), dImgs[i]->getHeight(), false, IMAGE_RGB);
		unsigned char err=m_tga->Write(file_name.c_str(), dImgs[i]->getColorMode());
		m_tga->Close();
		if (Err(err)) {
			return err;
		}
	}
	return ECODE_OK;
}
unsigned char TrainStamps::genPipeObjsForLuna() {
	/*generates an eye that is just under the size of the image*/
	m_nnet_hexEye = new s_HexEye;
	if (Err(m_genEye->spawn(m_nnet_hexEye)))
		return ECODE_FAIL;
	if (Err(m_hexEyeImg->root(m_baseImg, *m_nnet_hexEye))) /*roots the eye in the center of the image*/
		return ECODE_FAIL;

	/*generate the color plate layer and attach it to the bottom plate of the hex eye*/
	m_colPlates = new s_ColPlateLayer;
	s_HexPlate* bottom_plate_hexeye = m_nnet_hexEye->getBottom();
	if (Err(m_genCol->spawn(bottom_plate_hexeye, m_colPlates)))
		return ECODE_FAIL;

	/*generate the luna layers object and imbedd the luna in the object*/
	m_lunaPlates = new s_HexBasePlateLayer;
	if (Err(m_genLuna->spawn(m_lunaPat, m_lunaPlates, m_colPlates)))
		return ECODE_FAIL;

	return ECODE_OK;
}

unsigned char TrainStamps::runLunaOnEyes(int stamp_NNet_num) {
	/*debug*/
	genStampPipes(stamp_NNet_num);
	/*     */
	/*for this part m_preImgs must have been generated*/
	for (int i = 0; i < m_numPreStamps; i++) {
		/*run the base of the eye on the image hexing the image, the eye has already been rooted in the correct location*/
		if (Err(m_hexEyeImg->run(m_preImgs[i], *m_nnet_hexEye)))
			return ECODE_FAIL;
		s_HexPlate* hexedEyeImage = m_nnet_hexEye->getBottom();
		long num_hexes = hexedEyeImage->N;

		/*run the col plate*/
		for (long plate_hex_i = 0; plate_hex_i < num_hexes; plate_hex_i++)
			n_Col::run(hexedEyeImage, m_colPlates, plate_hex_i);

		/*run the lunas, for imbedded*/
		for(long plate_hex_i=0; plate_hex_i<num_hexes; plate_hex_i++)
			n_Luna::run(m_colPlates, m_lunaPlates, plate_hex_i);
		/*debug*/
		if (Err(genStampPipeForStamp()))/*s_trainStamps_pipes auto increments internally, 
										this function automatically grabs the appropriate class variables
										creating simple copys of the bottom of the nnet_hexEye, colPlates, and lunaPlates*/
			return ECODE_FAIL;
		/*    */
	}
	return ECODE_OK;
}

unsigned char TrainStamps::genStampPipes(int stamp_NNet_num) {
	m_stampPipes = new s_trainStamps_pipe;
	if (Err(n_trainStamps_pipes::init(stamp_NNet_num, m_numPreStamps, *m_stampPipes)))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char TrainStamps::genStampPipeForStamp() {
	return n_trainStamps_pipes::fill(m_nnet_hexEye->getBottom(), m_colPlates, m_lunaPlates, *m_stampPipes);
}
