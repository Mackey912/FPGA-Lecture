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
#define width 32
#define height 32
#define in_channels 16
#define out_channels 32
#define ksize 2


void conv2d(data_t* x_outer, data_t* weight_outer, data_t* bias_outer, data_t* y_outer) {

#pragma HLS interface m_axi port=x_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=weight_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=bias_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=y_outer offset=slave bundle=gmem
#pragma HLS interface s_axilite port=x_outer
#pragma HLS interface s_axilite port=weight_outer
#pragma HLS interface s_axilite port=bias_outer
#pragma HLS interface s_axilite port=y_outer
#pragma HLS interface s_axilite port=return bundle=control

  data_t x[height*width*in_channels];
#pragma HLS RESOURCE variable=x  core=RAM_2P_BRAM
  data_t weight[in_channels*out_channels*ksize*ksize];
#pragma HLS RESOURCE variable=weight  core=RAM_2P_BRAM
  data_t bias[out_channels];
#pragma HLS RESOURCE variable=bias core=RAM_2P_BRAM
  data_t y[height*width*out_channels];
#pragma HLS RESOURCE variable=y core=RAM_2P_BRAM

  std::memcpy(x, x_outer, height*width*in_channels*sizeof(data_t));
  std::memcpy(weight, weight_outer, in_channels*out_channels*ksize*ksize*sizeof(data_t));
  std::memcpy(bias, bias_outer, out_channels*sizeof(data_t));


  for (int och = 0; och < out_channels; ++och) {
    for (int h = 0; h < height; ++h) {
      for (int w = 0; w < width; ++w) {
        float sum = 0.f;

        for (int ich = 0; ich < in_channels; ++ich) {
          for (int kh = 0; kh < ksize; ++kh) {
            for (int kw = 0; kw < ksize; ++kw) {
              int ph = h + kh - ksize/2;
              int pw = w + kw - ksize/2;

              // zero padding
              if (ph < 0 || ph >= height || pw < 0 || pw >= width) {
                continue;
              }

              int pix_idx = (ich * height + ph) * width + pw;
              int weight_idx = ((och * in_channels + ich) * ksize + kh) * ksize + kw;

              sum += x[pix_idx] * weight[weight_idx];
            }
          }
        }

        // add bias
        sum += bias[och];

        y[(och * height + h) * width + w] = sum;
      }
    }
  }
  std::memcpy(y_outer, y, height*width*out_channels*sizeof(data_t));
}

/*
//SA version
void conv2d(data_t* x_outer, data_t* weight_outer, data_t* bias_outer, data_t* y_outer) {

#pragma HLS interface m_axi port=x_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=weight_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=bias_outer offset=slave bundle=gmem
#pragma HLS interface m_axi port=y_outer offset=slave bundle=gmem
#pragma HLS interface s_axilite port=x_outer
#pragma HLS interface s_axilite port=weight_outer
#pragma HLS interface s_axilite port=bias_outer
#pragma HLS interface s_axilite port=y_outer
#pragma HLS interface s_axilite port=return bundle=control

  data_t x[height*width*in_channels];
#pragma HLS RESOURCE variable=x  core=RAM_2P_BRAM
  data_t weight[in_channels*out_channels*ksize*ksize];
#pragma HLS RESOURCE variable=weight  core=RAM_2P_BRAM
  data_t bias[out_channels];
#pragma HLS RESOURCE variable=bias core=RAM_2P_BRAM
  data_t y[height*width*out_channels];
#pragma HLS RESOURCE variable=y core=RAM_2P_BRAM

  std::memcpy(x, x_outer, height*width*in_channels*sizeof(data_t));
  std::memcpy(weight, weight_outer, in_channels*out_channels*ksize*ksize*sizeof(data_t));
  std::memcpy(bias, bias_outer, out_channels*sizeof(data_t));

  const int UNROLL_X = 8;
  const int UNROLL_OCH = 8;

  for (int block_och = 0; block_och < out_channels; block_och += UNROLL_OCH) {
    for (int h = 0; h < height; ++h) {
      for (int block_w = 0; block_w < width; block_w += UNROLL_X) {
        float sum[UNROLL_OCH][UNROLL_X];
#pragma HLS array_partition variable=sum complete dim=0

        for (int ich = 0; ich < in_channels; ++ich) {
          for (int kh = 0; kh < ksize; ++kh) {
            for (int kw = 0; kw < ksize; ++kw) {
#pragma HLS pipeline II=1
              for (int local_och = 0; local_och < UNROLL_OCH; local_och++) {
#pragma HLS unroll
                for (int local_w = 0; local_w < UNROLL_X; local_w++) {
#pragma HLS unroll
                  if (block_w + local_w < width && block_och + local_och < out_channels) {

                    int och = block_och + local_och;
                    int w = block_w + local_w;

                    int ph = h + kh - ksize/2;
                    int pw = w + kw - ksize/2;

                    float last = (ich == 0 && kh == 0 && kw == 0) ? 0 : sum[local_och][local_w];

                    // zero padding
                    if (ph < 0 || ph >= height || pw < 0 || pw >= width) {
                      sum[local_och][local_w] = last;
                      continue;
                    }

                    int pix_idx = (ich * height + ph) * width + pw;
                    int weight_idx = ((och * in_channels + ich) * ksize + kh) * ksize + kw;

                    sum[local_och][local_w] = last + x[pix_idx] * weight[weight_idx];
                  }
                }
              }
            }
          }
        }

        for (int local_och = 0; local_och < UNROLL_OCH; local_och++) {
#pragma HLS unroll
          for (int local_w = 0; local_w < UNROLL_X; local_w++) {
#pragma HLS unroll
            if (block_w + local_w < width && block_och + local_och < out_channels) {
              int och = block_och + local_och;
              int w = block_w + local_w;

              // add bias
              y[(och * height + h) * width + w] = sum[local_och][local_w] + bias[och];
            }
          }
        }
      }
    }
  }
  std::memcpy(y_outer, y, height*width*out_channels*sizeof(data_t));
}
*/
