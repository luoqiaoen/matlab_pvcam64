function new_struct = roioverlap(old_struct, ser_size, par_size);

% ROIOVERLAP - separate overlapping ROIs
%
%    NEW = ROIOVERLAP(OLD, SER, PAR) recreates a set of ROIs for camera
%    readout if overlap between parallel registers is detected:
%
%            #####                         #####
%            #####                         #####
%            #####  ##### (2 ROIs)  --->   ############ (3 ROIs)
%            #####  #####                  ############
%                   #####                         #####
%                   #####                         #####
%
%    For multiple ROIs, the smallest serial and parallel binning values are
%    used for all ROIs, regardless of overlap.  ROIs are also checked to
%    ensure they do not extend beyond the pixel array as specified by the
%    scalars SER and PAR, and are resized to produce integer multiples of
%    binning parameters.

% 12/31/03 SCM

% validate arguments
new_struct = [];
roi_fields = {'s1', 's2', 'sbin', 'p1', 'p2', 'pbin'};
if (nargin ~= 3)
    warning('type ''help roioverlap'' for syntax');
    return
elseif (~isstruct(old_struct))
    warning('ROI must be a structure array');
    return
elseif (~all(ismember(roi_fields, fieldnames(old_struct))))
    warning('ROI must have fields s1, s2, sbin, p1, p2 and pbin');
    return
elseif (~isscalar(ser_size))
    warning('SER must be a scalar');
    return
elseif ((ser_size <= 0) | (ser_size ~= floor(ser_size)))
    warning('SER must be a positive integer');
    return
elseif (~isscalar(par_size))
    warning('PAR must be a scalar');
    return
elseif ((par_size <= 0) | (par_size ~= floor(par_size)))
    warning('PAR must be a positive integer');
    return
end

% limit coordinates to array size
s1 = min(max(min(floor([old_struct(:).s1; old_struct(:).s2])), 0), ser_size - 1);
s2 = min(max(max(floor([old_struct(:).s1; old_struct(:).s2])), 0), ser_size - 1);
p1 = min(max(min(floor([old_struct(:).p1; old_struct(:).p2])), 0), par_size - 1);
p2 = min(max(max(floor([old_struct(:).p1; old_struct(:).p2])), 0), par_size - 1);

% set binning to minimum binning parameter found
% limit binning to minimum ROI size
sbin = min(max(min(floor([old_struct(:).sbin])), 1), min(s2 - s1 + 1));
pbin = min(max(min(floor([old_struct(:).pbin])), 1), min(p2 - p1 + 1));

% prevent overlap for multiple ROIs
% otherwise recreate single ROI
if (length(old_struct) > 1)
    
    % create image mask
    % scale ROI coordinates to mask coordinates
    image_size = floor([(max(p2) - min(p1) + 1) (max(s2) - min(s1) + 1)] ./ [pbin sbin]);
    image_mask = zeros(image_size);
    i_ser = floor(([s1; s2] - min(s1)) / sbin) + 1;
    i_par = floor(([p1; p2] - min(p1)) / pbin) + 1;
    for i = 1 : min(length(i_ser), length(i_par))
        image_mask(i_par(1, i) : i_par(2, i), i_ser(1, i) : i_ser(2, i)) = 1;
    end
    
    % find rows with ROIs
    zero_col = zeros(size(image_mask, 1), 1);
    [i_beg, j_beg] = find(diff([zero_col image_mask], 1, 2) == 1);
    [i_end, j_end] = find(diff([image_mask zero_col], 1, 2) == -1);
    par_coord = unique([i_beg; i_end]);
    j_col = [j_beg; j_end];
    for i = 1 : length(par_coord)
        index = find([i_beg; i_end] == par_coord(i));
        if (~isempty(index))
            ser_coord(i, 1 : 2) = [min(j_col(index)) max(j_col(index))];
        end
    end
    
    % find continuous S1-S2, P1-P2 segments
    all_coord = [par_coord ser_coord];
    all_flag = abs(diff([zeros(1, size(all_coord, 2)); all_coord]));
    all_flag(:, 1) = all_flag(:, 1) - 1;
    all_index = find(sum(all_flag, 2));
    
    % rescale mask coordinates back to ROI coordinates
    s1_new = sbin * (all_coord(all_index, 2) - 1) + min(s1);
    s2_new = sbin * all_coord(all_index, 3) + min(s1) - 1;
    p1_new = pbin * (all_coord(all_index, 1) - 1) + min(p1);
    p2_new = pbin * all_coord([all_index(2 : end) - 1; size(all_coord, 1)], 1) + min(p1) - 1;
    
    % prevent rescaled coordinates from exceeding pixel dimensions
    s1_new = rescaled_min_max(s1_new, 0, ser_size - 1, sbin);
    s2_new = rescaled_min_max(s2_new, 0, ser_size - 1, sbin);
    p1_new = rescaled_min_max(p1_new, 0, par_size - 1, pbin);
    p2_new = rescaled_min_max(p2_new, 0, par_size - 1, pbin);
    
    % recreate ROI structure
    for i = 1 : size(all_index, 1)
        if (i == 1)
            new_struct = cell2struct({s1_new(i), s2_new(i), sbin, p1_new(i), p2_new(i), pbin}, roi_fields, 2);
        else
            new_struct(i) = cell2struct({s1_new(i), s2_new(i), sbin, p1_new(i), p2_new(i), pbin}, roi_fields, 2);
        end
    end
    
else
    % make sure ROI sizes are integer multiple of binning
    % adjust binning if ROI size < binning
    s2 = s1 + sbin * floor((s2 - s1 + 1) / sbin) - 1;
    p2 = p1 + pbin * floor((p2 - p1 + 1) / pbin) - 1;
    new_struct = cell2struct({s1, s2, sbin, p1, p2, pbin}, roi_fields, 2);
end
return



% RESCALED_MIN_MAX
% make sure rescaled coordinates don't exceed pixel dimensions
function new_coord = rescaled_min_max(old_coord, min_val, max_val, inc_val);

new_coord = old_coord;
min_index = find(new_coord < min_val);
while (~isempty(min_index))
    new_coord(min_index) = new_coord(min_index) + inc_val;
    min_index = find(new_coord < min_val);
end
max_index = find(new_coord > max_val);
while (~isempty(max_index))
    new_coord(max_index) = new_coord(max_index) - inc_val;
    max_index = find(new_coord > max_val);
end
return
