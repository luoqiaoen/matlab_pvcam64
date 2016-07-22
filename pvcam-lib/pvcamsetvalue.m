function flag = pvcamsetvalue(h_cam, param_id, param_value);

% PVCAMSETVALUE - sets parameter value on PVCAM camera
%
%    FLAG = PVCAMSETVALUE(HCAM, ID, VALUE) set the VALUE of the parameter
%    specified by the string ID on the PVCAM camera HCAM and returns FLAG
%    to indicate success (FLAG = 1) or failure (FLAG = 0).  See the PVCAM
%    manual for valid parameter IDs and values.
%
%    Although the user may call PVCAMSET.DLL directly, this code checks
%    that numeric parameter values are within range, and allows users to
%    provide a numeric index or string value for enumerated parameters.

% 3/27/03 SCM

% validate arguments
flag = 0;
if (nargin ~= 3)
    warning('type ''help pvcamsetvalue'' for syntax');
    return
elseif (~isscalar(h_cam))
    warning('HCAM must be a scalar');
    return
elseif (~ischar(param_id) | isempty(param_id))
    warning('ID must be a string');
    return
elseif ((~ischar(param_value) & ~isscalar(param_value)) | isempty(param_value))
    warning('VALUE must be a string or a scalar');
    return
end

% obtain parameter information from PVCAMGETVALUE
% check accessibility
[old_value, param_type, param_access, param_range, enum_index] = pvcamgetvalue(h_cam, param_id);
if (isempty(findstr(lower(param_access), 'write')))
    warning(sprintf('%s access is %s', param_id, param_access));
    return
end

% check limits and set parameter value based on type
switch (param_type)
    
    case 'string'
        % check parameter value is string
        if (~ischar(param_value) | isempty(param_value))
            warning(sprintf('%s type is %s, VALUE must be a string', param_id, param_type));
        else
            flag = pvcamset(h_cam, param_id, param_value);
        end
        
    case 'enumerated'
        % allow parameter value to be string or numeric
        % match string to enumerated list values
        % otherwise assume value is numeric index
        if (isscalar(param_value))
            param_index = find(enum_index == param_value);
            if (isempty(param_index))
                warning(sprintf('%d is not a valid index for %s', param_value, param_id));
            else
                flag = pvcamset(h_cam, param_id, enum_index(param_index(1)));
            end
        elseif (ischar(param_value) & ~isempty(param_value))
            param_index = strmatch(lower(param_value), lower(param_range));
            if (isempty(param_index))
                warning(sprintf('%s is not a valid value for %s', param_value, param_id));
            else
                flag = pvcamset(h_cam, param_id, enum_index(param_index(1)));
            end
        else
            warning(sprintf('%s type is %s, VALUE must be a string or a scalar', param_id, param_type));
        end
        
    otherwise
        % check for numeric parameter value
        % check parameter within range
        if (~isscalar(param_value))
            warning(sprintf('%s type is %s, VALUE must be a scalar', param_id, param_type));
        elseif (param_value < min(param_range))
            warning(sprintf('%s minimum is %d, VALUE is %d', param_id, min(param_range), param_value));
        elseif (param_value > max(param_range))
            warning(sprintf('%s maximum is %d, VALUE is %d', param_id, max(param_range), param_value));
        else
            flag = pvcamset(h_cam, param_id, param_value);
        end
end
return    
