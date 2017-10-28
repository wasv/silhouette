/*
Increment a vector, one value per work item.

It is useless to do this on a GPU, not enough work / IO,
it's just a clEnqueueNDRangeKernel + get_global_id hello world.

- http://stackoverflow.com/questions/15194798/vector-step-addition-slower-on-cuda
- http://stackoverflow.com/questions/22005405/how-to-add-up-the-elements-of-an-array-in-gpu-any-function-similar-to-cublasdas
- http://stackoverflow.com/questions/15161575/reduction-for-sum-of-vector-when-size-is-not-power-of-2
*/
#include "common.h"
#include <stdio.h>


#define NCOLS  600
#define NROWS  600
//#define NCOLS 10
//#define NROWS 8
//#define FILENAME "binmask.bin"

#define KERNAL(src) #src

int main(int argc, char **argv) {
	if(argc <= 1) {
		printf("%s","Please specify one or more input files.\n");
		return EXIT_FAILURE;
	}
    const char *source = KERNAL(
								__kernel void kmain(__global uchar *input,
													__global uint *L,
													__global uint *R,
													const uint NC, const uint NR) {
									uint i = get_global_id(0);
									const uint center = NR >> 1;
									uint tL = center;
									uint tR = center;
									for(uint j = 0; j < center; j++) {
										tL = (uint)max( (uint)0, (uint)(tL - (uint)min((uint)1,(uint)input[tL*NC+i])) );
										tR = (uint)min( (uint)NR-1, (uint)(tR + (uint)min((uint)1,(uint)input[tR*NC+i])) );
									}
									L[i] = tL;
									R[i] = tR;
								}
								);
    cl_uchar input[NCOLS][NROWS];

	cl_uint L[NCOLS];
	cl_uint R[NCOLS];
    cl_mem input_buff, L_buff, R_buff;
    Common common;
    const size_t global_work_size = NCOLS;
	cl_uint NC = NCOLS;
	cl_uint NR = NROWS;



	/* Run kernel. */
    common_init(&common, source);
	
	FILE *outputfile;
	outputfile = fopen("output.mesh", "w");  // Open the file in binary mode

	for(int fnum = 0; fnum < argc; fnum++) {
		FILE *inputfile;
		inputfile = fopen(argv[fnum], "rb");  // Open the file in binary mode
		fread(input, sizeof(cl_uchar), NROWS*NCOLS, inputfile); // Read in the entire file
		fclose(inputfile); // Close the file
		
		input_buff = clCreateBuffer(common.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(input), input, NULL);
		L_buff = clCreateBuffer(common.context, CL_MEM_READ_ONLY, sizeof(L), NULL, NULL);
		R_buff = clCreateBuffer(common.context, CL_MEM_WRITE_ONLY, sizeof(R), NULL, NULL);
		clSetKernelArg(common.kernel, 0, sizeof(input_buff), &input_buff);
		clSetKernelArg(common.kernel, 1, sizeof(L_buff), &L_buff);
		clSetKernelArg(common.kernel, 2, sizeof(R_buff), &R_buff);
		clSetKernelArg(common.kernel, 3, sizeof(NC), &NC);
		clSetKernelArg(common.kernel, 4, sizeof(NR), &NR);
		clEnqueueNDRangeKernel(common.command_queue, common.kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
		clFlush(common.command_queue);
		clFinish(common.command_queue);
		clEnqueueReadBuffer(common.command_queue, L_buff, CL_TRUE, 0, sizeof(L), L, 0, NULL, NULL);
		clEnqueueReadBuffer(common.command_queue, R_buff, CL_TRUE, 0, sizeof(R), R, 0, NULL, NULL);

		float angle = (((float)(fnum-1)/(float)(argc-1))) * 360.0f;
		/* Assertions. */
		for(int i = 0; i < NCOLS; i++) {
			//printf("%d/%d: %d %d\n", fnum, i, L[i], R[i]);
			if(L[i] != 0 && R[i] != NROWS-1 && L[i] != R[i]) {
				fprintf(outputfile, "%f %d %d\n", angle, i, L[i]);
				fprintf(outputfile, "%f %d %d\n", angle, i, R[i]);
			}
		}

		/* Cleanup. */
		clReleaseMemObject(input_buff);
		clReleaseMemObject(L_buff);
		clReleaseMemObject(R_buff);
	}

	fclose(outputfile); // Close the file

    common_deinit(&common);
    return EXIT_SUCCESS;
}
