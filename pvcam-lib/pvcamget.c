/* PVCAMGET - obtain PVCAM device parameters

      STRUCT = PVCAMGET(HCAM, PARAM) returns a structure containing
	  information about the PVCAM parameter specified by the string
	  PARAM for the camera specified by HCAM.  See the PVCAM manual
      for valid parameter names. */

/* 2/17/03 SCM */


// inclusions
#include "pvcamutil.h"


// definitions
#define FIELD_SIZE		12		// max length for structure field names
#define	STRING_FIELD	5		// number of fields in string param structure
#define NUMERIC_FIELD	9		// number of fields in numeric param structure
#define ENUM_FIELD		8		// number of fields in enumerated param structure


// function prototypes

// obtain string parameter
mxArray *pvcam_param_string(int16 hcam, uns32 param_id);

// obtain numeric parameter
mxArray *pvcam_param_numeric(int16 hcam, uns32 param_id);

// obtain enumerated parameter
mxArray *pvcam_param_enum(int16 hcam, uns32 param_id);


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
	int		param_len;		// parameter name length
	int16	hcam;			// camera handle
	uns32	param_id;		// parameter ID

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

	// do not proceed if parameter is not read only or read/write
	//else if ((attr_access != ACC_READ_ONLY) && (attr_access != ACC_READ_WRITE)) {
	//else if ((attr_access == ACC_ERROR) || (attr_access == ACC_EXIST_CHECK_ONLY)) {
	//	pvcam_error(hcam, "Parameter cannot be read");
	//}

	// obtain parameter type 
	else if (!pl_get_param(hcam, param_id, ATTR_TYPE, (void *) &attr_type)) {
		pvcam_error(hcam, "Cannot obtain parameter type");
	}

	// obtain parameter count
	else if (!pl_get_param(hcam, param_id, ATTR_COUNT, (void *) &attr_count)) {
		pvcam_error(hcam, "Cannot obtain parameter count");
	}

	// obtain output structure STRUCT based on parameter type
	else {
		switch (attr_type) {
		case TYPE_CHAR_PTR:
			if ((plhs[0] = pvcam_param_string(hcam, param_id)) != NULL) {
				success = 1;
			}
			break;
		case TYPE_INT8:
		case TYPE_UNS8:
		case TYPE_INT16:
		case TYPE_UNS16:
		case TYPE_INT32:
		case TYPE_UNS32:
		case TYPE_FLT64:
		case TYPE_BOOLEAN:
			if ((plhs[0] = pvcam_param_numeric(hcam, param_id)) != NULL) {
				success = 1;
			}
			break;
		case TYPE_ENUM:
			if ((plhs[0] = pvcam_param_enum(hcam, param_id)) != NULL) {
				success = 1;
			}
			break;
		case TYPE_VOID_PTR:
			pvcam_error(hcam, "Data type TYPE_VOID_PTR cannot be processed");
			break;
		case TYPE_VOID_PTR_PTR:
			pvcam_error(hcam, "Data type TYPE_VOID_PTR_PTR cannot be processed");
			break;
		default:
			pvcam_error(hcam, "Data type ID not recognized");
			break;
		}
	}

	// assign empty matrix if failure
	if (!success) {
		plhs[0] = mxCreateDoubleMatrix(0, 0, mxREAL);
	}

	// free allocated space
	mxFree((void *) param_name);
}


