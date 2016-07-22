/* PVCAMACQ - acquire image sequence from PVCAM device

      DATA = PVCAMACQ(HCAM, NI, ROI, EXPTIME, EXPMODE) acquires an image
	  sequence of NI images over the CCD region(s) specified by the structure
	  array ROI from the camera specified by HCAM.  The exposure time is
	  specified by EXPTIME; the units depend on the PARAM_EXP_RES and the
	  PARAM_EXP_RES_INDEX settings.  The exposure mode ('timed', 'trigger',
	  'strobe', or 'bulb') is provided by EXPMODE.  The structure array ROI
	  must have the following scalar fields:

					s1 = first serial register
					s2 = last serial register
					sbin = serial binning factor
					p1 = first parallel register
					p2 = last parallel register
					pbin = parallel binning factor

	  The length of the structure array ROI determines the number of CCD
	  regions that will be imaged.  If successful, DATA will be a vector
	  (unsigned 16-bit integer) containing the data from the image sequence.
	  The calling routine must reshape this vector based upon ROIs and images
	  in the sequence.  If unsuccessful, DATA = []. */


/* 2/19/03 SCM */


// inclusions
#include "pvcamutil.h"


// function prototypes

// acquire image(s) from camera
mxArray *pvcam_acquire(int16 hcam, uns16 nimage, uns16 nregion, rgn_type *region, uns32 exptime, int16 expmode);

// obtain field values from ROI structure
uns16 get_field_value(const mxArray *struct_array, uns16 nstruct, uns16 nfield);


// gateway routine
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	// declarations
	char		*modestr;	// exposure mode string
	int			modelen;	// exposure mode string length
	int16		hcam;		// camera handle
	int16		expmode;	// exposure mode
	rgn_type	*region;	// ROI structure
	uns16		nimage;		// number of images
	uns16		nregion;	// number of regions
	uns16		nfield[6];	// field numbers in ROI structure
	uns16		i;			// loop counters
	uns32		exptime;	// exposure time

	// validate arguments
	if ((nrhs != 5) || (nlhs > 1)) {
        mexErrMsgTxt("type 'help pvcamacq' for syntax");
    }

	// obtain camera handle
	if (!mxIsNumeric(prhs[0])) {
		mexErrMsgTxt("HCAM must be numeric");
	}
	else if (mxGetNumberOfElements(prhs[0]) != 1) {
		mexErrMsgTxt("HCAM must be a scalar");
	}
	else {
		hcam = (int16) mxGetScalar(prhs[0]);
	}

	// obtain number of images
	if (!mxIsNumeric(prhs[1])) {
		mexErrMsgTxt("NI must be numeric");
	}
	else if (mxGetNumberOfElements(prhs[1]) != 1) {
		mexErrMsgTxt("NI must be a scalar");
	}
	else {
		nimage = (uns16) mxGetScalar(prhs[1]);
	}

	// obtain ROI structure from MATLAB structure array
	if (!mxIsStruct(prhs[2])) {
		mexErrMsgTxt("ROI must be a structure array");
	}
	else if ((nregion = mxGetNumberOfElements(prhs[2])) < 1) {
		mexErrMsgTxt("ROI cannot be empty");
	}
	else if ((nfield[0] = mxGetFieldNumber(prhs[2], "s1")) < 0) {
		mexErrMsgTxt("ROI must contain a field named s1");
	}
	else if ((nfield[1] = mxGetFieldNumber(prhs[2], "s2")) < 0) {
		mexErrMsgTxt("ROI must contain a field named s2");
	}
	else if ((nfield[2] = mxGetFieldNumber(prhs[2], "sbin")) < 0) {
		mexErrMsgTxt("ROI must contain a field named sbin");
	}
	else if ((nfield[3] = mxGetFieldNumber(prhs[2], "p1")) < 0) {
		mexErrMsgTxt("ROI must contain a field named p1");
	}
	else if ((nfield[4] = mxGetFieldNumber(prhs[2], "p2")) < 0) {
		mexErrMsgTxt("ROI must contain a field named p2");
	}
	else if ((nfield[5] = mxGetFieldNumber(prhs[2], "pbin")) < 0) {
		mexErrMsgTxt("ROI must contain a field named pbin");
	}

	// allocate space for ROI data
	// obtain elements from MATLAB structure
	region = (rgn_type *) mxCalloc((size_t) nregion, sizeof(rgn_type));
	for (i = 0; i < nregion; i++) {
		region[i].s1 = get_field_value(prhs[2], i, nfield[0]);
		region[i].s2 = get_field_value(prhs[2], i, nfield[1]);
		region[i].sbin = get_field_value(prhs[2], i, nfield[2]);
		region[i].p1 = get_field_value(prhs[2], i, nfield[3]);
		region[i].p2 = get_field_value(prhs[2], i, nfield[4]);
		region[i].pbin = get_field_value(prhs[2], i, nfield[5]);
	}

	// obtain exposure time
	if (!mxIsNumeric(prhs[3])) {
		mexErrMsgTxt("EXPTIME must be numeric");
	}
	else if (mxGetNumberOfElements(prhs[3]) != 1) {
		mexErrMsgTxt("EXPTIME must be a scalar");
	}
	else {
		exptime = (uns16) mxGetScalar(prhs[3]);
	}

	// obtain exposure mode
	if (!mxIsChar(prhs[4])) {
		mexErrMsgTxt("EXPMODE must be a string");
	}
	else if ((modelen = mxGetNumberOfElements(prhs[4])) < 1) {
		mexErrMsgTxt("EXPMODE cannot be empty");
	}
	else {
		modelen++;
		modestr = (char *) mxCalloc(modelen, sizeof(char));
		if (mxGetString(prhs[4], modestr, modelen)) {
			mexErrMsgTxt("Cannot read EXPMODE string");
		}
		else if (strcmp(modestr, "timed") == 0) {
			expmode = TIMED_MODE;
		}
		else if (strcmp(modestr, "trigger") == 0) {
			expmode = TRIGGER_FIRST_MODE;
		}
		else if (strcmp(modestr, "strobe") == 0) {
			expmode = STROBED_MODE;
		}
		else if (strcmp(modestr, "bulb") == 0) {
			expmode = BULB_MODE;
		}
		else if (strcmp(modestr, "flash") == 0) {
			expmode = FLASH_MODE;
		}
		else {
			mexWarnMsgTxt("EXPMODE not recognized, using timed mode");
			expmode = TIMED_MODE;
		}
		mxFree((void *) modestr);
	}

	// check for open camera
	// acquire image sequence
	// assign empty matrix if failure
	if (pl_cam_check(hcam)) {
		plhs[0] = pvcam_acquire(hcam, nimage, nregion, region, exptime, expmode);
	}
	else {
		pvcam_error(hcam, "HCAM is not a handle to an open camera");
		plhs[0] = mxCreateNumericMatrix(0, 0, mxUINT16_CLASS, mxREAL);
	}

	// free allocated arrays
	mxFree((void *) region);
}


