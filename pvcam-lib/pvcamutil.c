/* Utilities for PVCAM MEX files */
/* SCM 9/2/02 */


// inclusions
#include "pvcamutil.h"


// create 2D array
char **pvcam_create_array(int nstring, int nchar) {

	// declarations
	char	**char_array;	// output pointer
	int		i;				// loop counter

	// allocate array storage
	char_array = (char **) mxMalloc((size_t) nstring * sizeof(char *));
	for (i = 0; i < nstring; i++) {
		char_array[i] = (char *) mxCalloc((size_t) nchar, sizeof(char));
	}
	return(char_array);
}


// free 2D array
void pvcam_destroy_array(char **char_array, int nstring) {

	// declarations
	int		i;				// loop counter

	// deallocate array storage
	for (i = 0; i < nstring; i++) {
		mxFree((void *) char_array[i]);
	}
	mxFree((void *) char_array);
}


// open PVCAM camera
rs_bool pvcam_open(int16 ncamera, int16 *hcam) {
	
	// declarations
	char	cam_name[CAM_NAME_LEN];
	int16	total_cameras;

	// try to initialize PVCAM routines
	if (!pl_pvcam_init()) {
		pvcam_error(0, "Cannot init PVCAM");
		return(0);
	}

	// obtain number of cameras
	if (!pl_cam_get_total(&total_cameras)) {
		pvcam_error(0, "Cannot find number of cameras");
		return(0);
	}

	// check specified camera doesn't exceed number found
	if (total_cameras <= ncamera) {
		pvcam_error(0, "Specified camera number not found");
		return(0);
	}

	// obtain name of specified camera
	if (!pl_cam_get_name(0, cam_name)) {
		pvcam_error(0, "Cannot obtain camera name");
		return(0);
	}

	// obtain camera handle
	if (!pl_cam_open(cam_name, hcam, OPEN_EXCLUSIVE)) {
		pvcam_error(*hcam, "Cannot open specified camera");
		return(0);
	}
	
	return(1);
}


// close camera
void pvcam_close(int16 hcam) {
	if (pl_cam_check(hcam)) {
		pl_cam_close(hcam);
	}
	pl_pvcam_uninit();
}


// display error message
void pvcam_error(int16 hcam, const char *err_msg) {

	// declarations
	char	*final_msg;
	char	*pvcam_msg;

	// display message from user
	// only display PVCAM message if error code set
	if (pl_error_code() == 0) {
		//pvcam_close(hcam);
		mexWarnMsgTxt(err_msg);
	}
	else {
		pvcam_msg = (char *) mxCalloc((size_t) ERROR_MSG_LEN, sizeof(char));
		pl_error_message(pl_error_code(), pvcam_msg);
		final_msg = (char *) mxCalloc(strlen(pvcam_msg) + ERROR_MSG, sizeof(char));
		sprintf(final_msg, "PVCAM error %d: %s", pl_error_code(), pvcam_msg);
		//pvcam_close(hcam);
		mexWarnMsgTxt(err_msg);
		mexWarnMsgTxt(final_msg);
		mxFree((void *) final_msg);
		mxFree((void *) pvcam_msg);
	}
}


// return access type string
char *pvcam_access_string(int16 hcam, uns16 access_id) {
	
	// declarations
	char	*access_string;
	
	// allocate space for return string
	access_string = (char *) mxCalloc((size_t) ACCESS_STR_LEN, sizeof(char));

	// return access string that matches ID
	switch (access_id) {
	case ACC_ERROR:
		strcpy(access_string, "access error");
		break;
	case ACC_EXIST_CHECK_ONLY:
		strcpy(access_string, "check only");
		break;
	case ACC_READ_ONLY:
		strcpy(access_string, "read only");
		break;
	case ACC_WRITE_ONLY:
		strcpy(access_string, "write only");
		break;
	case ACC_READ_WRITE:
		strcpy(access_string, "read/write");
		break;
	default:
		pvcam_error(hcam, "Access ID not recognized");
		mxFree((void *) access_string);
		access_string = NULL;
		break;
	}
	return(access_string);
}


