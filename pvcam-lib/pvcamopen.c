/* PVCAMOPEN - opens PVCAM device

      HCAM = PVCAMOPEN(NCAMERA) opens and initializes the PVCAM device specified
	  by NCAM and returns a handle to the open camera HCAM.  NCAM is an integer
	  from 0 to NCAMERA - 1.  If unsuccessful, HCAM = []. */


/* 2/19/03 SCM */
/* 11/18/16 QL */

// inclusions
#include "pvcamutil.h"


// gateway routine
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	// declarations
	int16		hcam;		// camera handle
	int16		ncam;		// camera number

	// validate arguments
	if ((nrhs != 1) || (nlhs > 1)) {
        mexErrMsgTxt("type 'help pvcamopen' for syntax");
    }

	// obtain camera handle
	if (!mxIsNumeric(prhs[0])) {
		mexErrMsgTxt("NCAMERA must be numeric");
	}
	else if (mxGetNumberOfElements(prhs[0]) != 1) {
		mexErrMsgTxt("NCAMERA must be a scalar");
	}
	else {
		ncam = (int16) mxGetScalar(prhs[0]);
	}

	// return HCAM if camera opened
	// return [] otherwise
	if (pvcam_open(ncam, &hcam)) {
		plhs[0] = mxCreateDoubleScalar((double) hcam);
	}
	else {
		plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
	}
}
