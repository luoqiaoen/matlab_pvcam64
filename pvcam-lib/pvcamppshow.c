/* PVCAMPPSHOW - DISPLAY POST PROCESSING FEATURES

      Success = PVCAMAVAIL(HCAM) display available functions and features and corresponding values.
	  return 1 if successful, 0 otherwise.


/* 2/19/17 JQL */


// inclusions
#include "pvcamutil.h"


// function prototypes

// acquire image(s) from camera
mxArray *pvcam_pp_show(int16 hcam);

// global variables
rs_bool	attr_avail;		// flag for available parameter
uns16	attr_access;	// flag for read only, read/write
uns16	attr_type;		// data type of parameter values
uns32	attr_count;		// count for enumerated/char parameters

// gateway routine
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	// declarations
	int16		hcam;		// camera handle

	// validate arguments
	if ((nrhs != 1) || (nlhs > 0)) {
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

	// check for open camera
	// acquire image sequence
	// assign empty matrix if failure
	
	if (pl_cam_check(hcam)) {
		pvcam_pp_show(hcam);
	}
	else {
		pvcam_error(hcam, "HCAM is not a handle to an open camera");
	}
}


// acquire image(s) from camera
mxArray *pvcam_pp_show(int16 hcam) {
	
	// declarations
	uns32 featCount; // number of features
	// create empty mxArray for error output
	
	// load exposure sequence
	// obtain number of bytes needed to store images
	if (!pl_get_param(hcam, PARAM_PP_INDEX, ATTR_COUNT, &featCount)) {
		pvcam_error(hcam, "Failed to know number of pp features");
		return (0);
	}
	printf("\nThe list of valid features:\n");
	    for (int16 featureIndex = 0; featureIndex < (int16)featCount; featureIndex++)
    {
        // Set index to a particular feature
        if (!pl_set_param(hcam, PARAM_PP_INDEX, &featureIndex))
        {
            pvcam_error(hcam, "Failed to set feature index");
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

        // The list of usable functions of the feature
        uns32 funcCount;
        if (!pl_get_param(hcam, PARAM_PP_PARAM_INDEX, ATTR_COUNT, &funcCount))
        {
            pvcam_error(hcam, "Failed to get list of usable feature functions");
            return (0);
        }
        printf("\nThe list of valid functions of the feature, their minimum, maximum, default and current value:\n");

        for (int16 functionIndex = 0; functionIndex < (int16)funcCount; functionIndex++)
        {
            // Set index to a particular post processing function
            if (!pl_set_param(hcam, PARAM_PP_PARAM_INDEX, &functionIndex))
            {
            pvcam_error(hcam, "Failed to set index for list of usable feature functions");
            return (0);
            }
            // Get the function Name
            char functionName[MAX_PP_NAME_LEN];
            if (!pl_get_param(hcam, PARAM_PP_PARAM_NAME, ATTR_CURRENT,
                        functionName))
            {
                pvcam_error(hcam, "Failed to set index for list of usable feature functions");
                return (0);
            }
            // Get the function ID
            uns16 functionID;
            if (!pl_get_param(hcam, PARAM_PP_PARAM_ID, ATTR_CURRENT,
                        &functionID))
            {
                pvcam_error(hcam,
                        "Failed to get the function IDs");
                return (0);
            }
            
		    uns32 minValue;		
            if (!pl_get_param(hcam, PARAM_PP_PARAM, ATTR_MIN, &minValue))
            {
                pvcam_error(hcam,
                        "Failed to get the function minimum values");
                return (0);
            }
        // The maximal value of the required parameter
            uns32 maxValue;
            if (!pl_get_param(hcam, PARAM_PP_PARAM, ATTR_MAX, &maxValue))
            {
                pvcam_error(hcam,
                        "Failed to get the function maximum values");
                return (0);
            }
        // The default value of the required parameter
            uns32 defValue;
            if (!pl_get_param(hcam, PARAM_PP_PARAM, ATTR_DEFAULT, &defValue))
            {
                pvcam_error(hcam,
                        "Failed to get the function default values");
                return (0);
            }
        // The current value of the required parameter
            uns32 curValue;
            if (!pl_get_param(hcam, PARAM_PP_PARAM, ATTR_CURRENT, &curValue))
            {
                pvcam_error(hcam,
                        "Failed to get the function current values");
                return (0);
            }
		// Show obtained parameters and values on the screen
        printf ( "\n Function ID: %u Function index: %d Function Name: %s Minimum: %lu , Maximum: %lu, Default: %lu, Current:%lu \n",
                functionID, functionIndex, functionName, minValue, maxValue, defValue, curValue);
        }
    }
	}