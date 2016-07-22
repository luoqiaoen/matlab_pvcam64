/* PVCAMSET - set PVCAM device parameters

      FLAG = PVCAMSET(HCAM, PARAM, VALUE) assigns VALUE to the PVCAM
	  parameter specified by the string PARAM for the camera specified
	  by HCAM.  FLAG returns 0 if an error occurred, 1 if the program
	  was successful.  The user should call PVCAMGET() first to obtain
	  acceptable values for the specified parameter.  See the PVCAM
	  manual for valid parameter names. */

/* 2/17/03 SCM */


// inclusions
#include "pvcamutil.h"
#include <math.h>				// for fmod() function used in pvcam_set_numeric

// definitions
#define FIELD_SIZE		12		// max length for structure field names
#define	STRING_FIELD	5		// number of fields in string param structure
#define NUMERIC_FIELD	9		// number of fields in numeric param structure
#define ENUM_FIELD		8		// number of fields in enumerated param structure


// function prototypes

// set numeric parameter value
rs_bool pvcam_set_numeric(int16 hcam, uns32 param_id, double param_value);


// global variables
rs_bool	attr_avail;		// flag for available parameter
uns16	attr_access;	// flag for read only, read/write
uns16	attr_type;		// data type of parameter values
uns32	attr_count;		// count for enumerated/char parameters


// gateway routine
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	// declarations
	rs_bool	success = 0;	// flag for successful execution
	char	*param_name;	// parameter name pointer
	double	*param_value;	// parameter value pointer
	int		param_len;		// parameter name length
	int16	hcam;			// camera handle
	uns32	param_id;		// parameter ID

	// validate arguments
	if ((nrhs != 3) || (nlhs > 1)) {
        mexErrMsgTxt("type 'help pvcamset' for syntax");
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
		mexErrMsgTxt("PARAM must be a string");
	}
	else if ((param_len = mxGetNumberOfElements(prhs[1])) < 1) {
		mexErrMsgTxt("PARAM cannot be empty");
	}
	else {
		param_name = (char *) mxCalloc(param_len + 1, sizeof(char));
		if (mxGetString(prhs[1], param_name, param_len + 1)) {
			mexErrMsgTxt("Could not retrieve PARAM from input arguments");
		}
	}

	// obtain parameter value
	if (!mxIsDouble(prhs[2])) {
		mexErrMsgTxt("VALUE must be numeric");
	}
	else if (mxGetNumberOfElements(prhs[2]) != 1) {
		mexErrMsgTxt("VALUE must be a scalar");
	}
	else if ((param_value = mxGetPr(prhs[2])) == NULL) {
		mexErrMsgTxt("Could not retrieve VALUE from input arguments");
	}

	// check for open camera
	if (!pl_cam_check(hcam)) {
		pvcam_error(hcam, "HCAM is not a handle to an open camera");
	}

	// obtain parameter ID
	else if (!pvcam_param_id(hcam, param_name, &param_id)) {
	}

	// obtain parameter availability
	else if (!pl_get_param(hcam, param_id, ATTR_AVAIL, (void *) &attr_avail)) {
		pvcam_error(hcam, "Cannot obtain parameter availability");
	}

	// do not proceed if parameter is not available
	else if (!attr_avail) {
		pvcam_error(hcam, "Parameter not available on this camera");
	}

	// obtain parameter accessibility 
	else if (!pl_get_param(hcam, param_id, ATTR_ACCESS, (void *) &attr_access)) {
		pvcam_error(hcam, "Cannot obtain parameter accessibility");
	}

	// do not proceed if parameter is not write only or read/write
	else if ((attr_access != ACC_WRITE_ONLY) && (attr_access != ACC_READ_WRITE)) {
		pvcam_error(hcam, "Parameter cannot be set");
	}

	// obtain parameter type 
	else if (!pl_get_param(hcam, param_id, ATTR_TYPE, (void *) &attr_type)) {
		pvcam_error(hcam, "Cannot obtain parameter type");
	}

	// obtain parameter count
	else if (!pl_get_param(hcam, param_id, ATTR_COUNT, (void *) &attr_count)) {
		pvcam_error(hcam, "Cannot obtain parameter count");
	}

	// set success flag to return value of pvcam_set_numeric()
	else {
		success = pvcam_set_numeric(hcam, param_id, *param_value);
	}

	// set output to return value of success flag
	plhs[0] = mxCreateDoubleScalar((double) success);

	// free allocated space
	mxFree((void *) param_name);
}


