function [h_cam, pvcam_par,roi_struct] = InitializeCamera()
%% not input, open camera and retreive some pvcam parameters, and return a ROI structure based on the parameters.
%% some parameters
pvcam_getpar = {'PARAM_BIT_DEPTH', 'PARAM_CHIP_NAME', ...
    'PARAM_PAR_SIZE', 'PARAM_SER_SIZE', 'PARAM_PREMASK', 'PARAM_TEMP', 'PARAM_GAIN_INDEX',...
    'PARAM_PIX_TIME','PARAM_EXP_RES','PARAM_PIX_TIME'};
pvcam_setpar = {'PARAM_CLEAR_MODE', 'PARAM_CLEAR_CYCLES', 'PARAM_GAIN_INDEX', ...
    'PARAM_PMODE', 'PARAM_SHTR_OPEN_MODE', 'PARAM_SHTR_CLOSE_DELAY', 'PARAM_SHTR_OPEN_DELAY', ...
    'PARAM_SPDTAB_INDEX', 'PARAM_TEMP_SETPOINT','PARAM_PIX_TIME'};

pvcam_para_value = {[],[],[],[],[],[],[]};
pvcam_para_field = {'serdim','pardim','gain','speedns','timeunit','temp','readout'};
pvcam_par = cell2struct(pvcam_para_value, pvcam_para_field, 2);
%% open camera
h_cam = pvcamopen(0);
if (isempty(h_cam))
     disp([datestr(datetime('now')) ':could not open camera']);
    pvcamclose(1);
else
    disp([datestr(datetime('now')) ':camera detected']);
end

pvcamset(h_cam,pvcam_setpar{7},1);%10Mhz readout mode Readout Port 2 is 1, Port 1 is 0
pvcam_par.serdim   = pvcamgetvalue(h_cam, pvcam_getpar{4});%CCDpixelser
pvcam_par.pardim   = pvcamgetvalue(h_cam, pvcam_getpar{3});%CCDpixelpar
pvcam_par.gain     = pvcamgetvalue(h_cam, pvcam_getpar{7});%CCDgainindex
pvcam_par.speedns  = pvcamgetvalue(h_cam, pvcam_getpar{8});%CCDpixtime
pvcam_par.timeunit = pvcamgetvalue(h_cam, pvcam_getpar{9});%CameraResolution
pvcam_par.temp     = pvcamgetvalue(h_cam, pvcam_getpar{6});%temperature
pvcam_par.readout  = pvcamgetvalue(h_cam, pvcam_getpar{10});%readout rate 50 means 20MHz, 100 mens 10MHz
if pvcam_par.timeunit == 'One Millisecond'
    disp([datestr(datetime('now')) ':exposure in milliseconds']);
else
     disp([datestr(datetime('now')) ':NOT in milliseconds!']);
end
    
roi_name = {'s1','s2','sbin','p1','p2','pbin'};
roi_value = {0, pvcam_par.serdim-1, 1, 0, pvcam_par.pardim-1, 1};
roi_struct = cell2struct(roi_value, roi_name, 2);
end

