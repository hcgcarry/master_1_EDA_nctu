all:target

CUDA_LINK_FLAGS =  -rdc=true -gencode=arch=compute_61,code=sm_61 -Xcompiler '-fPIC' 
#CUDA_COMPILE_FLAGS = --device-c -gencode=arch=compute_61,code=sm_61 -Xcompiler '-fPIC' -g -O3
CXXFLAGS = -g -Ofast -fopenmp
OBJS=main_vec_cuda.o kernel.o 
#NVCC=/usr/local/cuda-10.1/bin/nvcc
NVCC=nvcc
CUDA_COMPILE_FLAGS = -ccbin /usr/bin/gcc4.8 --device-c -gencode=arch=compute_61,code=sm_61 -Xcompiler '-fPIC' -g -O3


target: $(OBJS)
	${NVCC} ${CUDA_LINK_FLAGS} -o $@ $(OBJS) 


%.o:%.cpp
	g++ $<  -c -o $@ $(CXXFLAGS)

kernel.o : kernel.cu kernel.h
	${NVCC} ${CUDA_COMPILE_FLAGS} -c kernel.cu -o $@

clean:
	rm *.o

main_list_omp:main_list_omp.cpp
	g++ $< -o $@ $(CXXFLAGS)