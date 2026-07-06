# VBR: variable-bit-rate KV cache

VBR runs the KV cache on the TurboQuant codec ladder (turbo8 → turbo4 → turbo3_tcq →
turbo2_tcq → turbo1_tcq, 8.125 → 1.25 effective bits/value) and, in dynamic mode, changes
tiers **at runtime**: the cache starts at the high-quality entry tier and individual
(layer, K/V) tensors are transcoded down the ladder in place as the context fills, following
a measured per-model price order. You get near-entry-tier quality at short contexts and
bottom-tier capacity at full depth, from one flag.

## Quickstart

```sh
llama-server -m model.gguf -ctk vbr
```

That single flag is the complete product:

- the cache starts at the **turbo8** entry tier;
- a KV VRAM budget is derived automatically from the memory left after weights/compute
  (the fit pass), or falls back to the floor-layout cost of the full context;
- when `-c` is unset, the advertised context is the budget's capacity at the **floor**
  tier (t1 by default), capped at the model's training context;
- as mapped KV approaches the budget, the runtime controller degrades tensors down the
  price order, cheapest quality-per-byte first. With `-v` you can watch the
  `VBR degrade #…` steps fire.

The cache resets losslessly when it empties (tiers return to entry), and fully-degraded
runs stay coherent — the price orders were measured on KLD panels per model.

## Flags

| flag | meaning |
|---|---|
| `-ctk vbr` / `-ctv vbr` / `-ct vbr` | select VBR for K, V, or both. One side can stay a plain type (`-ctv f16`). |
| `--vbr-budget <tier\|number>` (`--vbr-bits`) | `dynamic` (default) = runtime controller. A tier (`t8/t4/t3/t2/t1`) or a number selects a **fixed** static tier instead — no runtime degrades. |
| `--vbr-floor <bits\|tier>` (`--vbr-min-bits`) | LITERAL aggregate bits/value floor for dynamic mode. The degrade order stops at the last step whose aggregate stays ≥ the floor — e.g. `4.25` means "t4 layout with a few units held one tier higher", **not** a snap-up to the next tier. Clamped to the ladder range [1.25, 8.125]. Default: t1 (1.25). |
| `--vbr-vram <SIZE>` | explicit KV VRAM budget (e.g. `8G`). Default `auto` = derived by the fit pass. |
| `--vbr-policy <json>` | fixed mode only: a measured policy ladder; the best rung ≤ the fixed budget (and ≥ the floor) is selected and its per-layer schedule applied. |

Notes:

- The C API mirror is `llama_context_params.vbr_dynamic` / `vbr_vram_budget_bytes` /
  `vbr_min_bits` — that struct is the runtime channel; the CLI flags map onto it.
- A non-tier floor costs slightly more than the tier below it; capacity estimates account
  for this (the advertised context is scaled to the literal floor's cost).
- `/props` and `/models` on the server expose a `vbr` object (mode, floor, budget,
  realized/selected bits — `null` where a value is not a fixed number under the dynamic
  controller).

## Requirements and limitations

- **Backend**: turbo-typed KV needs a backend that exports the TurboQuant interface
  (`ggml-vbr.h`) — currently CUDA (and ROCm for the VMM pool). KV on other backends is
  refused at init; layers whose KV lands on the CPU (partial offload, `--no-kv-offload`)
  fall back to q8_0 per layer with a warning.
- **Flash attention** is required and force-enabled (turbo KV stores rotated-space data
  that only the FA paths decode).
- **Unified KV**: dynamic mode needs single-stream KV; with parallel sequences
  (`-np > 1`, perplexity/imatrix multi-sequence batching) unified KV is forced
  automatically, with a warning.
- **Context shift / self-extend** is disabled under dynamic VBR (the shift graph would
  touch unmapped pool pages); generation stops cleanly when the context fills.
- **State save/load** (session files, server slot save, prompt-cache-ram, cache reuse) is
  not supported in dynamic mode: snapshots carry tier-typed KV, and a degraded-tier save
  could never restore. Saves are refused loudly; the server disables these features on
  startup with warnings. Context checkpoints stay enabled on hybrid models (recurrent
  state only) but are disabled on SWA models.
- Degrade progress lines are `INFO`-level: visible with `-v`. Greedy-identical output does
  NOT mean no degrades happened.

## Degrade orders

The price order — which (layer, side) to degrade next, per tier band — is measured per
model on KLD panels and baked into the binary for the supported fleet
(`src/llama-vbr-degrade-orders.inc`). Models without a baked order use a **generic**
cross-model order synthesized from normalized-position rank curves averaged over the
measured fleet (dense, MoE-hybrid and SWA-mixed layouts); it is a good hedge but a
measured order is better. Override or supply one with:

```sh
VBR_DEGRADE_ORDER=order.txt llama-server ...   # tokens: <layer><k|v>:<t8|t4|t3|t2|t1>
```

`VBR_FORCE_GENERIC=1` forces the generic order on a supported model (A/B instrument).

## Environment appendix

CLI-managed internals (set nothing yourself; these are developer overrides on top of the
cparams channel): `VBR_VMM` (=0 forces the controller off, =1 on), `VBR_MODE`,
`VBR_BUDGET_MIB`, `VBR_MIN_BITS`.

Product-adjacent knobs: `VBR_DEGRADE_ORDER` (order file), `VBR_LAYER_SCHEDULE`
(per-layer static schedule, `<il0>-<il1>:<k|v>:<tier>;…` or `@file`; `VBR_SCHEDULE_CTX`
sets the discovery window the row coordinates refer to, `VBR_LAYER_STRICT=1` errors on
unsupported bands), `VBR_POLICY_LADDER` (default for `--vbr-policy auto`),
`VBR_STASH_ROWS` (f16 sink-stash rows per tensor, default 128; 0 disables),
`VBR_FORCE_GENERIC`.

Developer instruments (off by default, safe to ignore): `VBR_VRAM_HEADROOM_MIB` (free-VRAM
clamp headroom, default 192), `VBR_PROMOTE` (container promotion kill switch, default on),
`VBR_TRANSCODE_TEST[_N]` (in-binary transcode oracle), `VBR_TRANSCODE_FIDELITY`
(per-transcode error report; slow), `VBR_TRANSCODE_NOTILE` (tiling isolator; unbounded
scratch), `VBR_STASH_CAPTURE_ONLY` (stash ablation).

Telemetry exports for wrapper scripts (written, never read): `VBR_SELECTED_*`,
`VBR_CAPACITY_BITS`, `VBR_VRAM_BUDGET`, `VBR_BUDGET`.
