/*
 * CommonFlags.h
 *
 *  Created on: 14 May 2012
 *      Author: aaw35
 */

#ifndef COMMONFLAGS_H_
#define COMMONFLAGS_H_

#include <fst/fstlib.h>

DEFINE_string(error_function, "bleu","Error function to optimise");
DEFINE_string(lambda, "", "starting point");

#endif /* COMMONFLAGS_H_ */
