FetchContent_Declare(
    ktx
    URL https://github.com/KhronosGroup/KTX-Software/archive/d8be8ff731dfccef4419ad75d16d0b7a9930c345.zip # Change build image for deployment to VS2022.
)
set(KTX_FEATURE_TOOLS OFF CACHE INTERNAL "")
set(KTX_FEATURE_TESTS OFF CACHE INTERNAL "")
set(KTX_FEATURE_VULKAN OFF CACHE INTERNAL "")
set(KTX_FEATURE_GL_UPLOAD OFF CACHE INTERNAL "")
set(KTX_FEATURE_STATIC_LIBRARY ON CACHE INTERNAL "")
FetchDependency(ktx)

set_target_properties(ktx_read PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
set_target_properties(objUtil PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
set_target_properties(obj_basisu_cbind PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)

set_source_files_properties(
    ${ktx_SOURCE_DIR}/lib/basis_transcode.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/transcoder/basisu_transcoder.cpp

    ${ktx_SOURCE_DIR}/lib/basis_encode.cpp
    ${ktx_SOURCE_DIR}/lib/astc_encode.cpp

    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_backend.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_basis_file.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_bc7enc.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_comp.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_enc.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_etc.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_frontend.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_gpu_texture.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_kernels_sse.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_opencl.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_pvrtc1_4.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_resample_filters.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_resampler.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_ssim.cpp
    ${ktx_SOURCE_DIR}/lib/basisu/encoder/basisu_uastc_enc.cpp

    TARGET_DIRECTORY ktx
    PROPERTIES HEADER_FILE_ONLY ON
)

get_property(ASTC_TARGETS DIRECTORY ${ktx_SOURCE_DIR}/lib/astc-encoder/Source PROPERTY BUILDSYSTEM_TARGETS)
foreach(ASTC_TARGET ${ASTC_TARGETS})
    target_precompile_headers(${ASTC_TARGET} PRIVATE "astc-pch.h")
    set_target_properties(${ASTC_TARGET}
        PROPERTIES
        UNITY_BUILD ON
        UNITY_BUILD_BATCH_SIZE 64
    )

    set_source_files_properties(
        ${ktx_SOURCE_DIR}/lib/astc-encoder/Source/astcenc_color_unquantize.cpp
        ${ktx_SOURCE_DIR}/lib/astc-encoder/Source/astcenc_symbolic_physical.cpp
        ${ktx_SOURCE_DIR}/lib/astc-encoder/Source/astcenc_weight_align.cpp

        TARGET_DIRECTORY ${ASTC_TARGET}
        PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
    )
endforeach()
