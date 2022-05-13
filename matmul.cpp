#include <stdint.h>
#include <cstring>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "hls_half.h"
#include "ap_int.h"
#include "hls_stream.h"
#include "ap_axi_sdata.h"


typedef half data_t;
#define SIZE 100

/*
//normal version
void matmul(data_t A_outer[SIZE*SIZE], data_t B_outer[SIZE*SIZE], data_t bias_outer[SIZE], data_t output_outer[SIZE*SIZE]){

#pragma HLS interface m_axi port=A_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=B_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=output_outer bundle=gmem
#pragma HLS interface s_axilite port=A_outer
#pragma HLS interface s_axilite port=B_outer
#pragma HLS interface s_axilite port=bias_outer
#pragma HLS interface s_axilite port=output_outer
#pragma HLS interface s_axilite port=return bundle=control

	#pragma hls pipeline off

	data_t A[SIZE*SIZE];
	data_t B[SIZE*SIZE];
	data_t bias[SIZE];
	data_t output[SIZE*SIZE];

  data_t sum = 0;
  matmul_label1:for(int i = 0; i < SIZE; i++){
      matmul_label2:for(int j = 0; j < SIZE; j++){
          matmul_label3:for(int k = 0; k < SIZE; k++){
              sum += A[i*SIZE+k] * B[k*SIZE+j];
          }
          output[i*SIZE+j] = sum + bias[j];
          sum=0;
      }
  }
  std::memcpy(output_outer, output, A_c*SIZE*SIZE*sizeof(data_t));
}
*/

//SA version
void matmul(data_t A_outer[SIZE*SIZE], data_t B_outer[SIZE*SIZE], data_t bias_outer[SIZE], data_t output_outer[SIZE*SIZE])
{

#pragma HLS interface m_axi port=A_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=B_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=output_outer bundle=gmem
//#pragma HLS interface m_axi port=A_outer
//#pragma HLS interface m_axi port=B_outer
//#pragma HLS interface m_axi port=output_outer
#pragma HLS interface s_axilite port=A_outer
#pragma HLS interface s_axilite port=B_outer
#pragma HLS interface s_axilite port=bias_outer
#pragma HLS interface s_axilite port=output_outer
#pragma HLS interface s_axilite port=return bundle=control

//#pragma hls pipeline off
	data_t A[SIZE*SIZE];
#pragma HLS RESOURCE variable=A  core=RAM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable=A block factor=100 dim=1
	data_t B[SIZE*SIZE];
#pragma HLS RESOURCE variable=B  core=RAM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable=B cyclic factor=100 dim=1
	data_t bias[SIZE];
#pragma HLS RESOURCE variable=bias  core=RAM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable=bias dim=1 complete
	data_t output[SIZE*SIZE];
#pragma HLS RESOURCE variable=output core=RAM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable=output cyclic factor=100 dim=1

	std::memcpy(A, A_outer, SIZE*SIZE*sizeof(data_t));
	std::memcpy(B, B_outer, SIZE*SIZE*sizeof(data_t));
	std::memcpy(bias, bias_outer, SIZE*sizeof(data_t));

	const int UNROLL_OCH=8;
	const int UNROLL_X=8;

	for(int block_i = 0; block_i < SIZE; block_i+=UNROLL_OCH){
		for(int block_j = 0; block_j < SIZE; block_j+=UNROLL_X){
		data_t sum[UNROLL_OCH][UNROLL_X];
#pragma HLS array_partition variable=sum complete dim=0
			for(int k = 0; k < SIZE; k++){
#pragma HLS pipeline
				for(int local_i=0; local_i<UNROLL_OCH; local_i++){
#pragma HLS unroll
					for(int local_j=0; local_j<UNROLL_X; local_j++){
#pragma HLS unroll
						if(block_i+local_i < A_h && block_j+local_j < B_w){
							int i=block_i+local_i;
							int j=block_j+local_j;
							data_t last = (k==0) ? (data_t)0 : sum[local_i][local_j];
							sum[local_i][local_j]=last+A[i*SIZE+k] * B[k*SIZE+j];
						}
					}
				}
			}
			for(int local_i=0; local_i<UNROLL_OCH; local_i++){
#pragma HLS unroll
				for(int local_j=0; local_j<UNROLL_X; local_j++){
#pragma HLS unroll
					if(block_i+local_i < SIZE && block_j+local_j < SIZE){
						int i=block_i+local_i;
						int j=block_j+local_j;
						output[i*SIZE+j] = sum[local_i][local_j] + bias[j];
					}
				}
			}
		}
	}
  	std::memcpy(output_outer, output, SIZE*SIZE*sizeof(data_t));
}


