/** Copyright (c) 2022 NVIDIA CORPORATION.  All rights reserved.
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#pragma once

// defines all crt + builtin types
#include "builtin.h"

// this is the core runtime API exposed on the DLL level
extern "C"
{
    WP_API int init();
    //WP_API void shutdown();

    WP_API void* alloc_host(size_t s);
    WP_API void* alloc_device(size_t s);

    WP_API void free_host(void* ptr);
    WP_API void free_device(void* ptr);

    // all memcpys are performed asynchronously
    WP_API void memcpy_h2h(void* dest, void* src, size_t n);
    WP_API void memcpy_h2d(void* dest, void* src, size_t n);
    WP_API void memcpy_d2h(void* dest, void* src, size_t n);
    WP_API void memcpy_d2d(void* dest, void* src, size_t n);
    WP_API void memcpy_peer(void* dest, void* dest_ctx, void* src, void* src_ctx, size_t n);

    // all memsets are performed asynchronously
    WP_API void memset_host(void* dest, int value, size_t n);
    WP_API void memset_device(void* dest, int value, size_t n);

    // create a user-accesible copy of the mesh, it is the 
    // users reponsibility to keep-alive the points/tris data for the duration of the mesh lifetime
	WP_API uint64_t mesh_create_host(wp::vec3* points, wp::vec3* velocities, int* tris, int num_points, int num_tris);
	WP_API void mesh_destroy_host(uint64_t id);
    WP_API void mesh_refit_host(uint64_t id);

	WP_API uint64_t mesh_create_device(wp::vec3* points, wp::vec3* velocities, int* tris, int num_points, int num_tris);
	WP_API void mesh_destroy_device(uint64_t id);
    WP_API void mesh_refit_device(uint64_t id);

    WP_API uint64_t hash_grid_create_host(int dim_x, int dim_y, int dim_z);
    WP_API void hash_grid_reserve_host(uint64_t id, int num_points);
    WP_API void hash_grid_destroy_host(uint64_t id);
    WP_API void hash_grid_update_host(uint64_t id, float cell_width, const wp::vec3* positions, int num_points);

    WP_API uint64_t hash_grid_create_device(int dim_x, int dim_y, int dim_z);
    WP_API void hash_grid_reserve_device(uint64_t id, int num_points);
    WP_API void hash_grid_destroy_device(uint64_t id);
    WP_API void hash_grid_update_device(uint64_t id, float cell_width, const wp::vec3* positions, int num_points);

    WP_API uint64_t volume_create_host(void* buf, uint64_t size);
    WP_API void volume_get_buffer_info_host(uint64_t id, void** buf, uint64_t* size);
    WP_API void volume_destroy_host(uint64_t id);

    WP_API uint64_t volume_create_device(void* buf, uint64_t size);
    WP_API void volume_get_buffer_info_device(uint64_t id, void** buf, uint64_t* size);
    WP_API void volume_destroy_device(uint64_t id);

    WP_API uint64_t marching_cubes_create_device();
    WP_API void marching_cubes_destroy_device(uint64_t id);
    WP_API int marching_cubes_surface_device(uint64_t id, const float* field, int nx, int ny, int nz, float threshold, wp::vec3* verts, int* triangles, int max_verts, int max_tris, int* out_num_verts, int* out_num_tris);

    WP_API void array_inner_host(uint64_t a, uint64_t b, uint64_t out, int len);
    WP_API void array_sum_host(uint64_t a, uint64_t out, int len);

    WP_API void array_inner_device(uint64_t a, uint64_t b, uint64_t out, int len);
    WP_API void array_sum_device(uint64_t a, uint64_t out, int len);

    // return cudaError_t code
    WP_API uint64_t cuda_check_device();

    WP_API int cuda_device_get_count();
    WP_API void* cuda_device_get_primary_context(int ordinal);
    WP_API const char* cuda_device_get_name(int ordinal);
    WP_API int cuda_device_get_arch(int ordinal);
    WP_API int cuda_device_is_uva(int ordinal);

    WP_API void* cuda_context_get_current();
    WP_API void cuda_context_set_current(void* context);
    WP_API void cuda_context_push_current(void* context);
    WP_API void cuda_context_pop_current();
    WP_API void* cuda_context_create(int device_ordinal);
    WP_API void cuda_context_destroy(void* context);
    WP_API int cuda_context_get_device_ordinal(void* context);
    WP_API int cuda_context_is_primary(void* context);
    WP_API void* cuda_context_get_stream(void* context);
    WP_API int cuda_context_can_access_peer(void* context, void* peer_context);
    WP_API int cuda_context_enable_peer_access(void* context, void* peer_context);

    // ensures all device side operations have completed in the current context
    WP_API void cuda_context_synchronize();

    WP_API void* cuda_stream_get_current();

    WP_API void cuda_graph_begin_capture();
    WP_API void* cuda_graph_end_capture();
    WP_API void cuda_graph_launch(void* graph);
    WP_API void cuda_graph_destroy(void* graph);

    WP_API size_t cuda_compile_program(const char* cuda_src, int arch, const char* include_dir, bool debug, bool verbose, bool verify_fp, const char* output_file);
    WP_API void* cuda_load_module(const char* ptx);
    WP_API void cuda_unload_module(void* module);
    WP_API void* cuda_get_kernel(void* module, const char* name);
    WP_API size_t cuda_launch_kernel(void* kernel, size_t dim, void** args);

} // extern "C"

namespace wp
{
enum class DeviceType { CPU, CUDA };

template<DeviceType Source, DeviceType Target> void memcpy(void* dest, void* src, size_t n);
template<> inline void memcpy<DeviceType::CPU, DeviceType::CPU>(void* dest, void* src, size_t n)   { memcpy_h2h(dest, src, n); }
template<> inline void memcpy<DeviceType::CPU, DeviceType::CUDA>(void* dest, void* src, size_t n)  { memcpy_h2d(dest, src, n); }
template<> inline void memcpy<DeviceType::CUDA, DeviceType::CPU>(void* dest, void* src, size_t n)  { memcpy_d2h(dest, src, n); }
template<> inline void memcpy<DeviceType::CUDA, DeviceType::CUDA>(void* dest, void* src, size_t n) { memcpy_d2d(dest, src, n); }

template<DeviceType device> void* alloc(size_t s);
template<> inline void* alloc<DeviceType::CPU>(size_t s)  { return alloc_host(s); }
template<> inline void* alloc<DeviceType::CUDA>(size_t s) { return alloc_device(s); }

template<DeviceType device> void free(void* ptr);
template<> inline void free<DeviceType::CPU>(void* ptr)  { free_host(ptr); }
template<> inline void free<DeviceType::CUDA>(void* ptr) { free_device(ptr); }
} // namespace wp
