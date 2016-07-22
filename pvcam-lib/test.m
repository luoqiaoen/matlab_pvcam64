h_cam = pvcamopen(0); % open camera connection 
x_size = pvcamgetvalue(h_cam, 'PARAM_SER_SIZE'); % obtain size of serial register 
y_size = pvcamgetvalue(h_cam, 'PARAM_PAR_SIZE'); % obtain size of parallel register 
pvcamsetvalue(h_cam, 'PARAM_SPDTAB_INDEX', 2); % set camera to max readout speed 
pvcamsetvalue(h_cam, 'PARAM_GAIN_INDEX', 2); % set camera to max gain 

% set the ROI structure to full camera array with full spatial detail (no binning) 
roi_struct = cell2struct({0, x_size - 1, 1, 0, y_size - 1, 1}, {'s1', 's2', 'sbin', 'p1', 'p2', 'pbin'}, 2); 

% initialize filter wheel and open shutter 
lambdactrl('initialize'); % connect to Lambda 10-2 
lambdactrl('callback', 'IMAGE_ACQ'); % set callback function to IMAGE_ACQ 
lambdactrl('open'); % open illumination shutter 
return 

% IMAGE_ACQ (save as separate script file) 
% acquire a single image 
image_data = pvcamacq(h_cam, 1, roi_struct, 100, 'timed'); % acquire image 
lambdactrl('callback', ' '); % no need to wait for shutter to close 
lambdactrl('close'); % close illumination shutter 
pvcamclose(h_cam); % close camera 
image_data = roiparse(image_data, roi_struct); 
image(image_data / 16); % display image (convert from 12 bit to 8 bit before display)