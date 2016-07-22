function varargout = pvcamgetvalue(h_cam, param_id);

% PVCAMGETVALUE - gets parameter value from PVCAM camera
%
%    VALUE = PVCAMGETVALUE(HCAM, ID) obtains the parameter specified by
%    the string ID from the PVCAM camera HCAM and returns VALUE.  See the
%    PVCAM manual for valid parameter IDs.
%
%    [VALUE, TYPE, ACCESS, RANGE] = PVCAMGETVALUE(HCAM, ID) returns the
%    parameter TYPE, read/write ACCESS, and all acceptable parameter values
%    in RANGE.  RANGE is two element vector if ID is numeric, a cell array
%    of strings if ID is enumerated, and is 'string' if ID is a string.
%
%    Although the user may call PVCAMGET.DLL directly, this code returns the
%    current parameter value rather than the entire parameter structure
%    returned by PVCAMGET, and may be useful for certain implementations.

% 3/27/03 SCM

% initialize outputs
if (nargout > 0)
    varargout = cell(1, nargout);
end

% validate arguments
if (nargin ~= 2)
    warning('type ''help pvcamgetvalue'' for syntax');
    return
elseif (~isscalar(h_cam))
    warning('HCAM must be a scalar');
    return
elseif (~ischar(param_id) | isempty(param_id))
    warning('ID must be a string');
    return
end

% obtain structure from PVCAMGET.DLL
param_struct = pvcamget(h_cam, param_id);
if (isstruct(param_struct))
    switch (lower(param_struct.type))
        case 'string'
            param_value = param_struct.current;
            param_range = param_struct.type;
        case 'enumerated'
            param_range = param_struct.enumlist;
            param_index = find(param_struct.enumindex == param_struct.current);
            param_value = param_range{param_index(1)};
        otherwise
            param_value = param_struct.current;
            param_range = [param_struct.min param_struct.max];
    end
    
    for i = 1 : nargout
        switch (i)
            case 1
                varargout{i} = param_value;
            case 2
                varargout{i} = param_struct.type;
            case 3
                varargout{i} = param_struct.access;
            case 4
                varargout{i} = param_range;
            case 5
                if (isfield(param_struct, 'enumindex'))
                    varargout{i} = param_struct.enumindex;
                else
                    varargout{i} = [];
                end
            otherwise
                varargout{i} = [];
        end
    end
end
return    
