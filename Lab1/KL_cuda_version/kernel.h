#include"op.h"
#ifndef KERNEL_H_
#define KERNEL_H_

//extern "C"
struct op *hostFE (int nodeNum,float *graph,float *Dv,int *locked,int* group);

#endif /* KERNEL_H_ */
