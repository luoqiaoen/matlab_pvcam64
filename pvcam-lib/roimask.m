function image_mask = roimask(roi_struct);

% ROIMASK - creates a mask array to indicate pixel locations
%
%    MASK = ROIMASK(ROI) creates a 2D logical array MASK to indicate the
%    physical locations of pixels specified by the ROI structure array.
%    This is useful for determining the readout order of pixels in the data
%    stream following image acquisition.
%
%    Note the mask is transposed, with parallel registers being stored as
%    columns.  For display purposes, the user must transpose the array to
%    display parallel registers as rows.

% 1/5/04 SCM

% limit coordinates to array size
s1 = max(min([roi_struct(:).s1; roi_struct(:).s2]), 0);
s2 = max(max([roi_struct(:).s1; roi_struct(:).s2]), 0);
p1 = max(min([roi_struct(:).p1; roi_struct(:).p2]), 0);
p2 = max(max([roi_struct(:).p1; roi_struct(:).p2]), 0);

% set binning to minimum binning parameter found
% limit binning to minimum ROI size
% determine image mask size based on ROI extent
sbin = min(max(min([roi_struct(:).sbin]), 1), min(s2 - s1 + 1));
pbin = min(max(min([roi_struct(:).pbin]), 1), min(p2 - p1 + 1));

% create image mask
image_size = floor([(max(s2) - min(s1) + 1) (max(p2) - min(p1) + 1)] ./ [sbin pbin]);
image_mask = zeros(image_size);
i_ser = floor(([s1; s2] - min(s1)) / sbin) + 1;
i_par = floor(([p1; p2] - min(p1)) / pbin) + 1;
for i = 1 : min(size(i_ser, 2), size(i_par, 2))
    image_mask(i_ser(1, i) : i_ser(2, i), i_par(1, i) : i_par(2, i)) = 1;
end
return
