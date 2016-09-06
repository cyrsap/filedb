#ifndef _ASSERTION_H
#define _ASSERTION_H

#include <stdio.h>

// debug macros
#define ASSERT_PAR(_Expr) {if ( !_Expr ) printf("Bad param %s, %u\n", __FILE__, __LINE__);}
#define ASSERT(_Expr) {if ( !_Expr ) printf("Bad check %s, %u\n", __FILE__, __LINE__);}
#define ASSERT_PAR_R(_Expr) { if ( !_Expr ) {printf("Bad param %s, %u\n", __FILE__, __LINE__); return ERR_BAD_PARAM;} }
#define ASSERT_PAR_R_VOID(_Expr) { if ( !_Expr ) {printf("Bad param %s, %u\n", __FILE__, __LINE__); return;} }
#define ASSERT_R(_Expr, Ret ) {if ( !_Expr ) printf("Bad check %s, %u\n", __FILE__, __LINE__); return Ret;}
#define ASSERT_FUNC_R(Res) \
                            if (Res != ERR_OK) {\
                                if ( Res < ERR_MAX ) {\
                                    printf( "%s %s:%u\n", ErrCodes[Res], __FILE__, __LINE__ );\
                                }\
                                return Res;\
                            }
#define ASSERT_FUNC( Res )  \
                            if (Res != ERR_OK) {\
                                if ( Res < ERR_MAX ) {\
                                    printf( "%s %s:%u\n", ErrCodes[Res], __FILE__, __LINE__ );\
                                }\
                            }

// Error Codes
enum {
    ERR_OK  = 0,
    ERR_FAIL   ,
    ERR_MEM    ,
    ERR_NOT_FOUND,
    ERR_BAD_PARAM,
    ERR_MAX
};

extern const char * ErrCodes[ERR_MAX];

#endif //_ASSERTION_H