// obtain string parameter
mxArray *pvcam_param_string(int16 hcam, uns32 param_id) {

	// declarations
	char	*param_string;	// parameter string
	char	*access_string;	// access string
	char	*type_string;	// type string
	mxArray	*param_struct;	// output structure
	char	**field_list;	// field names for output structure

	// obtain strings and check for errors
	param_string = (char *) mxCalloc((size_t) attr_count, sizeof(char));
	if ((access_string = pvcam_access_string(hcam, attr_access)) == NULL) {
		param_struct = NULL;
	}
	else if ((type_string = pvcam_type_string(hcam, attr_type)) == NULL) {
		param_struct = NULL;
	}
	else if (!pl_get_param(hcam, param_id, ATTR_CURRENT, (void *) param_string)) {
		pvcam_error(hcam, "Error obtaining parameter string");
		param_struct = NULL;
	}
	
	else {
		
		// assign field names
		field_list = pvcam_create_array(STRING_FIELD, FIELD_SIZE);
		strcpy(field_list[0], "access_id");
		strcpy(field_list[1], "access");
		strcpy(field_list[2], "type_id");
		strcpy(field_list[3], "type");
		strcpy(field_list[4], "current");

		// store field values
		param_struct = mxCreateStructMatrix(1, 1, STRING_FIELD, field_list);
		mxSetField(param_struct, 0, field_list[0], mxCreateDoubleScalar((double) attr_access));
		mxSetField(param_struct, 0, field_list[1], mxCreateString(access_string));
		mxSetField(param_struct, 0, field_list[2], mxCreateDoubleScalar((double) attr_type));
		mxSetField(param_struct, 0, field_list[3], mxCreateString(type_string));
		mxSetField(param_struct, 0, field_list[4], mxCreateString(param_string));
		pvcam_destroy_array(field_list,	STRING_FIELD);
	}

	// return parameter structure
	mxFree((void *) param_string);
	return(param_struct);
}


// obtain numeric parameter
mxArray *pvcam_param_numeric(int16 hcam, uns32 param_id) {

	// declarations
	char	*access_string;	// access string
	char	*type_string;	// type string
	double	param_value;	// parameter value
	double	param_default;	// default value
	double	param_min;		// minimum value
	double	param_max;		// maximum value
	double	param_inc;		// parameter increment
	mxArray	*param_struct;	// output structure
	char	**field_list;	// field names for output structure

	// obtain strings and check for errors
	if ((access_string = pvcam_access_string(hcam, attr_access)) == NULL) {
		param_struct = NULL;
	}
	else if ((type_string = pvcam_type_string(hcam, attr_type)) == NULL) {
		param_struct = NULL;
	}

	// obtain values and check for errors
	else if (!pvcam_param_value(hcam, param_id, ATTR_CURRENT, attr_type, &param_value)) {
		param_struct = NULL;
	}
	else if (!pvcam_param_value(hcam, param_id, ATTR_DEFAULT, attr_type, &param_default)) {
		param_struct = NULL;
	}
	else if (!pvcam_param_value(hcam, param_id, ATTR_MIN, attr_type, &param_min)) {
		param_struct = NULL;
	}
	else if (!pvcam_param_value(hcam, param_id, ATTR_MAX, attr_type, &param_max)) {
		param_struct = NULL;
	}
	else if (!pvcam_param_value(hcam, param_id, ATTR_INCREMENT, attr_type, &param_inc)) {
		param_struct = NULL;
	}

	else {

		// assign field names
		field_list = pvcam_create_array(NUMERIC_FIELD, FIELD_SIZE);
		strcpy(field_list[0], "access_id");
		strcpy(field_list[1], "access");
		strcpy(field_list[2], "type_id");
		strcpy(field_list[3], "type");
		strcpy(field_list[4], "current");
		strcpy(field_list[5], "default");
		strcpy(field_list[6], "min");
		strcpy(field_list[7], "max");
		strcpy(field_list[8], "step");

		// store field values
		param_struct = mxCreateStructMatrix(1, 1, NUMERIC_FIELD, field_list);
		mxSetField(param_struct, 0, field_list[0], mxCreateDoubleScalar((double) attr_access));
		mxSetField(param_struct, 0, field_list[1], mxCreateString(access_string));
		mxSetField(param_struct, 0, field_list[2], mxCreateDoubleScalar((double) attr_type));
		mxSetField(param_struct, 0, field_list[3], mxCreateString(type_string));
		mxSetField(param_struct, 0, field_list[4], mxCreateDoubleScalar(param_value));
		mxSetField(param_struct, 0, field_list[5], mxCreateDoubleScalar(param_default));
		mxSetField(param_struct, 0, field_list[6], mxCreateDoubleScalar(param_min));
		mxSetField(param_struct, 0, field_list[7], mxCreateDoubleScalar(param_max));
		mxSetField(param_struct, 0, field_list[8], mxCreateDoubleScalar(param_inc));
		pvcam_destroy_array(field_list,	NUMERIC_FIELD);
	}

	// return parameter structure
	return(param_struct);
}


