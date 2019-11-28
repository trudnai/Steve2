//
//  Shaders.metal
//  A2Mac
//
//  Created by Tamas Rudnai on 9/27/19.
//  Copyright © 2019 GameAlloy. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;


vertex float4 basic_vertex(                           // 1
  const device packed_float3* vertex_array [[ buffer(0) ]], // 2
  unsigned int vid [[ vertex_id ]]) {                 // 3
  return float4(vertex_array[vid], 1.0);              // 4
}

fragment half4 basic_fragment() { // 1
  return half4(0.7);              // 2
}


kernel void add_arrays(device const float* inA,
                       device const float* inB,
                       device float* result,
                       uint index [[thread_position_in_grid]])
{
    // the for-loop is replaced with a collection of threads, each of which
    // calls this function.
    result[index] = inA[index] + inB[index] + 1;
}