void 2d_memcpy(data_t input[SIZE][SIZE], int height, int width, data_t output[SIZE][SIZE]){
  for(int i=0: i<height: i++){
    for(int j=0: j<width: j++){
      output[i][j] = input[i][j];
    }
  }
}

//2d SA version
void matmul(data_t A_outer[SIZE][SIZE], data_t B_outer[SIZE][SIZE], data_t bias_outer[SIZE], data_t output_outer[SIZE][SIZE])
{

#pragma HLS interface m_axi port=A_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=B_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=output_outer bundle=gmem
//#pragma HLS interface m_axi port=A_outer
//#pragma HLS interface m_axi port=B_outer
//#pragma HLS interface m_axi port=output_outer
#pragma HLS interface s_axilite port=A_outer
#pragma HLS interface s_axilite port=B_outer
#pragma HLS interface s_axilite port=bias_outer
#pragma HLS interface s_axilite port=output_outer
#pragma HLS interface s_axilite port=return bundle=control

//#pragma hls pipeline off
	data_t A[SIZE][SIZE];
#pragma HLS RESOURCE variable=A core=RAM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable=A block dim=1 complete
	data_t B[SIZE][SIZE];
#pragma HLS RESOURCE variable=B core=RAM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable=B dim=2 complete
	data_t bias[SIZE];
#pragma HLS RESOURCE variable=bias core=RAM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable=bias dim=1 complete
	data_t output[SIZE][SIZE];
#pragma HLS RESOURCE variable=output core=RAM_2P_BRAM
#pragma HLS ARRAY_PARTITION variable=output dim=2 complete

  2d_memcpy(A_outer, SIZE, SIZE, A);
  2d_memcpy(B_outer, SIZE, SIZE, B);
	std::memcpy(bias, bias_outer, SIZE*sizeof(data_t));

  const int UNROLL_OCH=8;
  const int UNROLL_X=8;

	for(int block_i = 0; block_i < SIZE; block_i+=UNROLL_OCH){
		for(int block_j = 0; block_j < SIZE; block_j+=UNROLL_X){
		data_t sum[UNROLL_OCH][UNROLL_X];
#pragma HLS array_partition variable=sum complete dim=0
			for(int k = 0; k < SIZE; k++){
#pragma HLS pipeline
				for(int local_i=0; local_i<UNROLL_OCH; local_i++){
#pragma HLS unroll
					for(int local_j=0; local_j<UNROLL_X; local_j++){
#pragma HLS unroll
						if(block_i+local_i < A_h && block_j+local_j < B_w){
							int i=block_i+local_i;
							int j=block_j+local_j;
							data_t last = (k==0) ? (data_t)0 : sum[local_i][local_j];
							sum[local_i][local_j]=last+A[i][k] * B[k][j];
						}
					}
				}
			}
			for(int local_i=0; local_i<UNROLL_OCH; local_i++){
#pragma HLS unroll
				for(int local_j=0; local_j<UNROLL_X; local_j++){
#pragma HLS unroll
					if(block_i+local_i < SIZE && block_j+local_j < SIZE){
						int i=block_i+local_i;
						int j=block_j+local_j;
						output[i][j] = sum[local_i][local_j] + bias[j];
					}
				}
			}
		}
	}
  	2d_memcpy(output, SIZE, SIZE, output_outer);
}

//testbench
int main(){
  
  //SIZE=2 version! Change SIZE=2 from SIZE=100 before starting this code!
  data_t a[SIZE*SIZE] = {0, 1, 2, 3};
  data_t b[SIZE*SIZE] = {3, 2, 1, 0};
  data_t c[SIZE*SIZE];
  matmul(a, b, c);
	return 0;
  
}
