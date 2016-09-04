//
// Created by skr on 03.09.16.
//

#ifndef _ASSERTION_H
#define _ASSERTION_H

#include <stdio.h>

// debug macros
#define ASSERT_PAR(_Expr) {if ( !_Expr ) printf("Bad param %s, %u", __FILE__, __LINE__);}
#define ASSERT(_Expr) {if ( !_Expr ) printf("Bad check %s, %u", __FILE__, __LINE__);}

// Error Codes
enum {
    ERR_OK  = 0,
    ERR_FAIL   ,
    ERR_MEM    ,
    ERR_NOT_FOUND,
    ERR_MAX
};

#endif //_ASSERTION_H
