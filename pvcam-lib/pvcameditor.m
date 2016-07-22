function varargout = pvcameditor(h_cam, param_id)

% PVCAMEDITOR - allow user to edit PVCAM parameters
%
%    PVCAMEDITOR(HCAM, ID) allows users to edit the PVCAM parameters
%    specified by the cell array ID from the PVCAM camera HCAM in a
%    graphical user interface.
%
%    [VALUE, FLAG, STRUCT] = PVCAMEDITOR(HCAM, ID) returns the edited
%    parameter VALUEs, a FLAG to indicate OK or CANCEL, and the STRUCT
%    in native format for the GUISTRUCT routine that is called.

% 5/27/03 SCM

% initialize output
if (nargout > 0)
    varargout = cell(1, nargout);
end

% validate arguments
if (nargin ~= 2)
    warning('MATLAB:pvcameditor', 'type ''help pvcameditor'' for syntax');
    return
elseif (~isscalar(h_cam))
    warning('MATLAB:pvcameditor', 'HCAM must be a scalar');
    return
elseif (~iscellstr(param_id) || isempty(param_id))
    warning('MATLAB:pvcameditor', 'ID must be a cell array of strings');
    return
end

% construct a structure for PVCAM parameters and limits
element_field = {'name', 'value', 'lower', 'upper', 'title', 'format', 'multi'};
element_list = [];
for i = 1 : length(param_id)
    [param_value, param_type, param_access, param_range] = pvcamgetvalue(h_cam, param_id{i});
    if (isempty(param_value))
        warning('MATLAB:pvcameditor', 'PVCAMGETVALUE could not obtain value of %s', upper(param_id{i}));
    elseif (isempty(findstr(lower(param_access), 'write')))
        warning('MATLAB:pvcameditor', '%s access is %s', upper(param_id), upper(param_access));
    else
        
        % determine format and multi flag for GUISTRUCT
        % create numeric parameter ranges for string/enumerated
        switch (param_type)
            case 'string'
                param_format = '%s';
                param_multi = 'string';
                param_range = [-Inf Inf];
            case 'enumerated'
                param_format = param_range;
                param_multi = 'enumerated';
                param_range = [1 length(param_format)];
            case 'double'
                param_format = '%g';
                param_multi = 'scalar';
            otherwise
                param_format = '%d';
                param_multi = 'scalar';
        end
        
        % strip away 'PARAM_' for parameter title
        param_title = param_id{i};
        if (~isempty(strmatch('PARAM_', param_id{i})))
            param_title = param_title(7 : end);
        end
        
        % create element for GUISTRUCT native format
        element_value = {param_id{i}, param_value, min(param_range), max(param_range), param_title, param_format, param_multi};
        if (isempty(element_list))
            element_list = cell2struct(element_value, element_field, 2);
        else
            element_list(end + 1) = cell2struct(element_value, element_field, 2);
        end
    end    
end

% call GUISTRUCT to build graphical editor
% call PVCAMSETVALUE to set parameter values
[edit_list, edit_flag] = guistruct('PVCAM Parameters', element_list);
if (edit_flag)
    for i = 1 : length(edit_list)
        pvcamsetvalue(h_cam, edit_list(i).name, edit_list(i).value);
    end
end

% return outputs if specified
for i = 1 : nargout
    switch (i)
        case 1
            varargout{i} = {edit_list(:).value};
        case 2
            varargout{i} = edit_flag;
        case 3
            varargout{i} = edit_list;
        otherwise
            varargout{i} = [];
    end
end
return
