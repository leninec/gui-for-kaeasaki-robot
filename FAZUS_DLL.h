#ifndef MYDLLH
#define MYDLLH
//#define BYTE  unsigned char
typedef unsigned char       BYTE;

extern "C" _declspec(dllexport) int Contr_Start(char *);
extern "C" _declspec(dllexport) int Contr_Stop(void);
extern "C" _declspec(dllexport) int AScan_Rec(BYTE *, BYTE *, BYTE *, BYTE *, BYTE *, BYTE *, BYTE *, BYTE *, int);
extern "C" _declspec(dllexport) int Def_Rec(BYTE *);

#endif