// obtain enumerated parameter
mxArray *pvcam_param_enum(int16 hcam, uns32 param_id) {

	// declarations
	char	*access_string;	// access string
	char	*type_string;	// type string
	double	param_value;	// parameter value
	double	param_default;	// default value

	rs_bool	success = 1;	// flag for successful execution
	char	*enum_string;	// enumerated parameter string
	double	*enum_index;	// pointer to enumerated parameter values
	int32	enum_value;		// enumerated parameter value
	mxArray	*enum_list;		// list of enumerated parameter strings
	mxArray	*enum_array;	// array of enumerated parameter values
	uns32	enum_length;	// enumerated string length
	uns32	i;				// loop counter

	mxArray	*param_struct;	// output structure
	char	**field_list;	// field names for output structure

	// obtain strings and check for errors
	if ((access_string = pvcam_access_string(hcam, attr_access)) == NULL) {
		param_struct = NULL;
	}
	else if ((type_string = pvcam_type_string(hcam, attr_type)) == NULL) {
		param_struct = NULL;
	}

	// obtain values and check for errors
	else if (!pvcam_param_value(hcam, param_id, ATTR_CURRENT, attr_type, &param_value)) {
		param_struct = NULL;
	}
	else if (!pvcam_param_value(hcam, param_id, ATTR_DEFAULT, attr_type, &param_default)) {
		param_struct = NULL;
	}

	else {

		// create storage for enumerated strings & values
		enum_list = mxCreateCellMatrix(1, (int) attr_count);
		enum_array = mxCreateDoubleMatrix(1, (int) attr_count, mxREAL);
		enum_index = mxGetPr(enum_array);

		// loop through indices to collect enumerated values & strings
		// allocate storage for strings by obtaining string length first
		for (i = 0; i < attr_count; i++) {
			if (pl_enum_str_length(hcam, param_id, i, &enum_length)) {
				enum_string = (char *) mxCalloc((size_t) enum_length, sizeof(char));
			}
			else {
				pvcam_error(hcam, "Error obtaining enumerated string length");
				success = 0;
				break;
			}
			if (pl_get_enum_param(hcam, param_id, i, &enum_value, enum_string, enum_length)) {
				enum_index[i] = (double) enum_value;
				mxSetCell(enum_list, i, mxCreateString(enum_string));
			}
			else {
				pvcam_error(hcam, "Error obtaining enumerated string");
				success = 0;
				break;
			}
			mxFree((void *) enum_string);
		}

		if (success) {

			// assign field names
			field_list = pvcam_create_array(ENUM_FIELD, FIELD_SIZE);
			strcpy(field_list[0], "access_id");
			strcpy(field_list[1], "access");
			strcpy(field_list[2], "type_id");
			strcpy(field_list[3], "type");
			strcpy(field_list[4], "current");
			strcpy(field_list[5], "default");
			strcpy(field_list[6], "enumlist");
			strcpy(field_list[7], "enumindex");

			// store field values for scalar/string values
			param_struct = mxCreateStructMatrix(1, 1, ENUM_FIELD, field_list);
			mxSetField(param_struct, 0, field_list[0], mxCreateDoubleScalar((double) attr_access));
			mxSetField(param_struct, 0, field_list[1], mxCreateString(access_string));
			mxSetField(param_struct, 0, field_list[2], mxCreateDoubleScalar((double) attr_type));
			mxSetField(param_struct, 0, field_list[3], mxCreateString(type_string));
			mxSetField(param_struct, 0, field_list[4], mxCreateDoubleScalar(param_value));
			mxSetField(param_struct, 0, field_list[5], mxCreateDoubleScalar(param_default));
			mxSetField(param_struct, 0, field_list[6], enum_list);
			mxSetField(param_struct, 0, field_list[7], enum_array);
			pvcam_destroy_array(field_list, ENUM_FIELD);
		}
	}

	// return parameter structure
	return(param_struct);
}
