#include<stdio.h>
#include<ctime>
#include<cmath>
#include <iostream>
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>


#define THREADS 1024 
__global__ void zeroPad(float *a, int arraySize, int SIZE)
{
	int blockId = blockIdx.x 
			 + blockIdx.y * gridDim.x 
			 + gridDim.x * gridDim.y * blockIdx.z; 
	int threadId = blockId * blockDim.x + threadIdx.x;

	
	if(threadId>=SIZE && threadId<arraySize)
	{
		a[threadId] = 0;
	}
}

__global__ void bitonic_sort_step(float *dev_values, int stage, int step, int newSize)

{
  unsigned int arrayIndex, xorVal; 
  int blockId = blockIdx.x 
			 + blockIdx.y * gridDim.x 
			 + gridDim.x * gridDim.y * blockIdx.z; 

  arrayIndex = blockId * blockDim.x + threadIdx.x;

  xorVal = arrayIndex^stage;

 
  //amongst the two threads that are being compared pick the lower thread and as each thread holds an array element 
  //pick the ones with threadID less than the size of the array being sorted 

  if ((xorVal)>arrayIndex && arrayIndex<newSize) {
	// sort the data in ascending order
    if ((arrayIndex&step)==0) {

	//swap if not in the right order
      if (dev_values[arrayIndex]>dev_values[xorVal]) {

        float temp = dev_values[arrayIndex];

        dev_values[arrayIndex] = dev_values[xorVal];

        dev_values[xorVal] = temp;

      }
    }
	// sort the data in descending order
    if ((arrayIndex&step)!=0) {

      //swap if not in the right order
      if (dev_values[arrayIndex]<dev_values[xorVal]) {

        float temp = dev_values[arrayIndex];

        dev_values[arrayIndex] = dev_values[xorVal];

        dev_values[xorVal] = temp;

      }
    }
  }

}
 


float bitonic_sort(float *values, int newSize, int SIZE)

{
  float *dev_values,tgpu = -1;
  clock_t start, end;
  cudaError_t status;

 
  status = cudaMalloc((void**) &dev_values, newSize * sizeof(float));
  
  status = cudaMemcpy(dev_values, values, SIZE * sizeof(float), cudaMemcpyHostToDevice);
    
  dim3 dimBlock(THREADS,1);  
  //decide on the grid size based on the array size
  int grid1 = ceil((float)newSize/(float)THREADS);

	int grid2 = 1;
	int grid3=1;
	if(grid1>65535)
	{
		grid2 =   ceil((float)newSize/(float)THREADS);
		grid1 = 65535;
	}
	if(grid2>65535)
	{
		grid3 = ceil((float)newSize/(float)THREADS);
		grid2 = 65535;
	}
	dim3 dimGrid(grid1,grid2,grid3); 
     



 
  int stage, step;

  start = clock();
  //pad extra zeros when the actual array size is not a power of 2
  if(newSize!=SIZE)
  {
  zeroPad<<<dimGrid, dimBlock>>>(dev_values,newSize,SIZE);
		status = cudaDeviceSynchronize();
  }
  //This loop computes each step of the sorting network
  for (step = 2; step <= newSize; step <<= 1) {

  //This computes each stage in a step
    for (stage=step>>1; stage>0; stage=stage>>1) {

      bitonic_sort_step<<<dimGrid, dimBlock>>>(dev_values, stage, step, newSize);
	  status = cudaDeviceSynchronize();
    }
  }
  end = clock();
  // Check for errors and exit on any failures
		status = cudaGetLastError();
		if (status != cudaSuccess) 
		{
		std::cout << "\n Kernel failed: " << cudaGetErrorString(status) << 
		std::endl;
		
		}
 //if everything is successfully cpy back the results
		else{
			// if the actual array size is a power of 2 copy the entire array
		if(newSize==SIZE)
		cudaMemcpy(values, dev_values, newSize * sizeof(float), cudaMemcpyDeviceToHost);
		//else discard the zeros that are padded and copy the actual sorted entries of the array
		else
		cudaMemcpy(values, dev_values+(newSize-SIZE), SIZE * sizeof(float), cudaMemcpyDeviceToHost);
		tgpu = ((float)((end - start)*1000))/((float)(CLOCKS_PER_SEC));
		}
  cudaFree(dev_values);
  // Returns the time for which the actual computation was being executed on the device 
  return tgpu;
}
 

