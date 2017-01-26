function [ image ] = AcquireImage(h_cam, roi_struct)
%SIMPLE ACQUIRE SINGLE IMAGE WITH 1000MS EXPOSURE TIME
%   Detailed explanation goes here

if pvcamgetvalue(h_cam, 'PARAM_METADATA_ENABLED')
    exptime = 100;
    image_stream = pvcamacq(h_cam, 1, roi_struct, exptime, 'timed');
    if max(image_stream)== 4095
        error('TOO MUCH EXPOSURE, Picture may saturate!')
    end
    disp([datestr(datetime('now')) ' picture acquired']);
    image = image_stream(41:end);
    %meta  = image_stream(1:40);
    mean(image)
else
    disp('Metadata not enabled!')
end
end
