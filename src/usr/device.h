#ifndef _DEVICE_H
#define _DEVICE_H
#include "stm32f30x.h"  


#ifdef  DEFIN_DEVIVE_EXTERN
	#define  DEVIVE_EXTERN
#else 
	#define  DEVIVE_EXTERN extern 
#endif
	
/*
0�Ǵ������ǵ� �豸
1������һ��		
2�Ǵ��������豸
*/
DEVIVE_EXTERN uint8_t g_ucDeviceNum ;  

//#define IS_MAIN_CONTROLLOR

#ifdef  IS_MAIN_CONTROLLOR  //USB  device
	
	
#else 
	
	
#endif
  
#define    ANSINUMBER    2
#define		USE_MAGNETIC_SENSER 	
#ifdef USE_MAGNETIC_SENSER
    #define MAGSECTOR       7
#else

#define     MAGSECTOR       4

#endif


#endif
  