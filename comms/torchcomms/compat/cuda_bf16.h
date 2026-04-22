// Copyright (c) Meta Platforms, Inc. and affiliates.
// CUDA-to-HIP compatibility wrapper for bfloat16.
#pragma once

#if defined(__HIPCC__)
#include <hip/hip_bf16.h>
#define __nv_bfloat16 __hip_bfloat16
#else
// Host-only stub — see cuda.h compat for details
#ifndef __nv_bfloat16
struct __nv_bfloat16 { unsigned short __x; };
#endif
#endif
