% PVCAMACQ - acquire image sequence from PVCAM device
%
%     DATA = PVCAMACQ(HCAM, NI, ROI, EXPTIME, EXPMODE) acquires an image
%     sequence of NI images over the CCD region(s) specified by the structure
%     array ROI from the camera specified by HCAM.  The exposure time is
%     specified by EXPTIME; the units depend on the PARAM_EXP_RES and the
%     PARAM_EXP_RES_INDEX settings.  The exposure mode ('timed', 'trigger',
%     'strobe', 'bulb' or 'flash') is provided by EXPMODE.  The structure
%     array ROI must have the following scalar fields:
%
%					s1 = first serial register
%					s2 = last serial register
%					sbin = serial binning factor
%					p1 = first parallel register
%					p2 = last parallel register
%					pbin = parallel binning factor
%
%	  The length of the structure array ROI determines the number of CCD
%	  regions that will be imaged.  If successful, DATA will be a vector
%	  (unsigned 16-bit integer) containing the data from the image sequence.
%	  The calling routine must reshape this vector based upon ROIs and images
%	  in the sequence.  If unsuccessful, DATA = [].

% 2/19/03 SCM
% mex DLL code
