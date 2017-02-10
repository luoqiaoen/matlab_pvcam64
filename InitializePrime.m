function [h_cam, pvcam_par,roi_struct] = InitializePrime()
%% not input, open camera and retreive some pvcam parameters, and return a ROI structure based on the parameters.
%% some parameters
pvcam_getpar =...
    {'PARAM_CLEAR_CYCLES', 'PARAM_CLEAR_MODE',...
    'PARAM_COOLING_MODE','PARAM_TEMP',...
    'PARAM_TEMP_SETPOINT','PARAM_FAN_SPEED_SETPOINT',...
    'PARAM_EXP_TIME','PARAM_METADATA_ENABLED',...
    'PARAM_CENTROIDS_ENABLED','PARAM_CENTROIDS_COUNT',...
    'PARAM_PAR_SIZE','PARAM_SER_SIZE',...
    'PARAM_PIX_PAR_DIST','PARAM_PIX_SER_DIST',...
    'PARAM_PIX_PAR_SIZE','PARAM_PIX_SER_SIZE',...
    'PARAM_ROI_COUNT','PARAM_EXP_RES'};

pvcam_setpar =...
    {'PARAM_CLEAR_CYCLES', 'PARAM_CLEAR_MODE',...
    'PARAM_COOLING_MODE','PARAM_TEMP',...
    'PARAM_TEMP_SETPOINT','PARAM_FAN_SPEED_SETPOINT',...
    'PARAM_EXP_TIME','PARAM_METADATA_ENABLED',...
    'PARAM_CENTROIDS_ENABLED','PARAM_CENTROIDS_COUNT'};


% Default Timed Mode, internal time stamp
% for PRIME, enable PrimeEnhance: PP_FEATURE_DENOISING, for multiple ROI or Prime locate:
% PARAM_METADATA_ENABLED, PARAM_CENTROIDS_ENABLED,PARAM_CENTROIDS_COUNT, PARAM_CENTROIDS_RADIUS
pvcam_para_value = {[],[],[],[]};
pvcam_para_field = {'serdim','pardim','timeunit','temp'};
pvcam_par = cell2struct(pvcam_para_value, pvcam_para_field, 2);
%% open camera
h_cam = pvcamopen(0);
if (isempty(h_cam))
     disp([datestr(datetime('now')) ':could not open camera']);
    pvcamclose(1);
else
    disp([datestr(datetime('now')) ':camera detected']);
end

pvcamset(h_cam,pvcam_setpar{8},1);%Enable Metadata
for i = 0:4
    pvcamppselect(h_cam, i, 0, 0);
    % make sure despeckle and denoising is off for speckle imaging
end

pvcam_par.serdim   = pvcamgetvalue(h_cam, pvcam_getpar{12});%CCDpixelser
pvcam_par.pardim   = pvcamgetvalue(h_cam, pvcam_getpar{11});%CCDpixelpar
pvcam_par.timeunit = pvcamgetvalue(h_cam, pvcam_getpar{18});%CameraResolution
pvcam_par.temp     = pvcamgetvalue(h_cam, pvcam_getpar{4});%temperature
if strcmp(pvcam_par.timeunit,'One Millisecond') == 1
    disp([datestr(datetime('now')) ':exposure in milliseconds']);
else
     disp([datestr(datetime('now')) ':NOT in milliseconds!']);
end
    
roi_name = {'s1','s2','sbin','p1','p2','pbin'};
roi_value = {0, pvcam_par.serdim-1, 1, 0, pvcam_par.pardim-1, 1};
roi_struct = cell2struct(roi_value, roi_name, 2);
end

