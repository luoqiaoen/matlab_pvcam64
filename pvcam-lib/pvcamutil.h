/* Utilities for PVCAM MEX files */
/* SCM 9/2/02 */


// inclusions
#include "mex.h"
#include "master.h"
#include "pvcam.h"
#include <string.h>


// definitions
#define	ERROR_MSG		((size_t) 256)
#define ACCESS_STR_LEN	32
#define TYPE_STR_LEN	32


// function prototypes

// create 2D array
char **pvcam_create_array(int nstring, int nchar);

// free 2D array
void pvcam_destroy_array(char **char_array, int nstring);

// open PVCAM camera
rs_bool pvcam_open(int16 ncamera, int16 *hcam);

// close camera
void pvcam_close(int16 hcam);

// display error message
void pvcam_error(int16 hcam, const char *err_msg);

// return access type string
char *pvcam_access_string(int16 hcam, uns16 access_id);

// return data type string
char *pvcam_type_string(int16 hcam, uns16 type_id);

// obtain parameter value recast as DOUBLE
rs_bool pvcam_param_value(int16 hcam, uns32 param_id, int16 param_attrib,  
						  uns16 param_type, double *param_value);

// return selected PVCAM parameter ID
rs_bool pvcam_param_id(int16 hcam, const char *param_name, uns32 *param_id);
