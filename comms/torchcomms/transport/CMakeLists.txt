# Copyright (c) Meta Platforms, Inc. and affiliates.
# Extension: torchcomms._transport
file(GLOB TORCHCOMMS_TRANSPORT_SOURCES
    "comms/torchcomms/transport/*.cpp"
)

if(USE_ROCM)
    enable_language(HIP)

    set(ROCM_HOME $ENV{ROCM_HOME})
    if(NOT ROCM_HOME)
        set(ROCM_HOME "/opt/rocm")
    endif()
    set(ROCM_INCLUDE "${ROCM_HOME}/include")
else()
    find_package(CUDA)
    if(NOT CUDA_FOUND)
        message(
            FATAL_ERROR
            "Transport build requires CUDA or ROCm. \
Install the CUDA toolkit, set up ROCm, or rerun with USE_TRANSPORT=OFF.")
    endif()

    if(NOT DEFINED ENV{NVCC_GENCODE})
        set(ENV{NVCC_GENCODE} "-gencode=arch=compute_90,code=sm_90")
        message(STATUS "ENV NVCC_GENCODE: $ENV{NVCC_GENCODE}")
    endif()
    set(NVCC_GENCODE $ENV{NVCC_GENCODE})
    message(STATUS "NVCC_GENCODE: ${NVCC_GENCODE}")

    if(NOT DEFINED ENV{TORCH_CUDA_ARCH_LIST})
        set(ENV{TORCH_CUDA_ARCH_LIST} "9.0a")
    endif()
    set(TORCH_CUDA_ARCH_LIST $ENV{TORCH_CUDA_ARCH_LIST})
    message(STATUS "TORCH_CUDA_ARCH_LIST: ${TORCH_CUDA_ARCH_LIST}")
    add_compile_options($<$<COMPILE_LANGUAGE:CUDA>:${NVCC_GENCODE}>)
endif()

add_compile_definitions(MOCK_SCUBA_DATA CTRAN_DISABLE_TCPDM FMT_USE_CONSTEXPR)

# CTranIB
add_subdirectory(comms/utils)
add_subdirectory(comms/ctran)

# Get folly LDFLAGS using pkg-config
set(ENV{PKG_CONFIG_PATH} "${CONDA_LIB}/pkgconfig")
execute_process(
    COMMAND pkg-config --libs --static libfolly
    OUTPUT_VARIABLE FOLLY_LDFLAGS_RAW
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE PKG_RESULT
)
if(NOT PKG_RESULT EQUAL 0)
    message(FATAL_ERROR "pkg-config for libfolly failed")
endif()
separate_arguments(FOLLY_LDFLAGS NATIVE_COMMAND "${FOLLY_LDFLAGS_RAW}")
# Remove glog/gflags/fmt from FOLLY_LDFLAGS — they're already linked
# statically via libtorchcomms.so from the third-party cmake modules.
# pkg-config may output -lglog, -l:libglog.a, or full paths like
# /path/to/libglog.a — the simple substring match handles all forms.
list(FILTER FOLLY_LDFLAGS EXCLUDE REGEX "glog")
list(FILTER FOLLY_LDFLAGS EXCLUDE REGEX "gflags")
list(FILTER FOLLY_LDFLAGS EXCLUDE REGEX "libfmt")

add_library(torchcomms_transport MODULE
    ${TORCHCOMMS_TRANSPORT_SOURCES}
)
set_target_properties(torchcomms_transport PROPERTIES
    PREFIX ""
    OUTPUT_NAME "_transport"
    SUFFIX ".${Python3_SOABI}.so"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/comms/torchcomms"
)

if(NOT USE_ROCM)
    set_target_properties(torchcomms_transport PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
        POSITION_INDEPENDENT_CODE ON
    )
else()
    set_target_properties(torchcomms_transport PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
endif()

target_include_directories(torchcomms_transport PRIVATE
    ${ROOT}
    ${CONDA_INCLUDE}
    ${Python3_INCLUDE_DIRS}
)
if(USE_ROCM)
    target_include_directories(torchcomms_transport PRIVATE
        ${ROCM_INCLUDE}
        ${ROCM_INCLUDE}/hip
    )
endif()

target_compile_features(torchcomms_transport PRIVATE cxx_std_20)
target_link_directories(torchcomms_transport PRIVATE ${CONDA_LIB})
target_link_libraries(torchcomms_transport PRIVATE
    ${TORCH_LIBRARIES}
    ${TORCH_PYTHON_LIB}
    ctran
    utils
)
if(USE_SYSTEM_LIBS)
    target_link_libraries(torchcomms_transport PRIVATE
        "-lfolly"
        glog::glog gflags::gflags fmt::fmt
        "-lboost_program_options"
        "-lboost_filesystem"
        "-lboost_context"
        ${FOLLY_LDFLAGS}
    )
else()
    target_link_libraries(torchcomms_transport PRIVATE
        "-l:libfolly.a"
        glog::glog gflags::gflags fmt::fmt
        "-l:libboost_program_options.a"
        "-l:libboost_filesystem.a"
        "-l:libboost_context.a"
        ${FOLLY_LDFLAGS}
    )
endif()

if(USE_ROCM)
    target_link_libraries(torchcomms_transport PRIVATE
        "${ROCM_HOME}/lib/libamdhip64.so"
    )
elseif(CUDA_FOUND)
    target_link_libraries(torchcomms_transport PRIVATE CUDA::cudart)
endif()

install(TARGETS torchcomms_transport
    LIBRARY DESTINATION .
)
