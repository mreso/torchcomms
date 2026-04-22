// Copyright (c) Meta Platforms, Inc. and affiliates.
// CUDA-to-HIP compatibility — type and constant mappings for the driver API.
// Included via -I priority when building with ROCm.
#pragma once

#include <hip/hip_runtime.h>

// Driver API types
#define CUresult hipError_t
#define CUdevice hipDevice_t
#define CUdeviceptr hipDeviceptr_t
#define CUcontext hipCtx_t
#define CUmemAllocationProp hipMemAllocationProp
#define CUmemGenericAllocationHandle hipMemGenericAllocationHandle_t
#define CUmemAllocationHandleType hipMemAllocationHandleType
#define CUmemAccessDesc hipMemAccessDesc

// Driver API error codes
#define CUDA_SUCCESS hipSuccess
#define CUDA_ERROR_NOT_READY hipErrorNotReady
#define CUDA_ERROR_NOT_INITIALIZED hipErrorNotInitialized
#define CUDA_ERROR_INVALID_VALUE hipErrorInvalidValue

// Driver API functions — 1:1 HIP equivalents
#define cuDeviceGet hipDeviceGet
#define cuDeviceGetAttribute hipDeviceGetAttribute
#define cuPointerGetAttribute hipPointerGetAttribute
#define cuCtxGetCurrent hipCtxGetCurrent
#define cuCtxSetCurrent hipCtxSetCurrent
#define cuCtxGetDevice hipCtxGetDevice
#define cuMemCreate hipMemCreate
#define cuMemMap hipMemMap
#define cuMemUnmap hipMemUnmap
#define cuMemRelease hipMemRelease
#define cuMemSetAccess hipMemSetAccess
#define cuMemGetAccess hipMemGetAccess
#define cuMemAddressReserve hipMemAddressReserve
#define cuMemAddressFree hipMemAddressFree
#define cuMemGetAllocationGranularity hipMemGetAllocationGranularity
#define cuMemGetAllocationPropertiesFromHandle hipMemGetAllocationPropertiesFromHandle
#define cuMemRetainAllocationHandle hipMemRetainAllocationHandle
#define cuMemExportToShareableHandle hipMemExportToShareableHandle
#define cuMemImportFromShareableHandle hipMemImportFromShareableHandle
#define cuMemGetAddressRange hipMemGetAddressRange
#define cuMemHostGetDevicePointer hipHostGetDevicePointer
#define cuGetErrorString hipDrvGetErrorString

// Driver API allocation constants
#define CU_MEM_ALLOCATION_TYPE_PINNED hipMemAllocationTypePinned
#define CU_MEM_LOCATION_TYPE_DEVICE hipMemLocationTypeDevice
#define CU_MEM_ALLOC_GRANULARITY_MINIMUM hipMemAllocationGranularityMinimum
#define CU_MEM_ACCESS_FLAGS_PROT_READWRITE hipMemAccessFlagsProtReadWrite

// Memory handle type constants
#define CU_MEM_HANDLE_TYPE_NONE hipMemHandleTypeNone
#define CU_MEM_HANDLE_TYPE_POSIX_FILE_DESCRIPTOR hipMemHandleTypePosixFileDescriptor
#define CU_MEM_HANDLE_TYPE_WIN32 hipMemHandleTypeWin32
#define CU_MEM_HANDLE_TYPE_FABRIC 0x8

// IPC
#define CU_IPC_HANDLE_SIZE 64

// Device attributes
#define CU_DEVICE_ATTRIBUTE_GPU_DIRECT_RDMA_WITH_CUDA_VMM_SUPPORTED 0
#define CU_DEVICE_ATTRIBUTE_DMA_BUF_SUPPORTED 0

// Memory range handle types (used by getCuMemDmaBufFd)
#define CU_MEM_RANGE_HANDLE_TYPE_DMA_BUF_FD 1
#define CU_MEM_RANGE_FLAG_DMA_BUF_MAPPING_TYPE_PCIE 0

// bfloat16 type mapping
#if defined(__HIPCC__)
#include <hip/hip_bf16.h>
#define __nv_bfloat16 __hip_bfloat16
#else
// Host-only: hip_bf16.h requires hipcc, but .cc files only reference the type
// in template parameters and kernel pointer tables. Provide a layout-compatible
// stub that links against the real __hip_bfloat16 from .cu object files.
struct __nv_bfloat16 { unsigned short __x; };
#endif

// Version macros — set to 0 to disable version-gated CUDA features
#ifndef CUDART_VERSION
#define CUDART_VERSION 0
#endif
#ifndef CUDA_VERSION
#define CUDA_VERSION 0
#endif
