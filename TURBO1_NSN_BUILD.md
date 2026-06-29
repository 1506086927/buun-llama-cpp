# turbo1_nsn (E3) — faithful NSNQuant centering build — IN PROGRESS

Branch `experiment/turbo1-1bit`, worktree `fork-1bit`. Builds on the turbo1 E0 codec.
Faithful NSNQuant (Son/Choi/Yoo, arXiv:2505.18231): token-norm → per-chunk per-head channel-center
→ renorm → FWHT → 1-bit sign. **1.25 bpw.** Goal: does per-head centering meaningfully beat turbo1
(0.084 median@16k) on the 1-bit codec? (Centering is already known-positive per-head — memory
[[project_perhead_kmean_gate_positive]]; pooled centering is a #135 trap, so it MUST be per-head.)

## Verified math (encode/decode round-trip)
Per 128-coord head group, chunk = the set_rows token batch:
- s1 = ‖v‖/√128 ; vn = v/s1  (‖vn‖=√128)
- o[c] = mean over tokens (this head) of vn[c]   ← computed in a reduction PASS 1
- vns = vn − o ; s2 = ‖vns‖/√128 ; x = vns/‖vns‖ (unit) → FWHT → sign
- store: s1 (fp16), s2 (fp16), 128 sign bits = 20 B/128 = 1.25 bpw ; o stored per (layer,head,channel)
- DECODE: x_recon = invFWHT(sign·σ), σ=1/√128 ; **v = s1·(s2·√128·x_recon + o[layer][head][channel])**
  Both K and V decode to ORIGINAL domain (per-row inv-FWHT, like turbo1's V path) — o is re-added in
  the reconstruction, so NO graph V-mean tap and NO softmax-cancellation trick (the s1 coupling means
  o does NOT cancel in Q·K, so K must reconstruct fully too).

## DONE (full build wired — compiling)
- `ggml.h`: GGML_TYPE_TURBO1_NSN = 49, COUNT = 50.
- `ggml-common.h`: `block_turbo1_nsn { ggml_half s1; ggml_half s2; uint8_t qs[16]; }` (20 B) + QK + assert.
- `turbo-quant-cuda.cuh`: `quantize_f32_turbo1_nsn_block(src, dst, o)` + `dequantize_turbo1_nsn`
  get_rows stub + QR_TURBO1_NSN + extern `turbo1_nsn_o_buf`.
- `ggml.c` type table + quantize dispatch; `ggml-quants.h` decls; `ggml-turbo-quant.c` CPU ref stub.
- `set-rows.cu`: `turbo1_nsn_o_buf` (the shared K/V per-layer buffer, defined here), `k_turbo1_nsn_reduce_o`
  (pass 1), `k_set_rows_turbo1_nsn` (pass 2), the 2-pass dispatch branch, extract gate.
- `fattn.cu`: `k_turbo1_nsn_dequant_f16` (decode reconstruction) + extern/PFHEAD defines + all dispatch
  gates (turbo_k/v, turbo_kv, turbo_k/v_only, turbo8_involved, is_turbo lambda, switch, VEC gate,
  Q-rot + fused exclusions) + 4 dequant branches (prefill K/V + decode K/V) with o_layer.
- `getrows.cu` case; `ggml-cuda.cu` set_rows + get_rows supports_op; `llama-kv-cache.cpp` 7 gate sites;
  `llama-context.cpp` 2 gates; `common/arg.cpp` CLI list.
- NOT in llama-graph V-unrotation gate (per-row decode → original domain). Correct.

## TODO (the rest of the build)
1. **o reduction kernel** (`set-rows.cu`): grid = n_heads (ne00/128), block 128 threads, loop over
   ne01 tokens; per token cooperative head-norm (√128/‖v_head‖), accumulate vn[c] into o[head*128+tid];
   write o[layer][c] = accum/ne01. Layout matches the affine tap: o indexed by absolute channel i00+j.
2. **o side-buffer**: static per-device `float* d_turbo1_nsn_o[GGML_CUDA_MAX_DEVICES]`, lazily alloc
   `PFHEAD_MAX_L * PFHEAD_MAX_C` floats. Encode pass1 writes `o + layer*PFHEAD_MAX_C`. Decode reads it.
   (Mirror kv_dequant_*_buf alloc pattern. Keyed by pf_layer = atoi(dst->name+9).)
3. **nsn quantize kernel** (`set-rows.cu`): custom k_set_rows_turbo1_nsn mirroring k_set_rows_quant's
   index math (i00,i01,src_block,dst_block) but calls `quantize_f32_turbo1_nsn_block(src_block,
   dst_block, o_layer + i00)`. Dispatch from the TURBO1_NSN branch: launch reduce → quantize.
4. **decode kernel** (`fattn.cu`): k_turbo1_nsn_dequant_f16 = per-row inv-FWHT (copy
   k_turbo1_dequant_f16_inv_fwht) then `v = s1*(s2*√128*val + o_layer[blockIdx.y*128+tid])`. Pass
   o_layer ptr + pf_layer. Used for BOTH K and V in prefill_attend + decode-dequant paths.
5. **dispatch wiring** (~14 sites, mirror turbo1, see commit `cuda(turbo1): E0`): fattn.cu turbo_k/v
   gates, K/V dequant branches, Q-rot exclusion, fused-exclusion, turbo_k_only/v_only, turbo8_involved
   (add nsn — no native VEC), get_best switch + VEC gate + is_turbo lambda; ggml.c type table +
   quantize dispatch + CPU ref stub (decl in ggml-quants.h); getrows.cu case; ggml-cuda.cu set_rows +
   get_rows supports_op; llama-kv-cache.cpp 5 gates; llama-context.cpp 2 gates; common/arg.cpp list.
   NOTE: turbo1_nsn V does NOT go in the llama-graph V-unrotation gate (per-row decode → original domain).
6. **CPU ref stub** (`ggml-turbo-quant.c`): quantize_row_turbo1_nsn_ref / dequantize_row_turbo1_nsn —
   centering needs o (GPU-only); stub as plain turbo1 (never called for KV) to satisfy the type table.
7. Build on Dorei, sanity coherence, measure median@16k + 2k/8k vs turbo1.

## Risks / watch
- The o side-buffer handoff is stateful across encode→decode; only valid for prefill (KLD harness ok;
  autoregressive single-token decode would need NSNQuant's 64-token residual machinery — out of scope).
- Re-applies the turbo1 lessons: nsn has no native VEC kernel (force dequant via turbo8_involved);
  per-row decode avoids the pooled-FWHT-V cancellation. See [[reference_turbo1_1bit_pooled_fwht_cancellation]].
- s1/s2 fp16 precision: s1 can be large (raw norm); fine in fp16.
