% PVCAMGET - obtain PVCAM device parameters
%
%    STRUCT = PVCAMGET(HCAM, PARAM) returns a structure containing
%    information about the PVCAM parameter specified by the string
%    PARAM for the camera specified by HCAM.  See the PVCAM manual
%    for valid parameter names.
%
%    If successful, STRUCT will contain the following fields:
%
%               access:     read/write flag (string)
%               access_id:  read/write flag (numeric)
%               type:       parameter type (string)
%               type_id:    parameter type (numeric)
%               current:    current parameter value
%
%    If the parameter is a string, STRUCT will contain no additional
%    fields.  If the parameter is numeric, STRUCT will contain the
%    following fields:
%
%               default:    default parameter value
%               min:        minimum parameter value
%               max:        maximum parameter value
%               step:       parameter increment (not used)
%
%    If the parameter is enumerated, then the parameter value is an
%    integer index that corresponds to a enumerated list of possible
%    parameter values, and STRUCT will contain the following fields:
% 
%               default:    default parameter index
%               enumlist:   cell array of possible parameter values
%               enumindex:  indices of possible parameter values

% 2/17/03 SCM
% mex DLL code
