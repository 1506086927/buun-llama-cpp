#pragma once

#include "ggml.h"
#include "ggml-backend.h"

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef GGML_USE_HIP
#define GGML_CUDA_NAME "ROCm"
#define GGML_CUBLAS_NAME "hipBLAS"
#elif defined(GGML_USE_MUSA)
#define GGML_CUDA_NAME "MUSA"
#define GGML_CUBLAS_NAME "muBLAS"
#else
#define GGML_CUDA_NAME "CUDA"
#define GGML_CUBLAS_NAME "cuBLAS"
#endif
#define GGML_CUDA_MAX_DEVICES       16

// backend API
GGML_BACKEND_API ggml_backend_t ggml_backend_cuda_init(int device);

// Dynamic VBR: transcode the first n_cells rows of a turbo KV tensor (src_A) to a lower turbo tier
// (type_B), writing into dst_B_data (a region of the KV pool buffer). dst_name must be the cache
// tensor name (cache_k_l<L> / cache_v_l<L>) so the encoder picks the right K/V codebook.
// stash_f16/stash_rows (nullable/0): f16 sink-stash — rows [0, stash_rows) re-encode from this
// pristine snapshot instead of the tier-A recon, capping the permanently-hot sink rows at
// single-hop error across any number of degrades. Capture it at the tensor's FIRST degrade.
GGML_BACKEND_API void ggml_backend_cuda_kv_transcode(ggml_backend_t backend,
                                                     const struct ggml_tensor * src_A, enum ggml_type type_B,
                                                     void * dst_B_data, ggml_backend_buffer_t pool_buf,
                                                     const char * dst_name, int64_t n_cells, bool is_v,
                                                     const void * stash_f16, int64_t stash_rows);
GGML_BACKEND_API void ggml_backend_cuda_kv_stash_capture(ggml_backend_t backend, const struct ggml_tensor * src,
                                                         void * stash_f16, int64_t n_rows, bool is_v);

// Block until all pending GPU work on the current device completes. Used to serialize a VBR transcode
// (which reads the live KV on a side backend/stream) against the model's KV writes. v1 brute-force
// equivalent of an event-based cudaStreamWaitEvent.
GGML_BACKEND_API void ggml_backend_cuda_sync_device(void);

GGML_BACKEND_API bool ggml_backend_is_cuda(ggml_backend_t backend);

// Dynamic VBR (S2): virtual-memory pool for the KV cache — one VA reservation, per-tensor fixed
// offsets, physical pages mapped on demand as occupancy grows and unmapped after tier degrades.
// Works on CUDA and ROCm (HIP maps the cuMem* driver API); *_available reports the device flag.
struct ggml_cuda_vmm_pool;
GGML_BACKEND_API bool   ggml_backend_cuda_vmm_available(int device);
GGML_BACKEND_API size_t ggml_backend_cuda_vmm_granularity(int device);
GGML_BACKEND_API struct ggml_cuda_vmm_pool * ggml_backend_cuda_vmm_pool_init(int device, size_t va_size);
GGML_BACKEND_API void   ggml_backend_cuda_vmm_pool_free(struct ggml_cuda_vmm_pool * pool);
GGML_BACKEND_API void * ggml_backend_cuda_vmm_pool_base(struct ggml_cuda_vmm_pool * pool);
GGML_BACKEND_API size_t ggml_backend_cuda_vmm_pool_mapped(struct ggml_cuda_vmm_pool * pool);
// ensure [off, off+len) is backed by physical pages (rounded out to granularity; new pages zeroed).
// false = physical memory exhausted (caller degrades or aborts); driver errors beyond OOM are fatal.
GGML_BACKEND_API bool   ggml_backend_cuda_vmm_pool_map(struct ggml_cuda_vmm_pool * pool, size_t off, size_t len);
// unmap chunks fully contained in [off, off+len); partially covered chunks stay mapped.
GGML_BACKEND_API bool   ggml_backend_cuda_vmm_pool_unmap(struct ggml_cuda_vmm_pool * pool, size_t off, size_t len);
// zero every mapped page (VMM-safe replacement for ggml_backend_buffer_clear on a partially-mapped VA)
GGML_BACKEND_API void   ggml_backend_cuda_vmm_pool_clear(struct ggml_cuda_vmm_pool * pool);

// wrap externally-managed device memory (e.g. a VMM VA range) as a CUDA backend buffer; the buffer
// does NOT take ownership — freeing it never cudaFree's ptr.
GGML_BACKEND_API ggml_backend_buffer_t ggml_backend_cuda_buffer_from_ptr(int device, void * ptr, size_t size);

// device buffer
GGML_BACKEND_API ggml_backend_buffer_type_t ggml_backend_cuda_buffer_type(int device);

// conduct allreduce operation between devices
GGML_BACKEND_API bool ggml_backend_cuda_allreduce_tensor(ggml_backend_t * backends, struct ggml_tensor ** tensors, size_t n_backends);

// split tensor buffer that splits matrices by rows across multiple devices
GGML_BACKEND_API ggml_backend_buffer_type_t ggml_backend_cuda_split_buffer_type(int main_device, const float * tensor_split);

// pinned host buffer for use with the CPU backend for faster copies between CPU and GPU
GGML_BACKEND_API ggml_backend_buffer_type_t ggml_backend_cuda_host_buffer_type(void);

GGML_BACKEND_API int  ggml_backend_cuda_get_device_count(void);
GGML_BACKEND_API void ggml_backend_cuda_get_device_description(int device, char * description, size_t description_size);
GGML_BACKEND_API void ggml_backend_cuda_get_device_memory(int device, size_t * free, size_t * total);

GGML_BACKEND_API bool ggml_backend_cuda_register_host_buffer(void * buffer, size_t size);
GGML_BACKEND_API void ggml_backend_cuda_unregister_host_buffer(void * buffer);

GGML_BACKEND_API ggml_backend_reg_t ggml_backend_cuda_reg(void);

#ifdef  __cplusplus
}
#endif
