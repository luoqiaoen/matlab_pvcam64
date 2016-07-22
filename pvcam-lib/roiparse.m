function image_array = roiparse(image_stream, old_struct)

% ROIPARSE - separate pixels from multiple ROIs
%
%    IMAGE = ROIPARSE(STREAM, ROI) parses pixels from the image data in the
%    vector STREAM based on the contents of the ROI structure.  Images from
%    individual ROIs are returned in the array IMAGE.
%
%    Images are displayed with parallel registers being stored as rows.
%    IMAGE will be 3-D if images from multiple acquisitions are stored in
%    the stream.
%
%    To insure valid ROI coordinates, this code assumes that the user has
%    called ROIOVERLAP.M before acquiring images.

% 2/26/03 SCM
% MOD 1/5/04 SCM

% validate arguments
image_array = [];
roi_fields = {'s1', 's2', 'sbin', 'p1', 'p2', 'pbin'};
if (nargin ~= 2)
    warning('MATLAB:roiparse', 'type ''help roiparse'' for syntax');
    return
elseif (~isnumeric(image_stream) || isempty(image_stream))
    warning('MATLAB:roiparse', 'STREAM must be a numeric array');
    return
elseif (~isstruct(old_struct))
    warning('MATLAB:roiparse', 'ROI must be a structure array');
    return
elseif (~all(ismember(roi_fields, fieldnames(old_struct))))
    warning('MATLAB:roiparse', 'ROI must have fields s1, s2, sbin, p1, p2 and pbin');
    return
end

% call ROIOVERLAP to create valid ROI structure
ser_size = ceil(max([old_struct(:).s1 old_struct(:).s2])) + 1;
par_size = ceil(max([old_struct(:).p1 old_struct(:).p2])) + 1;
new_struct = roioverlap(old_struct, ser_size, par_size);

% limit coordinates to array size
s1 = min(max(min(floor([new_struct(:).s1; new_struct(:).s2])), 0), ser_size - 1);
s2 = min(max(max(floor([new_struct(:).s1; new_struct(:).s2])), 0), ser_size - 1);
p1 = min(max(min(floor([new_struct(:).p1; new_struct(:).p2])), 0), par_size - 1);
p2 = min(max(max(floor([new_struct(:).p1; new_struct(:).p2])), 0), par_size - 1);

% set binning to minimum binning parameter found
% limit binning to minimum ROI size
% determine image mask size based on ROI extent
sbin = min(max(min(floor([new_struct(:).sbin])), 1), min(s2 - s1 + 1));
pbin = min(max(min(floor([new_struct(:).pbin])), 1), min(p2 - p1 + 1));
image_size = floor([(max(s2) - min(s1) + 1) (max(p2) - min(p1) + 1)] ./ [sbin pbin]);

% create image mask for readout
if (length(new_struct) == 1)
    % single ROI
    pixel_count = prod(image_size);
else
    % multiple ROIs
    % define image mask to obtain pixel indices within stream
    image_mask = zeros(image_size);
    i_ser = floor(([s1; s2] - min(s1)) / sbin) + 1;
    i_par = floor(([p1; p2] - min(p1)) / pbin) + 1;
    for i = 1 : min(length(i_ser), length(i_par))
        image_mask(i_ser(1, i) : i_ser(2, i), i_par(1, i) : i_par(2, i)) = 1;
    end
    
    % convert the mask to pixel indices via a cumulative sum
    image_mask = reshape(cumsum(reshape(image_mask, 1, numel(image_mask))), size(image_mask)) .* (image_mask > 0);
    pixel_count = max(max(image_mask));
end

% calculate number of images in stream
% make sure number of pixels is correct
image_count = floor(numel(image_stream) / pixel_count);
image_rem = mod(numel(image_stream), pixel_count);
if (image_count <= 0)
    warning('MATLAB:roiparse', 'insufficient pixels in STREAM (%d pixels) to fill ROI (%d pixels)', ...
        numel(image_stream), pixel_count);
    return
elseif (image_rem > 0)
    warning('MATLAB:roiparse', '%d excess pixels in STREAM (%d pixels) to fill ROI (%d pixels) with %d image(s)', ...
        image_rem, numel(image_stream), pixel_count, image_count);
    return
else
    % initialize image array
    % extract pixels from stream for single or multiple ROIs
    % transpose arrays to reorient image
    image_array = feval(class(image_stream), zeros([fliplr(image_size) image_count]));
    if (length(new_struct) == 1)
        % single ROI
        for k = 1 : image_count
            beg_index = prod(image_size) * (k - 1) + 1;
            end_index = prod(image_size) * k;
            image_array(:, :, k) = reshape(image_stream(beg_index : end_index), image_size)';
        end
    else
        % multiple ROIs
        for i = 1 : min(length(i_ser), length(i_par))
            pixel_list = image_mask(i_ser(1, i) : i_ser(2, i), i_par(1, i) : i_par(2, i));
            for k = 1 : image_count
                image_array(i_par(1, i) : i_par(2, i), i_ser(1, i) : i_ser(2, i), k) = ...
                    image_stream(pixel_list + pixel_count * (k - 1))';
            end
        end
    end
end
return