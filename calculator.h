#ifndef CALCULATOR_H
#define CALCULATOR_H

#ifndef bool
#define bool char
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#ifndef PI
#define PI 3.141592653589793
#endif
#ifndef NAN
#define NAN (0.0/0.0)
#endif
#ifndef INFINITY
#define INFINITY (1.0/0.0)
#endif


#define MAXTOKENLENGTH 512
#define MAXPRECISION 20
#define DEFAULTPRECISION 5
#define AUTOPRECISION -1
#define FUNCTIONSEPARATOR "|"


#ifdef CALCULATOR_AS_LIB
int calculator_init();
int calculator_setprecision();
int calculator_settokenlength();
int calculator_setprefix();
int calculator_calc(const char* formula, double* result);
#endif


#endif
