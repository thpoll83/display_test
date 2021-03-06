#pragma once

#define TRANSPOSE8_REVERSE(in7, in6, in5, in4, in3, in2, in1, in0 ) \
  uint8_t{(in0 & 1)*128| (in1 & 1)*64 | (in2 & 1)*32 | (in3 &   1)*16| (in4 &   1)*8 | (in5 &   1)*4 | (in6 &   1)*2 | (in7 &   1)}, \
  uint8_t{(in0 & 2)*64 | (in1 & 2)*32 | (in2 & 2)*16 | (in3 &   2)*8 | (in4 &   2)*4 | (in5 &   2)*2 | (in6 &   2)   | (in7 &   2)/2}, \
  uint8_t{(in0 & 4)*32 | (in1 & 4)*16 | (in2 &  4)*8 | (in3 &   4)*4 | (in4 &   4)*2 | (in5 &   4)   | (in6 &   4)/2  | (in7 &   4)/4}, \
  uint8_t{(in0 & 8)*16 | (in1 &  8)*8 | (in2 &  8)*4 | (in3 &   8)*2 | (in4 &   8)   | (in5 &   8)/2 | (in6 &   8)/4 | (in7 &   8)/8}, \
  uint8_t{(in0 & 16)*8 | (in1 & 16)*4 | (in2 & 16)*2 | (in3 & 16)   | (in4 & 16)/2 | (in5 & 16)/4 | (in6 & 16)/8 | (in7 & 16)/16}, \
  uint8_t{(in0 & 32)*4 | (in1 & 32)*2 | (in2 & 32)   | (in3 & 32)/2 | (in4 & 32)/4 | (in5 & 32)/8 | (in6 & 32)/16 | (in7 & 32)/32}, \
  uint8_t{(in0 & 64)*2 | (in1 & 64)   | (in2 & 64)/2 | (in3 & 64)/4 | (in4 & 64)/8 | (in5 & 64)/16 | (in6 & 64)/32 | (in7 & 64)/64}, \
  uint8_t{(in0 & 128) | (in1 & 128)/2 | (in2 & 128)/4 | (in3 & 128)/8 | (in4 & 128)/16 | (in5 & 128)/32 | (in6 & 128)/64 | (in7 & 128)/128}


#define TRANSPOSE8(in7, in6, in5, in4, in3, in2, in1, in0 ) \
  uint8_t{(in0 & 128) | (in1 & 128)/2 | (in2 & 128)/4 | (in3 & 128)/8 | (in4 & 128)/16 | (in5 & 128)/32 | (in6 & 128)/64 | (in7 & 128)/128}, \
  uint8_t{(in0 & 64)*2 | (in1 & 64)   | (in2 & 64)/2 | (in3 & 64)/4 | (in4 & 64)/8 | (in5 & 64)/16 | (in6 & 64)/32 | (in7 & 64)/64}, \
  uint8_t{(in0 & 32)*4 | (in1 & 32)*2 | (in2 & 32)   | (in3 & 32)/2 | (in4 & 32)/4 | (in5 & 32)/8 | (in6 & 32)/16 | (in7 & 32)/32}, \
  uint8_t{(in0 & 16)*8 | (in1 & 16)*4 | (in2 & 16)*2 | (in3 & 16)   | (in4 & 16)/2 | (in5 & 16)/4 | (in6 & 16)/8 | (in7 & 16)/16}, \
  uint8_t{(in0 & 8)*16 | (in1 &  8)*8 | (in2 &  8)*4 | (in3 &   8)*2 | (in4 &   8)   | (in5 &   8)/2 | (in6 &   8)/4 | (in7 &   8)/8}, \
  uint8_t{(in0 & 4)*32 | (in1 & 4)*16 | (in2 &  4)*8 | (in3 &   4)*4 | (in4 &   4)*2 | (in5 &   4)   | (in6 &   4)/2  | (in7 &   4)/4}, \
  uint8_t{(in0 & 2)*64 | (in1 & 2)*32 | (in2 & 2)*16 | (in3 &   2)*8 | (in4 &   2)*4 | (in5 &   2)*2 | (in6 &   2)   | (in7 &   2)/2}, \
  uint8_t{(in0 & 1)*128| (in1 & 1)*64 | (in2 & 1)*32 | (in3 &   1)*16| (in4 &   1)*8 | (in5 &   1)*4 | (in6 &   1)*2 | (in7 &   1)}
