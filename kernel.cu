#include"op.h"

struct op *computeResult(float* maxG,int* nodeI,int* nodeJ,int height,int width);
void printMatrix(float* matrix,int height,int width);
void printMatrixNode(int* matrix,int height,int width);
__global__ void compkernel(int nodeNum,float *graph,float *Dv,int *locked,int* group,float* d_maxG,int *d_nodeI,int * d_nodeJ)
{

	int X= blockIdx.x * blockDim.x + threadIdx.x;
	int Y= blockIdx.y * blockDim.y + threadIdx.y;
	float maxG= -1e10;
	int nodeI = -1,nodeJ = -1;
	for (int i = X; i < nodeNum; i+= blockDim.x)
	{
		if (group[i] == 0 && locked[i] == 0)
		{
			for (int j = Y; j < nodeNum; j+=blockDim.y)
			{
				if (group[j] == 1 && locked[j] == 0)
				{

					float localG = Dv[i] + Dv[j] - 2 * graph[i*nodeNum+j];
					if (localG >= maxG)
					{

						maxG = localG;
						nodeI = i;
						nodeJ = j;
					}
				}
			}
		}
	}
	int resultIndex = blockDim.y*threadIdx.x+threadIdx.y;
	d_maxG[resultIndex] = maxG;
	d_nodeI[resultIndex] = nodeI;
	d_nodeJ[resultIndex] = nodeJ;
	
}

struct op* hostFE (int nodeNum,float *graph,float *Dv,int *locked,int* group)
{
	
	float * d_graph,*d_Dv,*d_maxG;
	int *d_group,*d_locked,*d_nodeI,*d_nodeJ;
	int graphSize = nodeNum*nodeNum*sizeof(float);
	int DvSize = nodeNum*sizeof(float);
	int lockedSize = nodeNum*sizeof(float);
	int groupSize = nodeNum*sizeof(float);

	int block_size_x = 32;
	int block_size_y = 32;
	int resultSize = block_size_x*block_size_y*sizeof(float);
	float l_maxG[block_size_x*block_size_y];
	int l_nodeI[block_size_x*block_size_y];
	int l_nodeJ[block_size_x*block_size_y];
	cudaMalloc((void**)&d_graph,graphSize);
	cudaMalloc((void**)&d_Dv,DvSize);
	cudaMalloc((void**)&d_locked,lockedSize);
	cudaMalloc((void**)&d_group,groupSize);

	cudaMalloc((void**)&d_maxG,resultSize);
	cudaMalloc((void**)&d_nodeI,resultSize);
	cudaMalloc((void**)&d_nodeJ,resultSize);

	cudaMemcpy(d_graph,graph,graphSize,cudaMemcpyHostToDevice);
	cudaMemcpy(d_Dv,Dv,DvSize,cudaMemcpyHostToDevice);
	cudaMemcpy(d_locked,locked,lockedSize,cudaMemcpyHostToDevice);
	cudaMemcpy(d_group,group,groupSize,cudaMemcpyHostToDevice);

	dim3 blockSize(block_size_x,block_size_y);

	//int group_size_x =(nodeNum % block_size_x)? nodeNum / block_size_x +1:nodeNum/block_size_x;
	//int group_size_y  = group_size_x;
	int group_size_x =1;
	int group_size_y  = 1;
	dim3 numBlock(group_size_x,group_size_y);
	compkernel<<<numBlock,blockSize>>>(nodeNum,d_graph,d_Dv,d_locked,d_group,d_maxG,d_nodeI,d_nodeJ);
	
	cudaMemcpy(l_maxG,d_maxG,resultSize,cudaMemcpyDeviceToHost);
	cudaMemcpy(l_nodeI,d_nodeI,resultSize,cudaMemcpyDeviceToHost);
	cudaMemcpy(l_nodeJ,d_nodeJ,resultSize,cudaMemcpyDeviceToHost);

	#ifdef debug
	cout << "gpu result l_maxG----" << endl;
	printMatrix(l_maxG,block_size_x,block_size_y);
	cout << "gpu result nodeI----" << endl;
	printMatrixNode(l_nodeI,block_size_x,block_size_y);
	cout << "gpu result nodeJ----" << endl;
	printMatrixNode(l_nodeJ,block_size_x,block_size_y);
	#endif

	cudaFree(d_graph);
	cudaFree(d_Dv);
	cudaFree(d_locked);
	cudaFree(d_group);
	cudaFree(d_maxG);
	cudaFree(d_nodeI);
	cudaFree(d_nodeJ);
	return computeResult(l_maxG,l_nodeI,l_nodeJ,block_size_x,block_size_y);
}
void printMatrix(float* matrix,int height,int width){
	#ifdef debug
	for(int i=0;i<height;i++){
		for(int j=0;j<width;j++){
			cout << matrix[i*width+j] << " ";
		}
		cout << endl;
	}
	cout << endl;
	#endif
}
void printMatrixNode(int* matrix,int height,int width){
	#ifdef debug
	for(int i=0;i<height;i++){
		for(int j=0;j<width;j++){
			cout << matrix[i*width+j] << " ";
		}
		cout << endl;
	}
	cout << endl;
	#endif
}
struct op* computeResult(float* maxG,int * nodeI,int* nodeJ,int height,int width){
	float LmaxG = *maxG;
	int LnodeI = *nodeI, LnodeJ = *nodeJ;
	for (int i = 0; i < height*width; i++)
	{
		if (maxG[i]>= LmaxG)
		{
			LmaxG = maxG[i];
			LnodeI = nodeI[i];
			LnodeJ = nodeJ[i];
		}
	}
	struct op* result = (struct op*)malloc(sizeof(struct op));
	result->g= LmaxG;
	result->nodeI = LnodeI;
	result->nodeJ = LnodeJ;
	return result;
}

