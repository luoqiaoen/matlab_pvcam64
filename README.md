# matpvcam64
64bit PVCam 3.1 MATLAB library using mex

Original 32bit version from matPVCAM source http://www.eng.utoledo.edu/~smolitor/download.htm.
deleted pv_icl(not needed for my project), and replaced all 32bit library with 64bit library from Photometrics (Currently PVCAM 3.1).
exchange boolean to rs_bool for Visual Studio complier.

Sample Compile code in MATLAB terminal, under the directory:
mex pvcam64.lib pvcamopen.c pvcamutil.c

tested on CoolSNAP HQ, PRIME M and Retiga LUMO

For PRIME and PVCAM 3.1, meta is enabled, current files can be directly used for PRIME, adaptation may be needed for older cameras.
