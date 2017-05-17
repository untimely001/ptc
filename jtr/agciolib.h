/*****************************************************************
AGCIOLIB include file
******************************************************************/
#ifndef	_AGCIOLIB_H_
#define	_AGCIOLIB_H_
/*****************************************************************
**              Copyright (C) 1997, SunStep, Inc.
******************************************************************
** File:   agciolib.h
** Desc:   Function definitions for AGC PORT Input/Output drivers 
**         on Windows NT
******************************************************************
** First Pass ......................... Li Laizhan. 1997.7.7
******************************************************************/
/*****************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************/
//  globals 
//  no...

//	AGCIOLIB API's Definitions 
// Init & Exit
int AgcInitIO(void);
int AgcExitIO(void);

// Input
int AgcInport(unsigned short port);
unsigned short AgcInportw(unsigned short port);
unsigned long AgcInportd(unsigned short port);

// Output
int AgcOutport(unsigned short port,int b);
unsigned short AgcOutportw(unsigned short port,unsigned short i);
unsigned long AgcOutportd(unsigned short port,unsigned long d);

/******************************************************************/
#ifdef __cplusplus
}
#endif

#endif	//_AGCIOLIB_H_
