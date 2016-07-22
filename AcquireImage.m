function [ image_stream ] = AcquireImage(h_cam, roi_struct)
%SIMPLE ACQUIRE SINGLE IMAGE WITH 1000MS EXPOSURE TIME
%   Detailed explanation goes here
exptime = 1000; %in ms
image_stream = pvcamacq(h_cam, 1, roi_struct, exptime, 'timed');
if max(image_stream)== 4095
    error('TOO MUCH EXPOSURE, Picture may saturate!')
end
disp([datestr(datetime('now')) ' picture acquired']);
meanIntensity = mean(image_stream)
end
