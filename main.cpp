#include <stdio.h>
#include<stdlib.h>
#include<ctime>
#include<cmath>
#define SIZE 4096 //actual size of the input sequence to be sorted
#define ITERATIONS 100
void checkForError(float *arr1,float *arr2);
float bitonic_sort(float *values, int newSize, int size);
void bubblesort(float*);

void bubblesort(float* data)
{
	float temp;
	bool swap=false;
	for(int i=0; i<SIZE; i++)
	{
		for(int j=0; j<SIZE-i-1; j++)
		{
			if(data[j]>data[j+1])
			{
				swap = true;
				temp = data[j];
				data[j] = data[j+1];
				data[j+1] = temp; 
			}
		
		}
		//break out of the loop if there are no swappings
		if(!swap)
		{
			break;
		}
	}
	
}

void checkForError(float *arr1, float *arr2)
{
	bool brokeOutOfLoop = false;  
	for(int i=0; i<SIZE; i++)
	{
		if(arr1[i]!=arr2[i])
		{
			brokeOutOfLoop = true;
			break;
		}
	}
	  printf("\n\t***********CORRECTNESS OF DATA*************\n");
		  if(!brokeOutOfLoop)
		  {
			  printf("\nData sorted correctly\n");
		  }
		  else
		  {
			  printf("\nError in the sorted data\n");
		  }
}
// produce an ascending sequence that can be used to form a bitonic sequence for the next step
        void merge_up(float *arr1, int n) {
          int stage=n/2,i,j,k;
			  float temp;
			  //repeat the process for all stages in a particular step
          while (stage > 0) {
            for (i=0; i < n; i+=stage*2) {
              for (j=i,k=0;k < stage;j++,k++) {
            if (arr1[j] > arr1[j+stage]) {
              // swap
              temp = arr1[j];
              arr1[j]=arr1[j+stage];
              arr1[j+stage]=temp;
            }
              }
            }
            stage /= 2;
          }
        }
// produce a descending sequence that can be used to form a bitonic sequence for the next step
        void merge_down(float *arr1, int n) {
          int step=n/2,i,j,k;
		  float temp;
		   //repeat the process for all stages in a particular step
          while (step > 0) {
            for (i=0; i < n; i+=step*2) {
              for (j=i,k=0;k < step;j++,k++) {
            if (arr1[j] < arr1[j+step]) {
              // swap
              temp = arr1[j];
              arr1[j]=arr1[j+step];
              arr1[j+step]=temp;
            }
              }
            }
            step /= 2;
          }
        }

		void zeropad(int size, float *data)
	{
		for(int i=SIZE; i<size; i++)
		data[i] = 0;
	}
        void printArray(float *arr1, int n) {
          int i;

          printf("[%f",arr1[0]);
          for (i=1; i < n;i++) {
            printf(",%f",arr1[i]);
          }
          printf("]\n");
        }

  int main(int argc, char **argv) {
	clock_t start, end,clocks_elapsed=0;
	float tcpu, tgpu, tgpu_comp = 0,tcomp;
	float base = 2;
    int n, i, newSize;
	float *arr;
	 float *arr1;
	float *arr2;
	float *arr3;
	float *arr4;
	double temp;
	
	temp = log((float)SIZE)/log(base);
	//if array size is not a power of 2 find the nearest number that is greater than this number
	//and a power of 2 and use it as the bitonic implementation's array size
	if(temp - (int)temp == 0)
	{
		newSize= SIZE;
	}
	else
	{
		newSize = pow(base,(int)temp+1);
	}
	n = SIZE;
	arr = new float[SIZE]; //input and output for bubble sort
    arr1 =  new float[SIZE]; //output of Bitonic CPU 
	arr2 = new float[SIZE]; //input and output for Bitonic GPU 
	arr3 = new float[newSize]; // input for Bitonic CPU
	arr4 = new float[newSize]; //reference for reninitialization of arrays
	
          for (i=0; i < n; i++) {
           arr4[i] = (1+i)*rand(); //change this to (i+1) for best case scenario
		   arr3[i] = arr4[i];
		   arr2[i] = arr4[i];
		   arr[i] = arr4[i];
          }
        for(int i=0; i<ITERATIONS;i++)
		{
			//Reinitialize data after each execution to ensure randomness of inputs and avoid 
		for(int j=0; j<SIZE; j++)
		{
			arr[j] = arr4[j]; 
		}
		start = clock();
		bubblesort(arr);
		end = clock();
		clocks_elapsed+= end-start;
		
		}
		  tcomp = ((float)((clocks_elapsed)*1000)/(float)(CLOCKS_PER_SEC*ITERATIONS));
		  printf("\n\t*************BUBBLE SORT******************\n");
		  printf("\nTime taken to sort a sequence of length %d  is %f ms \n",SIZE,tcomp);
			
		   start = clock();
		  // obtain the average execution time for Bitonic Merge Sort on CPU
		  for(int x = 0; x<ITERATIONS; x++)
		  {
			  for(int j=0; j<SIZE; j++)
			{
				arr3[j] = arr4[j]; 
			}
			  if(newSize!=SIZE)
			  {
				zeropad(newSize,arr3);
				
			  }
          for (int step=2; step <= newSize; step*=2) {
            for (i=0; i < newSize; i += step*2) {
              merge_up((arr3+i),step);
			  if(step<newSize)
				merge_down((arr3+i+step),step);
              
            }
          }
		   int index = newSize-SIZE;
		  for(int i=0; i<SIZE; i++)
		  {
			 arr1[i] = arr3[i+index];
			  

		  }
		  }
		  end = clock();
		
		  tcpu = ((float)((end - start)*1000))/((float)(ITERATIONS*CLOCKS_PER_SEC));
		  printf("\n\t*************BITONIC ON CPU******************\n");
		  printf("\nTime taken to sort a sequence of length %d  is %f ms \n",SIZE,tcpu);
		   
           checkForError(arr, arr1);
		   
		  //warm up pass
		  tgpu_comp = bitonic_sort(arr2, newSize,SIZE);
		   
          //obtain the average execution time for Bitonic Merge Sort on GPU when the GPU implementation executed without any failures
		   if(tgpu_comp!=-1)
		   {
			   tgpu_comp = 0;
			   start = clock();
		   for(int x = 0; x<ITERATIONS; x++)
		  {
			 tgpu_comp+= bitonic_sort(arr2, newSize,SIZE); 
          
			  
		   }
          
		 
		  end = clock();
		 
		  tgpu_comp/=ITERATIONS;
		  tgpu = ((float)((end - start)*1000))/((float)(ITERATIONS*CLOCKS_PER_SEC));
		  printf("\n\t*************BITONIC ON GPU******************\n");
		  printf("\nTime taken to sort a sequence of length %d  is %f ms \nout of which computation time is %f ms\n",SIZE,tgpu,tgpu_comp);
         
		 checkForError(arr, arr2);
		   }
		  else
		  {
			  printf("Kernel computation failed\n");
		  }

		   return 0;
        }