/*****************************************************************************/
/*        Copyright (C) Roper Scientific, Inc. 2007 All rights reserved.     */
/*****************************************************************************/
#ifndef _MASTER_H
#define _MASTER_H

/* This allows us to insert the proper compiler flags, in pvcam.h for example,
   to cope properly with C++ definitions. */
#if defined(__cplusplus)
  /* BORLAND C++ and GCC */
  #define PV_C_PLUS_PLUS
#elif defined(__cplusplus__)
  /* MICROSOFT C++ */
  #define PV_C_PLUS_PLUS
#endif

#if defined(_MSC_VER)
  /* MICROSOFT C++ VS 2005 wants to use safe string functions.
     We have to turn this off for now. */
  #pragma message("Disabling String Safe Warnings")
  #pragma warning(disable : 4996)
#endif

/**************************** Calling Conventions ****************************/
#if defined(_WIN32) || defined(_WIN64)
  #define PV_DECL __stdcall
#elif defined(__linux__)
  #define PV_DECL
#elif defined(__APPLE__)
  #error TODO: Declare PV_DECL calling convention
#endif

#define PV_PTR_DECL  *

/******************************** PVCAM Types ********************************/
enum { PV_FAIL = 0, PV_OK };

typedef unsigned short rs_bool, PV_PTR_DECL  rs_bool_ptr;
typedef char                    PV_PTR_DECL  char_ptr;
typedef signed char    int8,    PV_PTR_DECL  int8_ptr;
typedef unsigned char  uns8,    PV_PTR_DECL  uns8_ptr;
typedef short          int16,   PV_PTR_DECL  int16_ptr;
typedef unsigned short uns16,   PV_PTR_DECL  uns16_ptr;
typedef long           int32,   PV_PTR_DECL  int32_ptr;
typedef unsigned long  uns32,   PV_PTR_DECL  uns32_ptr;
typedef double         flt64,   PV_PTR_DECL  flt64_ptr;
typedef void                    PV_PTR_DECL  void_ptr;
typedef void_ptr                PV_PTR_DECL  void_ptr_ptr;

#if defined(_MSC_VER)
  typedef unsigned __int64   ulong64, PV_PTR_DECL ulong64_ptr;
  typedef signed   __int64   long64,  PV_PTR_DECL long64_ptr;
#else
  typedef unsigned long long ulong64, PV_PTR_DECL ulong64_ptr;
  typedef signed   long long long64,  PV_PTR_DECL long64_ptr;
#endif

typedef const rs_bool PV_PTR_DECL rs_bool_const_ptr;
typedef const char    PV_PTR_DECL char_const_ptr;
typedef const int8    PV_PTR_DECL int8_const_ptr;
typedef const uns8    PV_PTR_DECL uns8_const_ptr;
typedef const int16   PV_PTR_DECL int16_const_ptr;
typedef const uns16   PV_PTR_DECL uns16_const_ptr;
typedef const int32   PV_PTR_DECL int32_const_ptr;
typedef const uns32   PV_PTR_DECL uns32_const_ptr;
typedef const flt64   PV_PTR_DECL flt64_const_ptr;

/****************************** PVCAM Constants ******************************/
#ifndef FALSE
  #define FALSE  PV_FAIL    /* FALSE == 0                                    */
#endif

#ifndef TRUE
  #define TRUE   PV_OK      /* TRUE  == 1                                    */
#endif

#define CAM_NAME_LEN     32 /* Max length of a cam name (includes null term) */
#define PARAM_NAME_LEN   32 /* Max length of a pp param                      */

/************************ PVCAM-Specific Definitions *************************/
#define MAX_CAM          16 /* Maximum number of cameras on this system.     */

#endif /* _MASTER_H */
