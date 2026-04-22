# Copyright (c) Meta Platforms, Inc. and affiliates.

if(USE_ROCM)
    enable_language(HIP)

    set(ROCM_HOME $ENV{ROCM_HOME})
    if(NOT ROCM_HOME)
        set(ROCM_HOME "/opt/rocm")
    endif()
    set(ROCM_INCLUDE "${ROCM_HOME}/include")
else()
    find_package(CUDA REQUIRED)
endif()

file(GLOB_RECURSE CTRAN_SOURCES
    "*.cc"
    "*.cu"
)
list(FILTER CTRAN_SOURCES EXCLUDE REGEX ".*/tests/.*")
list(FILTER CTRAN_SOURCES EXCLUDE REGEX ".*/benchmarks/.*")
list(FILTER CTRAN_SOURCES EXCLUDE REGEX ".*/examples/.*")
if (NOT DEFINED ENABLE_TCPDM OR NOT ENABLE_TCPDM)
    add_compile_definitions(CTRAN_DISABLE_TCPDM)
    list(FILTER CTRAN_SOURCES EXCLUDE REGEX ".*/ctran/backends/tcpdevmem/.*")
endif()

if(NOT USE_ROCM)
    list(FILTER CMAKE_CUDA_FLAGS EXCLUDE REGEX ".*arch=compute_50,code=sm_50")
    list(FILTER CMAKE_CUDA_FLAGS EXCLUDE REGEX ".*arch=compute_60,code=sm_60")
    list(FILTER CMAKE_CUDA_FLAGS EXCLUDE REGEX ".*arch=compute_70,code=sm_70")
    list(FILTER CMAKE_CUDA_FLAGS EXCLUDE REGEX ".*arch=compute_75,code=sm_75")

    message(STATUS "CMAKE_CUDA_FLAGS: ${CMAKE_CUDA_FLAGS}")
endif()

# genctran.py generates source files (*.cu) for template instantiations across
# different algorithms, data types, and operations.
execute_process(
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/algos/genctran.py
        ${CMAKE_CURRENT_BINARY_DIR}/gensrc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
file(GLOB CTRAN_GENSRC_FILES "${CMAKE_CURRENT_BINARY_DIR}/gensrc/*.cu")

add_library(ctran OBJECT
  ${CTRAN_GENSRC_FILES}
  ${CTRAN_SOURCES}
)

target_compile_features(ctran PRIVATE cxx_std_20)

if(USE_ROCM)
    target_compile_definitions(ctran PRIVATE
        USE_ROCM
        __HIP_PLATFORM_AMD__
        # BF16 types only available in .cu files compiled by hipcc
        $<$<COMPILE_LANGUAGE:HIP>:__CUDA_BF16_TYPES_EXIST__>
    )
endif()

target_compile_options(ctran PRIVATE
    -fPIC
)

if(USE_ROCM)
    # Only .cu files are compiled with hipcc; .cc files stay with g++
    foreach(src ${CTRAN_GENSRC_FILES} ${CTRAN_SOURCES})
        if(src MATCHES "\\.cu$")
            set_source_files_properties(${src} PROPERTIES LANGUAGE HIP)
        endif()
    endforeach()
else()
    target_compile_options(ctran PRIVATE
        $<$<COMPILE_LANGUAGE:CUDA>:--expt-extended-lambda -Xptxas -maxrregcount=96 -Xfatbin -compress-all>
    )
endif()

if (NOT DEFINED ENABLE_TCPDM OR NOT ENABLE_TCPDM)
    if(USE_ROCM)
        target_compile_definitions(ctran PRIVATE CTRAN_DISABLE_TCPDM)
    else()
        target_compile_options(ctran PRIVATE
            $<$<COMPILE_LANGUAGE:CUDA>:--compiler-options "-DCTRAN_DISABLE_TCPDM">
        )
    endif()
endif()

if(NOT USE_ROCM)
    set_target_properties(ctran PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
        POSITION_INDEPENDENT_CODE ON
    )
else()
    set_target_properties(ctran PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
endif()

target_include_directories(ctran PRIVATE
    ${ROOT}
    ${CONDA_INCLUDE}
)
if(USE_ROCM)
    # Compat headers must come first so #include <cuda_runtime.h> resolves to
    # our HIP wrapper rather than a missing CUDA toolkit header.
    target_include_directories(ctran BEFORE PRIVATE
        ${ROOT}/comms/torchcomms/compat
    )
    target_include_directories(ctran PRIVATE
        ${ROCM_INCLUDE}
        ${ROCM_INCLUDE}/hip
    )
else()
    target_include_directories(ctran PRIVATE
        ${CUDA_INCLUDE_DIRS}
    )
endif()
