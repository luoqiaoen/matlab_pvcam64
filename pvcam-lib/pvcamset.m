% PVCAMSET - set PVCAM device parameters
%
%    FLAG = PVCAMSET(HCAM, PARAM, VALUE) assigns VALUE to the PVCAM
%    parameter specified by the string PARAM for the camera specified
%    by HCAM.  FLAG returns 0 if an error occurred, 1 if the program
%    was successful.  The user should call PVCAMGET() first to obtain
%    acceptable values for the specified parameter.  See the PVCAM
%    manual for valid parameter names.

% 2/17/03 SCM
% mex DLL code
