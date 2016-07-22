function [] = CleanUpCamera( hcam )
% Close PVCAM interface and clean up camera

success = pvcamclose(hcam);
if success ==1
         disp([datestr(datetime('now')) ':camera is closed'])
end
end

