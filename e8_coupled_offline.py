#!/usr/bin/env python3
# E8 offline: does Coupled Quantization (joint c-channel codebook, k-means) beat independent 1-bit
# sign at MATCHED bitrate (1 bit/channel = 2^c codewords for c channels)? Test on RAW (correlated)
# vs POST-FWHT (decorrelated) KV. If CQ only wins on RAW, the FWHT already captured the structure.
import numpy as np
from scipy.linalg import hadamard

S1 = np.array([-1,1,1,-1,-1,1,-1,1,-1,-1,1,1,1,1,1,1,1,-1,1,-1,1,-1,-1,1,1,1,-1,1,1,-1,-1,-1,-1,1,1,-1,1,1,-1,1,-1,1,1,-1,-1,1,-1,1,1,1,1,-1,-1,-1,-1,-1,1,-1,1,1,1,1,-1,1,-1,-1,1,-1,-1,-1,1,-1,-1,-1,1,-1,-1,-1,1,1,1,-1,-1,1,1,1,-1,-1,1,1,-1,1,1,-1,1,-1,-1,1,1,-1,1,-1,1,-1,1,1,1,1,-1,1,-1,1,1,-1,1,1,-1,-1,-1,-1,-1,1,1,-1,1,1,-1,1],dtype=np.float64)
S2 = np.array([1,1,1,1,-1,1,1,-1,1,-1,-1,-1,1,-1,-1,-1,1,1,-1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,1,1,1,1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,-1,1,1,1,-1,-1,1,-1,-1,-1,-1,-1,-1,1,1,1,-1,1,-1,-1,-1,-1,1,-1,1,-1,1,-1,-1,1,1,-1,1,-1,1,1,-1,1,-1,-1,-1,-1,1,-1,-1,1,-1,1,-1,1,1,1,-1,-1,1,-1,1,-1,1,1,-1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,-1,-1,-1,-1,1,-1],dtype=np.float64)
H = hadamard(128).astype(np.float64); INV=1.0/np.sqrt(128.0)
def fwht_inv(x): return (S1*(INV*(S2*x)@H.T))

post = np.fromfile('/tmp/turbo_postrot.bin',dtype=np.float32).reshape(-1,128).astype(np.float64)[:40000]
raw = fwht_inv(post); raw/=np.linalg.norm(raw,axis=1,keepdims=True)+1e-12
post = post/ (np.linalg.norm(post,axis=1,keepdims=True)+1e-12)
rng=np.random.default_rng(0)

def kmeans(X,K,iters=25):
    idx=rng.choice(len(X),K,replace=False); C=X[idx].copy()
    for _ in range(iters):
        d=((X[:,None,:]-C[None,:,:])**2).sum(2); a=d.argmin(1)
        for k in range(K):
            m=a==k
            if m.any(): C[k]=X[m].mean(0)
    return C

def indep_sign_mse(X):                      # 1-bit sign per coord, sigma=1/sqrt(d) (norm-preserving)
    d=X.shape[1]; recon=np.sign(X)*(1.0/np.sqrt(d)); recon[recon==0]=1.0/np.sqrt(d)
    return ((recon-X)**2).sum(1).mean()

def cq_mse(X,c):                            # group c channels, 2^c-entry k-means codebook (1 bit/ch)
    d=X.shape[1]; ng=d//c; K=2**c
    subs=X[:, :ng*c].reshape(-1,c)          # [N*ng, c]
    samp=subs[rng.choice(len(subs),min(40000,len(subs)),replace=False)]
    C=kmeans(samp,K)
    dist=((subs[:,None,:]-C[None,:,:])**2).sum(2); a=dist.argmin(1)
    recon=C[a].reshape(X.shape[0],ng*c)
    # MSE over the quantized channels, summed per row (comparable to indep over same channels)
    return ((recon-X[:, :ng*c])**2).sum(1).mean() * (d/(ng*c))

for name,D in [("RAW(correlated)",raw),("POST-FWHT(decorr)",post)]:
    base=indep_sign_mse(D)
    print(f"\n{name}: independent-sign MSE = {base:.4f}")
    for c in (4,8):
        m=cq_mse(D,c)
        print(f"   CQ c={c} (2^{c}={2**c} codewords, 1 bit/ch): MSE={m:.4f}  ({100*(base-m)/base:+.1f}% vs indep)")
print("\n(CQ should beat independent on RAW; if it does NOT beat on POST-FWHT, FWHT already captured the dependence)")