// acquire image(s) from camera
mxArray *pvcam_acquire(int16 hcam, uns16 nimage, uns16 nregion, rgn_type *region, uns32 exptime, int16 expmode) {
	
	// declarations
	int		npixel;			// number of pixels to be read
	int16	status;			// camera read status
	mxArray	*data_struct;	// output structure
	mxArray	*empty_struct;	// empty structure if error
	uns16	*data_ptr;		// output data
	uns32	bytes_read;		// bytes read by camera
	uns32	image_size;		// image size in bytes

	// create empty mxArray for error output
	empty_struct = mxCreateNumericMatrix(0, 0, mxUINT16_CLASS, mxREAL);

	// initialize exposure sequence
	if (!pl_exp_init_seq()) {
		pvcam_error(hcam, "Cannot initialize exposure sequence");
		return(empty_struct);
	}
	
	// load exposure sequence
	// obtain number of bytes needed to store images
	if (!pl_exp_setup_seq(hcam, nimage, nregion, region, expmode, exptime, &image_size)) {
		pvcam_error(hcam, "Cannot setup exposure sequence");
		return(empty_struct);
	}

	// create output structure
	// set pointer to capture camera data
	// start exposure sequence
	npixel = (int) (image_size / sizeof(uns16));
	data_struct = mxCreateNumericMatrix(1, npixel, mxUINT16_CLASS, mxREAL);
	data_ptr = (uns16 *) mxGetData(data_struct);
	if (!pl_exp_start_seq(hcam, data_ptr)) {
		pvcam_error(hcam, "Cannot start exposure sequence");
		mxDestroyArray(data_struct);
		return(empty_struct);
	}
	
	// loop until exposure sequence is complete
	status = -1;
	while ((status != READOUT_COMPLETE) && (status != READOUT_NOT_ACTIVE) && (status != READOUT_FAILED)) {
		if (!pl_exp_check_status(hcam, &status, &bytes_read)) {
			pvcam_error(hcam, "Cannot check camera status during exposure");
			mxDestroyArray(data_struct);
			return(empty_struct);
		}
	}
	
	// uninitialize exposure sequence
	if (!pl_exp_uninit_seq()) {
		pvcam_error(hcam, "Cannot uninitialize exposure sequence");
		mxDestroyArray(data_struct);
		return(empty_struct);
	}
	
	// determine how exposure sequence terminated
	// return data structure if successful
	switch (status) {
	case READOUT_COMPLETE:
		mxDestroyArray(empty_struct);
		return(data_struct);
		break;
	case READOUT_NOT_ACTIVE:
		pvcam_error(hcam, "Camera readout never started");
		break;
	case READOUT_FAILED:
		pvcam_error(hcam, "Camera readout failed");
		break;
	default:
		pvcam_error(hcam, "Unknown camera readout termination");
		break;
	}
	mxDestroyArray(data_struct);
	return(empty_struct);
}


// obtain field values from ROI structure
uns16 get_field_value(const mxArray *struct_array, uns16 nstruct, uns16 nfield) {

	// declarations
	mxArray		*field_value;	// pointer to field value

	// extract pointer to field value
	field_value = mxGetFieldByNumber(struct_array, (int) nstruct, (int) nfield);
	if (field_value == NULL) {
		mexErrMsgTxt("ROI has empty field value");
	}
	else if (!mxIsNumeric(field_value)) {
		mexErrMsgTxt("ROI has non-numeric field value");
	}

	// return field value
	return((uns16) mxGetScalar(field_value));
}
