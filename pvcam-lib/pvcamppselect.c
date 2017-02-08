/* PVCAMPPSHOW - DISPLAY POST PROCESSING FEATURES

      DATA = PVCAMAVAIL(HCAM) returns available PP features


/* 2/19/17 JQL */


// inclusions
#include "pvcamutil.h"


// function prototypes

// acquire image(s) from camera
mxArray *pvcam_pp_select(int16 hcam);

// global variables
rs_bool	attr_avail;		// flag for available parameter
uns16	attr_access;	// flag for read only, read/write
uns16	attr_type;		// data type of parameter values
uns32	attr_count;		// count for enumerated/char parameters

// gateway routine
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	// declarations
	int16		hcam;		// camera handle
    int16       featureIndex;
	int16       functionIndex;
	uns32       featureValue;
	// validate arguments
	if ((nrhs != 4) || (nlhs > 0)) {
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
	
	if (!mxIsNumeric(prhs[1])) {
		mexErrMsgTxt("Feature Index must be numeric");
	}
	else if (mxGetNumberOfElements(prhs[1]) != 1) {
		mexErrMsgTxt("Feature Index must be a scalar");
	}
	else {
		featureIndex = (int16) mxGetScalar(prhs[1]);
	}
	
	if (!mxIsNumeric(prhs[2])) {
		mexErrMsgTxt("Function Index must be numeric");
	}
	else if (mxGetNumberOfElements(prhs[2]) != 1) {
		mexErrMsgTxt("Function Index must be a scalar");
	}
	else {
		functionIndex = (int16) mxGetScalar(prhs[2]);
	}
	
	if (!mxIsNumeric(prhs[3])) {
		mexErrMsgTxt("Function value must be numeric");
	}
	else if (mxGetNumberOfElements(prhs[3]) != 1) {
		mexErrMsgTxt("Function value must be a scalar");
	}
	else {
		featureValue = (uns32) mxGetScalar(prhs[3]);
	}

	// check for open camera
	// acquire image sequence
	// assign empty matrix if failure
	
	if (pl_cam_check(hcam)) {
		pvcam_pp_select(hcam, featureIndex, functionIndex, featureValue);
	}
	else {
		pvcam_error(hcam, "HCAM is not a handle to an open camera");
		plhs[0] = mxCreateNumericMatrix(0, 0, mxUINT16_CLASS, mxREAL);
	}
}


mxArray *pvcam_pp_select(int16 hcam, int16 featureIndex, int16 functionIndex, uns32 featureValue) {
	
	// declarations

    // Set index to a particular feature
    if (!pl_set_param(hcam, PARAM_PP_INDEX, &featureIndex))
    {
        pvcam_error(hcam, "Failed to set the feature index");
        return (0);
    }

        // Get the feature Name
    char featureName[MAX_PP_NAME_LEN];
    if (!pl_get_param(hcam, PARAM_PP_FEAT_NAME, ATTR_CURRENT, featureName))
    {
        pvcam_error(hcam, "Failed to get feature names");
        return (0);
    }
        // Get the feature ID
    uns16 featureID;
    if (!pl_get_param(hcam, PARAM_PP_FEAT_ID, ATTR_CURRENT, &featureID))
    {
        pvcam_error(hcam, "Failed to get feature ID");
        return (0);
    }
        // Show obtained features and index values on the screen
    printf ("\n\nFeature ID: %u Feature index: %d Feature Name: %s\n",
                featureID, featureIndex, featureName);

    
    // Set index to a particular post processing function
    if (!pl_set_param(hcam, PARAM_PP_PARAM_INDEX, &functionIndex))
    {
    pvcam_error(hcam, "Failed to set index for list of usable feature functions");
    return (0);
    }
    // Get the function Name
    char functionName[MAX_PP_NAME_LEN];
    if (!pl_get_param(hcam, PARAM_PP_PARAM_NAME, ATTR_CURRENT, functionName))
    {
    pvcam_error(hcam, "Failed to set index for list of usable feature functions");
    return (0);
    }
    // Get the function ID
    uns16 functionID;
    if (!pl_get_param(hcam, PARAM_PP_PARAM_ID, ATTR_CURRENT,
                        &functionID))
    {
        pvcam_error(hcam, "Failed to get the function IDs");
                return (0);
    }
    printf ( "\n Function ID: %u Function index: %d Function Name: %s\n",
                    functionID, functionIndex, functionName);
	
	if(!pl_set_param(hcam, PARAM_PP_PARAM, &featureValue))
    {
        pvcam_error(hcam, "Failed to set value");
                return (0);
    }
    printf("\nSetting the parameter value has succeeded!\n");
    
	uns32 curValue;
    // Get current value of the parameter
    if (!pl_get_param(hcam, PARAM_PP_PARAM, ATTR_CURRENT, &curValue))
    {
        pvcam_error(hcam, "Failed to get value set");
                return (0);
    }
    // Show the current valure of thew parameter on the screen
    printf("\nThe value of the post processing function is set to: %d\n", curValue);
}