// set numeric parameter value
rs_bool pvcam_set_numeric(int16 hcam, uns32 param_id, double param_value) {

	// declarations
	rs_bool	success = 0;	// flag for successful execution
	double	param_min;		// minimum parameter value
	double	param_max;		// maximum parameter value
	double	param_inc;		// parameter increment
	int8	int8_value;		// signed byte
	uns8	uns8_value;		// unsigned byte
	int16	int16_value;	// signed word
	uns16	uns16_value;	// unsigned word
	int32	int32_value;	// signed dword
	uns32	uns32_value;	// unsigned dword
	flt64	flt64_value;	// double
	rs_bool	bool_value;		// boolean

	// obtain min, max & increment for numeric types
	// enumerated parameter limits given in ATTR_COUNT
	if (attr_type == TYPE_ENUM) {
		param_min = 0.0;
		param_inc = 1.0;
		param_max = (double) attr_count - param_inc;
	}
	else if (!pvcam_param_value(hcam, param_id, ATTR_MIN, attr_type, &param_min)) {
		return(0);
	}
	else if (!pvcam_param_value(hcam, param_id, ATTR_MAX, attr_type, &param_max)) {
		return(0);
	}
	else if (!pvcam_param_value(hcam, param_id, ATTR_INCREMENT, attr_type, &param_inc)) {
		return(0);
	}

	// check for valid parameter value
	if (param_value < param_min) {
		pvcam_error(hcam, "Parameter value exceeds minimum value allowed");
		return(0);
	}
	else if (param_value > param_max) {
		pvcam_error(hcam, "Parameter value exceeds maximum value allowed");
		return(0);
	}
	else if (param_inc > 0.0) {
		if (fmod((param_value - param_min), param_inc) > 0.0) {
			pvcam_error(hcam, "Parameter value not integer multiple of increment");
			return(0);
		}
	}

	// recast parameter value and save to appropriate storage variable
	// set parameter value with appropriate storage variable
	switch (attr_type) {
	case TYPE_INT8:
		int8_value = (int8) param_value;
		success = pl_set_param(hcam, param_id, (void *) &int8_value);
		break;
	case TYPE_UNS8:
		uns8_value = (uns8) param_value;
		success = pl_set_param(hcam, param_id, (void *) &uns8_value);
		break;
	case TYPE_INT16:
		int16_value = (int16) param_value;
		success = pl_set_param(hcam, param_id, (void *) &int16_value);
		break;
	case TYPE_UNS16:
		uns16_value = (uns16) param_value;
		success = pl_set_param(hcam, param_id, (void *) &uns16_value);
		break;
	case TYPE_INT32:
		int32_value = (int32) param_value;
		success = pl_set_param(hcam, param_id, (void *) &int32_value);
		break;
	case TYPE_UNS32:
	case TYPE_ENUM:
		uns32_value = (uns32) param_value;
		success = pl_set_param(hcam, param_id, (void *) &uns32_value);
		break;
	case TYPE_FLT64:
		flt64_value = (flt64) param_value;
		success = pl_set_param(hcam, param_id, (void *) &flt64_value);
		break;
	case TYPE_BOOLEAN:
		bool_value = (rs_bool)param_value;
		success = pl_set_param(hcam, param_id, (void *) &bool_value);
		break;
	default:
		pvcam_error(hcam, "Invalid data type for setting numeric parameter value");
		break;
	}

	// return value of successful execution flag
	return(success);
}
