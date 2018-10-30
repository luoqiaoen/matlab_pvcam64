# matlab_pvcam64

64bit PVCam 3.1 MATLAB functions library using mex.

Adapted a lot from the original 32bit version from matPVCAM source, Year 2003 http://www.eng.utoledo.edu/~smolitor/download.htm.
deleted pv_icl(not needed for my project), and replaced all 32bit library with 64bit library from Photometrics (Currently PVCAM 3.1).
exchange boolean to rs_bool for Visual Studio complier.

## Instruction:
Compile in MATLAB terminal, under the directory:

mex pvcam64.lib pvcamopen.c pvcamutil.c

## Compatible Cameras:
tested on CoolSNAP HQ, Retiga LUMO and PRIME M

For PRIME and PVCAM 3.1, meta is enabled, current files can be directly used for PRIME, adaptation may be needed for older cameras.
Post processing features can be controlled.

As my application does not require advanced features, the codes written are bare-boned, 
but should have provided sufficient examples to help users make full use of the current PVCAM SDK.
