/* PVCAMSHUTTER - opens/closes PVCAM device shutter

      FLAG = PVCAMOPEN(HCAM, 'open') opens the shutter of the PVCAM device
	  specified by HCAM and returns 1 if successful, 0 if an error occurred.

      FLAG = PVCAMOPEN(HCAM, 'close') closes the shutter of the PVCAM device
	  specified by HCAM and returns 1 if successful, 0 if an error occurred.


/* 4/17/07 SCM */


// inclusions
#include "pvcamutil.h"

// gateway routine
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	// declarations
	char	*shutter_cmd;	// shutter command string
	int		shutter_len;	// shutter command length
	int16	hcam;			// camera handle
	rs_bool	shutter_flag;	// flag returned by pl_exp_abort

	// validate arguments
	if ((nrhs != 2) || (nlhs > 1)) {
        mexErrMsgTxt("type 'help pvcamget' for syntax");
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

	// obtain parameter name
	if (!mxIsChar(prhs[1])) {
		mexErrMsgTxt("COMMAND must be a string");
	}
	else if ((shutter_len = mxGetNumberOfElements(prhs[1])) < 1) {
		mexErrMsgTxt("COMMAND cannot be empty");
	}
	else {
		shutter_cmd = (char *) mxCalloc(shutter_len + 1, sizeof(char));
		if (mxGetString(prhs[1], shutter_cmd, shutter_len + 1)) {
			mexErrMsgTxt("Could not retrieve COMMAND from input arguments");
		}
	}

	// check for open camera
	if (!pl_cam_check(hcam)) {
		pvcam_error(hcam, "HCAM is not a handle to an open camera");
	}

	// use pl_exp_abort to open or close shutter
	if (strcmp(shutter_cmd, "open") == 0) {
		shutter_flag = pl_exp_abort(hcam, CCS_OPEN_SHTR);
	}
	else if (strcmp(shutter_cmd, "close") == 0) {
		shutter_flag = pl_exp_abort(hcam, CCS_HALT_CLOSE_SHTR);
	}
	else {
		mexErrMsgTxt("COMMAND must be 'open' or 'closed'");
	}

	// return flag and print error message if needed
	plhs[0] = mxCreateDoubleScalar((double) shutter_flag);
	if (!shutter_flag) {
		pvcam_error(hcam, "pl_exp_abort returned error");
	}

	// free allocated space
	mxFree((void *) shutter_cmd);
}
