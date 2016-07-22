/* PVCAMCLOSE - closes open PVCAM device

      STATUS = PVCAMCLOSE(HCAM) closes the PVCAM device specified by the
	  handle to the open camera HCAM.  STATUS is 1 if there are no errors.  */


/* 2/19/03 SCM */


// inclusions
#include "pvcamutil.h"


// gateway routine
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	// declarations
	int16		hcam;		// camera handle

	// validate arguments
	if ((nrhs != 1) || (nlhs > 1)) {
        mexErrMsgTxt("type 'help pvcamclose' for syntax");
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

	// close camera and return boolean
	pvcam_close(hcam);
	plhs[0] = mxCreateLogicalScalar(true);
}
