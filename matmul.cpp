//2d normal version
void matmul(data_t A_outer[SIZE][SIZE], data_t B_outer[SIZE][SIZE], data_t output_outer[SIZE][SIZE]){

#pragma HLS interface m_axi port=A_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=B_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=output_outer offset=slave bundle=gmem

#pragma HLS interface s_axilite port=A_outer bundle=ctrl
#pragma HLS interface s_axilite port=B_outer bundle=ctrl
#pragma HLS interface s_axilite port=output_outer bundle=ctrl
#pragma HLS interface s_axilite port=return bundle=ctrl

//#pragma hls pipeline off

	data_t A[SIZE][SIZE];
	data_t B[SIZE][SIZE];
	data_t output[SIZE][SIZE];

	memcpy_2d(A_outer, SIZE, SIZE, A);
	memcpy_2d(B_outer, SIZE, SIZE, B);

	data_t sum = 0;
	matmul_label1:for(int i = 0; i < SIZE; i++){
	  matmul_label2:for(int j = 0; j < SIZE; j++){
		  matmul_label3:for(int k = 0; k < SIZE; k++){
			  sum += A[i][k] * B[k][j];
		  }
		  output[i][j] = sum;
		  sum=0;
	  }
	}

	memcpy_2d(output, SIZE, SIZE, output_outer);
}

/*
//2d SA version with check
void matmul(data_t A_outer[SIZE][SIZE], data_t B_outer[SIZE][SIZE], data_t output_outer[SIZE][SIZE])
{

#pragma HLS interface m_axi port=A_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=B_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=output_outer offset=slave bundle=gmem

#pragma HLS interface s_axilite port=A_outer bundle=ctrl
#pragma HLS interface s_axilite port=B_outer bundle=ctrl
#pragma HLS interface s_axilite port=output_outer bundle=ctrl
#pragma HLS interface s_axilite port=return bundle=ctrl

	data_t A[SIZE][SIZE];
#pragma HLS bind_storage variable=A type=RAM_2P impl=BRAM
#pragma HLS ARRAY_PARTITION variable=A dim=1 complete
	data_t B[SIZE][SIZE];
#pragma HLS bind_storage variable=B type=RAM_2P impl=BRAM
#pragma HLS ARRAY_PARTITION variable=B dim=2 complete
	data_t output[SIZE][SIZE];
#pragma HLS bind_storage variable=output type=RAM_2P impl=BRAM
#pragma HLS ARRAY_PARTITION variable=output dim=2 complete

  memcpy_2d(A_outer, SIZE, SIZE, A);
  memcpy_2d(B_outer, SIZE, SIZE, B);

  const int UNROLL_FACTOR=4;

	for(int block_i = 0; block_i < SIZE; block_i+=UNROLL_FACTOR){
		for(int block_j = 0; block_j < SIZE; block_j+=UNROLL_FACTOR){
		data_t sum[UNROLL_FACTOR][UNROLL_FACTOR];
#pragma HLS array_partition variable=sum complete dim=0
			for(int k = 0; k < SIZE; k++){
#pragma HLS pipeline II=1
				for(int local_i=0; local_i<UNROLL_FACTOR; local_i++){
#pragma HLS unroll
					for(int local_j=0; local_j<UNROLL_FACTOR; local_j++){
#pragma HLS unroll
						if(block_i+local_i < SIZE && block_j+local_j < SIZE){
							int i=block_i+local_i;
							int j=block_j+local_j;
							data_t last = (k==0) ? (data_t)0 : sum[local_i][local_j];
							sum[local_i][local_j]=last+A[i][k] * B[k][j];
						}
					}
				}
			}
			for(int local_i=0; local_i<UNROLL_FACTOR; local_i++){
#pragma HLS unroll
				for(int local_j=0; local_j<UNROLL_FACTOR; local_j++){
#pragma HLS unroll
					if(block_i+local_i < SIZE && block_j+local_j < SIZE){
						int i=block_i+local_i;
						int j=block_j+local_j;
						output[i][j] = sum[local_i][local_j];
					}
				}
			}
		}
	}
  	memcpy_2d(output, SIZE, SIZE, output_outer);
}
*/

//testbench
int main(){
  
  //SIZE=2 version! Change SIZE=2 from SIZE=100 before starting this code!
  //data_t a[SIZE*SIZE] = {0, 1, 2, 3};
  //data_t b[SIZE*SIZE] = {3, 2, 1, 0};
  //data_t c[SIZE*SIZE];
  data_t a[SIZE][SIZE];
  a[0][0]=1; a[0][1]=2; a[1][0]=3; a[1][1]=4;
  data_t b[SIZE][SIZE];
  b[0][0]=4; b[0][1]=3; b[1][0]=2; b[1][1]=1;
  data_t c[SIZE][SIZE];

  data_t a_check[SIZE][SIZE];
  data_t b_check[SIZE][SIZE];

  matmul(a, b, c);
  //you should comment out when synthesizing ip
  //std::cout << c[0][0] << " " << c[0][1] << " \n" << c[1][0] << " " << c[1][1] << " " << std::endl;
  return 0;
  
}
