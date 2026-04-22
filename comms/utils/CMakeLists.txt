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

file(GLOB_RECURSE UTILS_SOURCES
    "*.cc"
    "*.cu"
)
file(GLOB_RECURSE UTILS_CU_SOURCES
    "*.cu"
)
list(FILTER UTILS_SOURCES EXCLUDE REGEX ".*/tests/.*")
list(FILTER UTILS_SOURCES EXCLUDE REGEX ".*/test_utils/.*")
list(FILTER UTILS_CU_SOURCES EXCLUDE REGEX ".*/tests/.*")
list(FILTER UTILS_CU_SOURCES EXCLUDE REGEX ".*/test_utils/.*")

add_library(utils OBJECT
    ${UTILS_SOURCES}
    ${UTILS_CU_SOURCES}
)

target_compile_features(utils PRIVATE cxx_std_20)
target_compile_options(utils PRIVATE
    -fPIC
)

if(USE_ROCM)
    target_compile_definitions(utils PRIVATE
        USE_ROCM
        __HIP_PLATFORM_AMD__
        __CUDA_BF16_TYPES_EXIST__
    )
    foreach(src ${UTILS_CU_SOURCES})
        set_source_files_properties(${src} PROPERTIES LANGUAGE HIP)
    endforeach()
    set_target_properties(utils PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
else()
    set_target_properties(utils PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
        POSITION_INDEPENDENT_CODE ON
    )
endif()

target_include_directories(utils PUBLIC
    ${ROOT}
    ${CONDA_INCLUDE}
)
if(USE_ROCM)
    target_include_directories(utils BEFORE PUBLIC
        ${ROOT}/comms/torchcomms/compat
    )
    target_include_directories(utils PUBLIC
        ${ROCM_INCLUDE}
        ${ROCM_INCLUDE}/hip
    )
else()
    target_include_directories(utils PUBLIC
        ${CUDA_INCLUDE_DIRS}
    )
endif()