// return data type string
char *pvcam_type_string(int16 hcam, uns16 type_id) {
	
	// declarations
	char	*type_string;

	// allocate space for return string
	type_string = (char *) mxCalloc((size_t) TYPE_STR_LEN, sizeof(char));

	// return data type string that matches ID
	switch (type_id) {
	case TYPE_CHAR_PTR:
		strcpy(type_string, "string");
		break;
	case TYPE_INT8:
		strcpy(type_string, "signed char");
		break;
	case TYPE_UNS8:
		strcpy(type_string, "unsigned char");
		break;
	case TYPE_INT16:
		strcpy(type_string, "short");
		break;
	case TYPE_UNS16:
		strcpy(type_string, "unsigned short");
		break;
	case TYPE_INT32:
		strcpy(type_string, "long");
		break;
	case TYPE_UNS32:
		strcpy(type_string, "unsigned long");
		break;
	case TYPE_FLT64:
		strcpy(type_string, "double");
		break;
	case TYPE_ENUM:
		strcpy(type_string, "enumerated");
		break;
	case TYPE_BOOLEAN:
		strcpy(type_string, "boolean");
		break;
	case TYPE_VOID_PTR:
		strcpy(type_string, "ptr to void");
		break;
	case TYPE_VOID_PTR_PTR:
		strcpy(type_string, "void ptr to ptr");
		break;
	default:
		pvcam_error(hcam, "Data type ID not recognized");
		mxFree((void *) type_string);
		type_string = NULL;
		break;
	}
	return(type_string);
}


// obtain parameter value recast as DOUBLE
rs_bool pvcam_param_value(int16 hcam, uns32 param_id, int16 param_attrib,
						  uns16 param_type, double *param_value) {

	// declarations
	rs_bool	status;			// error flag on pl_get_param
	int8	int8_value;		// signed byte
	uns8	uns8_value;		// unsigned byte
	int16	int16_value;	// signed word
	uns16	uns16_value;	// unsigned word
	int32	int32_value;	// signed dword
	uns32	uns32_value;	// unsigned dword
	flt64	flt64_value;	// double
	rs_bool	boolean_value;	// boolean

	// get parameter value with appropriate storage variable
	// recast storage value as DOUBLE
	switch (param_type) {
	case TYPE_INT8:
		status = pl_get_param(hcam, param_id, param_attrib, (void *) &int8_value);
		*param_value = (double) int8_value;
		break;
	case TYPE_UNS8:
		status = pl_get_param(hcam, param_id, param_attrib, (void *) &uns8_value);
		*param_value = (double) uns8_value;
		break;
	case TYPE_INT16:
		status = pl_get_param(hcam, param_id, param_attrib, (void *) &int16_value);
		*param_value = (double) int16_value;
		break;
	case TYPE_UNS16:
		status = pl_get_param(hcam, param_id, param_attrib, (void *) &uns16_value);
		*param_value = (double) uns16_value;
		break;
	case TYPE_INT32:
		status = pl_get_param(hcam, param_id, param_attrib, (void *) &int32_value);
		*param_value = (double) int32_value;
		break;
	case TYPE_UNS32:
	case TYPE_ENUM:
		status = pl_get_param(hcam, param_id, param_attrib, (void *) &uns32_value);
		*param_value = (double) uns32_value;
		break;
	case TYPE_FLT64:
		status = pl_get_param(hcam, param_id, param_attrib, (void *) &flt64_value);
		*param_value = (double) flt64_value;
		break;
	case TYPE_BOOLEAN:
		status = pl_get_param(hcam, param_id, param_attrib, (void *) &boolean_value);
		*param_value = (double) boolean_value;
		break;
	default:
		status = 0;
		pvcam_error(hcam, "Invalid data type for numeric parameter");
		break;
	}
	
	// check for error & return value
	if (!status) {
		pvcam_error(hcam, "Error obtaining numeric parameter value");
	}
	return(status);
}


