#!/usr/bin/env python3
# Train the turbo1_cq codebook: 256 codewords x 8-dim, k-means on POST-FWHT unit-block sub-vectors.
# Emit a C header (d_turbo1_cq_codebook[256*8]). Norm convention: blocks normalized to unit before
# FWHT (as in the codec), so codewords live in the unit-block sub-vector space.
import numpy as np
post = np.fromfile('/tmp/turbo_postrot.bin',dtype=np.float32).reshape(-1,128).astype(np.float64)
post /= (np.linalg.norm(post,axis=1,keepdims=True)+1e-12)   # unit blocks (codec normalizes)
subs = post[:, :128].reshape(-1,8)                          # [N*16, 8] 8-dim sub-vectors
print("sub-vectors:", subs.shape)
rng=np.random.default_rng(42)
K=256
# k-means++ init
idx=[rng.integers(len(subs))];
d2=((subs-subs[idx[0]])**2).sum(1)
for _ in range(K-1):
    p=d2/d2.sum(); i=rng.choice(len(subs),p=p); idx.append(i)
    d2=np.minimum(d2, ((subs-subs[i])**2).sum(1))
C=subs[idx].copy()
for it in range(60):
    # assign in chunks (memory)
    a=np.empty(len(subs),dtype=np.int32)
    for s in range(0,len(subs),200000):
        e=min(s+200000,len(subs))
        a[s:e]=((subs[s:e,None,:]-C[None,:,:])**2).sum(2).argmin(1)
    newC=C.copy()
    for k in range(K):
        m=a==k
        if m.any(): newC[k]=subs[m].mean(0)
    shift=np.abs(newC-C).max(); C=newC
    if it%10==0 or shift<1e-5: print(f" iter {it} shift {shift:.6f}")
    if shift<1e-5: break
# report final MSE vs independent sign
recon=C[a].reshape(post.shape[0],128)
mse_cq=((recon-post)**2).sum(1).mean()
rs=np.sign(post)*(1/np.sqrt(128.0)); mse_s=((rs-post)**2).sum(1).mean()
print(f"final: CQ MSE {mse_cq:.4f} vs sign {mse_s:.4f}  ({100*(mse_s-mse_cq)/mse_s:+.1f}%)")
# emit C header
with open('/tmp/turbo1_cq_codebook.h','w') as f:
    f.write("// turbo1_cq codebook: 256 codewords x 8 dims (k-means on post-FWHT unit-block sub-vectors)\n")
    f.write("static const float TURBO1_CQ_CODEBOOK[256*8] = {\n")
    flat=C.reshape(-1)
    for i in range(0,len(flat),8):
        f.write("  "+",".join(f"{v:.8f}f" for v in flat[i:i+8])+",\n")
    f.write("};\n")
print("wrote /tmp/turbo1_cq_codebook.h")
