# matpvcam64
64bit pvcam matlab library using mex

Original 32bit version from matPVCAM source http://www.eng.utoledo.edu/~smolitor/download.htm.
deleted pv_icl(not needed for my project), and replaced all 32bit library with 64bit library from Photometrics.
exchange boolean to rs_bool for Visual Studio complier.

Sample Compile code in MATLAB terminal:
mex -largeArrayDims '-I:\Dropbox\Workspace\Matlab\Current\MATLAB_mex\MATLAB_mex' pvcam64.lib pvcamopen.c pvcamutil.c

tested on CoolSNAP HQ, PRIME M and Retiga LUMO