// return selected PVCAM parameter ID
rs_bool pvcam_param_id(int16 hcam, const char *param_name, uns32 *param_id) {
	
	// declarations
	char	*err_msg;

	// find parameter ID that matches string

	// class 0 (camera communications)
	if (strcmp(param_name, "PARAM_DD_INFO") == 0) {
		*param_id = PARAM_DD_INFO;
	}
	else if (strcmp(param_name, "PARAM_DD_INFO_LENGTH") == 0) {
		*param_id = PARAM_DD_INFO_LENGTH;
	}
	else if (strcmp(param_name, "PARAM_DD_VERSION") == 0) {
		*param_id = PARAM_DD_VERSION;
	}
	else if (strcmp(param_name, "PARAM_DD_RETRIES") == 0) {
		*param_id = PARAM_DD_RETRIES;
	}
	else if (strcmp(param_name, "PARAM_DD_TIMEOUT") == 0) {
		*param_id = PARAM_DD_TIMEOUT;
	}

	// class 2 (CCD skip parameters)
	else if (strcmp(param_name, "PARAM_MIN_BLOCK") == 0) {
		*param_id = PARAM_MIN_BLOCK;
	}
	else if (strcmp(param_name, "PARAM_NUM_MIN_BLOCK") == 0) {
		*param_id = PARAM_NUM_MIN_BLOCK;
	}
	else if (strcmp(param_name, "PARAM_SKIP_AT_ONCE_BLK") == 0) {
		*param_id = PARAM_SKIP_AT_ONCE_BLK;
	}
	else if (strcmp(param_name, "PARAM_NUM_OF_STRIPS_PER_CLR") == 0) {
		*param_id = PARAM_NUM_OF_STRIPS_PER_CLR;
	}
	else if (strcmp(param_name, "PARAM_CONT_CLEARS") == 0) {
		*param_id = PARAM_CONT_CLEARS;
	}

	// class 2 (general camera properties)
	else if (strcmp(param_name, "PARAM_ANTI_BLOOMING") == 0) {
		*param_id = PARAM_ANTI_BLOOMING;
	}
	else if (strcmp(param_name, "PARAM_LOGIC_OUTPUT") == 0) {
		*param_id = PARAM_LOGIC_OUTPUT;
	}
	else if (strcmp(param_name, "PARAM_EDGE_TRIGGER") == 0) {
		*param_id = PARAM_EDGE_TRIGGER;
	}
	else if (strcmp(param_name, "PARAM_INTENSIFIER_GAIN") == 0) {
		*param_id = PARAM_INTENSIFIER_GAIN;
	}
	else if (strcmp(param_name, "PARAM_SHTR_GATE_MODE") == 0) {
		*param_id = PARAM_SHTR_GATE_MODE;
	}
	else if (strcmp(param_name, "PARAM_ADC_OFFSET") == 0) {
		*param_id = PARAM_ADC_OFFSET;
	}
	else if (strcmp(param_name, "PARAM_CHIP_NAME") == 0) {
		*param_id = PARAM_CHIP_NAME;
	}
	else if (strcmp(param_name, "PARAM_COOLING_MODE") == 0) {
		*param_id = PARAM_COOLING_MODE;
	}
	else if (strcmp(param_name, "PARAM_PREAMP_DELAY") == 0) {
		*param_id = PARAM_PREAMP_DELAY;
	}
	else if (strcmp(param_name, "PARAM_PREFLASH") == 0) {
		*param_id = PARAM_PREFLASH;
	}
	else if (strcmp(param_name, "PARAM_COLOR_MODE") == 0) {
		*param_id = PARAM_COLOR_MODE;
	}
	else if (strcmp(param_name, "PARAM_MPP_CAPABLE") == 0) {
		*param_id = PARAM_MPP_CAPABLE;
	}
	else if (strcmp(param_name, "PARAM_PREAMP_OFF_CONTROL") == 0) {
		*param_id = PARAM_PREAMP_OFF_CONTROL;
	}
	else if (strcmp(param_name, "PARAM_SERIAL_NUM") == 0) {
		*param_id = PARAM_SERIAL_NUM;
	}

	// class 2 (CCD dimensions)
	else if (strcmp(param_name, "PARAM_PREMASK") == 0) {
		*param_id = PARAM_PREMASK;
	}
	else if (strcmp(param_name, "PARAM_PRESCAN") == 0) {
		*param_id = PARAM_PRESCAN;
	}
	else if (strcmp(param_name, "PARAM_POSTMASK") == 0) {
		*param_id = PARAM_POSTMASK;
	}
	else if (strcmp(param_name, "PARAM_POSTSCAN") == 0) {
		*param_id = PARAM_POSTSCAN;
	}
	else if (strcmp(param_name, "PARAM_PIX_PAR_DIST") == 0) {
		*param_id = PARAM_PIX_PAR_DIST;
	}
	else if (strcmp(param_name, "PARAM_PIX_PAR_SIZE") == 0) {
		*param_id = PARAM_PIX_PAR_SIZE;
	}
	else if (strcmp(param_name, "PARAM_PIX_SER_DIST") == 0) {
		*param_id = PARAM_PIX_SER_DIST;
	}
	else if (strcmp(param_name, "PARAM_PIX_SER_SIZE") == 0) {
		*param_id = PARAM_PIX_SER_SIZE;
	}
	else if (strcmp(param_name, "PARAM_SUMMING_WELL") == 0) {
		*param_id = PARAM_SUMMING_WELL;
	}
	else if (strcmp(param_name, "PARAM_FWELL_CAPACITY") == 0) {
		*param_id = PARAM_FWELL_CAPACITY;
	}
	else if (strcmp(param_name, "PARAM_PAR_SIZE") == 0) {
		*param_id = PARAM_PAR_SIZE;
	}
	else if (strcmp(param_name, "PARAM_SER_SIZE") == 0) {
		*param_id = PARAM_SER_SIZE;
	}
	else if (strcmp(param_name, "PARAM_ACCUM_CAPABLE") == 0) {
		*param_id = PARAM_ACCUM_CAPABLE;
	}
	else if (strcmp(param_name, "PARAM_FLASH_DWNLD_CAPABLE") == 0) {
		*param_id = PARAM_FLASH_DWNLD_CAPABLE;
	}

	// class 2 (general camera properties)
	else if (strcmp(param_name, "PARAM_CONTROLLER_ALIVE") == 0) {
		*param_id = PARAM_CONTROLLER_ALIVE;
	}
	else if (strcmp(param_name, "PARAM_READOUT_TIME") == 0) {
		*param_id = PARAM_READOUT_TIME;
	}
	else if (strcmp(param_name, "PARAM_CLEAR_CYCLES") == 0) {
		*param_id = PARAM_CLEAR_CYCLES;
	}
	else if (strcmp(param_name, "PARAM_CLEAR_MODE") == 0) {
		*param_id = PARAM_CLEAR_MODE;
	}
	else if (strcmp(param_name, "PARAM_FRAME_CAPABLE") == 0) {
		*param_id = PARAM_FRAME_CAPABLE;
	}
	else if (strcmp(param_name, "PARAM_PMODE") == 0) {
		*param_id = PARAM_PMODE;
	}
	else if (strcmp(param_name, "PARAM_CCS_STATUS") == 0) {
		*param_id = PARAM_CCS_STATUS;
	}
	else if (strcmp(param_name, "PARAM_TEMP") == 0) {
		*param_id = PARAM_TEMP;
	}
	else if (strcmp(param_name, "PARAM_TEMP_SETPOINT") == 0) {
		*param_id = PARAM_TEMP_SETPOINT;
	}
	else if (strcmp(param_name, "PARAM_CAM_FW_VERSION") == 0) {
		*param_id = PARAM_CAM_FW_VERSION;
	}
	else if (strcmp(param_name, "PARAM_HEAD_SER_NUM_ALPHA") == 0) {
		*param_id = PARAM_HEAD_SER_NUM_ALPHA;
	}
	else if (strcmp(param_name, "PARAM_PCI_FW_VERSION") == 0) {
		*param_id = PARAM_PCI_FW_VERSION;
	}
	else if (strcmp(param_name, "PARAM_CAM_FW_FULL_VERSION") == 0) {
		*param_id = PARAM_CAM_FW_FULL_VERSION;
	}
	else if (strcmp(param_name, "PARAM_EXPOSURE_MODE") == 0) {
		*param_id = PARAM_EXPOSURE_MODE;
	}

	// class 2 (speed table)
	else if (strcmp(param_name, "PARAM_BIT_DEPTH") == 0) {
		*param_id = PARAM_BIT_DEPTH;
	}
	else if (strcmp(param_name, "PARAM_GAIN_INDEX") == 0) {
		*param_id = PARAM_GAIN_INDEX;
	}
	else if (strcmp(param_name, "PARAM_SPDTAB_INDEX") == 0) {
		*param_id = PARAM_SPDTAB_INDEX;
	}
	else if (strcmp(param_name, "PARAM_READOUT_PORT") == 0) {
		*param_id = PARAM_READOUT_PORT;
	}
	else if (strcmp(param_name, "PARAM_PIX_TIME") == 0) {
		*param_id = PARAM_PIX_TIME;
	}

	// class 2 (shutter)
	else if (strcmp(param_name, "PARAM_SHTR_CLOSE_DELAY") == 0) {
		*param_id = PARAM_SHTR_CLOSE_DELAY;
	}
	else if (strcmp(param_name, "PARAM_SHTR_OPEN_DELAY") == 0) {
		*param_id = PARAM_SHTR_OPEN_DELAY;
	}
	else if (strcmp(param_name, "PARAM_SHTR_OPEN_MODE") == 0) {
		*param_id = PARAM_SHTR_OPEN_MODE;
	}
	else if (strcmp(param_name, "PARAM_SHTR_STATUS") == 0) {
		*param_id = PARAM_SHTR_STATUS;
	}
	else if (strcmp(param_name, "PARAM_SHTR_CLOSE_DELAY_UNIT") == 0) {
		*param_id = PARAM_SHTR_CLOSE_DELAY_UNIT;
	}

	// class 2 (I/O)
	else if (strcmp(param_name, "PARAM_IO_ADDR") == 0) {
		*param_id = PARAM_IO_ADDR;
	}
	else if (strcmp(param_name, "PARAM_IO_TYPE") == 0) {
		*param_id = PARAM_IO_TYPE;
	}
	else if (strcmp(param_name, "PARAM_IO_DIRECTION") == 0) {
		*param_id = PARAM_IO_DIRECTION;
	}
	else if (strcmp(param_name, "PARAM_IO_STATE") == 0) {
		*param_id = PARAM_IO_STATE;
	}
	else if (strcmp(param_name, "PARAM_IO_BITDEPTH") == 0) {
		*param_id = PARAM_IO_BITDEPTH;
	}

	// class 2 (gain multiplier)
	else if (strcmp(param_name, "PARAM_GAIN_MULT_FACTOR") == 0) {
		*param_id = PARAM_GAIN_MULT_FACTOR;
	}
	else if (strcmp(param_name, "PARAM_GAIN_MULT_ENABLE") == 0) {
		*param_id = PARAM_GAIN_MULT_ENABLE;
	}

	// class 3 (acquisition)
	else if (strcmp(param_name, "PARAM_EXP_TIME") == 0) {
		*param_id = PARAM_EXP_TIME;
	}
	else if (strcmp(param_name, "PARAM_EXP_RES") == 0) {
		*param_id = PARAM_EXP_RES;
	}
	else if (strcmp(param_name, "PARAM_EXP_MIN_TIME") == 0) {
		*param_id = PARAM_EXP_MIN_TIME;
	}
	else if (strcmp(param_name, "PARAM_EXP_RES_INDEX") == 0) {
		*param_id = PARAM_EXP_RES_INDEX;
	}

	// class 3 (buffer)
	else if (strcmp(param_name, "PARAM_BOF_EOF_ENABLE") == 0) {
		*param_id = PARAM_BOF_EOF_ENABLE;
	}
	else if (strcmp(param_name, "PARAM_BOF_EOF_COUNT") == 0) {
		*param_id = PARAM_BOF_EOF_COUNT;
	}
	else if (strcmp(param_name, "PARAM_BOF_EOF_CLR") == 0) {
		*param_id = PARAM_BOF_EOF_CLR;
	}
	else if (strcmp(param_name, "PARAM_CIRC_BUFFER") == 0) {
		*param_id = PARAM_CIRC_BUFFER;
	}
	else if (strcmp(param_name, "PARAM_HW_AUTOSTOP") == 0) {
		*param_id = PARAM_HW_AUTOSTOP;
	}
	
	// generate error message if parameter not found
	else {
		err_msg = (char *) mxCalloc(strlen(param_name) + ERROR_MSG, sizeof(char));
		sprintf(err_msg, "Parameter %s is not recognized", param_name);
		pvcam_error(hcam, err_msg);
		mxFree((void *) err_msg);
		return(0);
	}
	return(1);
}
