# Cache Simulation and Optimization in Spike (C/C++)

## Overview
This project implements a FIFO cache replacement policy in the Spike RISC-V simulator and optimizes matrix transpose and matrix multiplication algorithms to reduce cache miss rate and memory access overhead.

The work combines hardware-level cache policy modification with software-level locality optimization.

---

## Part 1 – FIFO Cache Replacement Policy

Modified `cache_sim_t::victimize()` in Spike to replace the default random policy with FIFO.

### Key Design
- Each set maintains a FIFO queue to track insertion order.
- On miss:
  - If space available → allocate empty slot.
  - If full → evict oldest entry.
- Dirty lines trigger writeback.
- Fully associative cache implemented with `std::queue<uint64_t>`.

### Concepts Applied
- Set indexing and tag extraction
- Writeback handling
- Cache miss tracking
- Replacement policy design

---

## Part 2 – Cache-Aware Algorithm Optimization

### Matrix Transpose (Blocking)
- Applied 8×8 tiling.
- Aligned with 32-byte cache line (8 integers).
- Improved spatial locality.

Miss rate:
6.165% → 2.795%  
Improvement Ratio: 1.70×

---

### Matrix Multiplication (3D Blocking)
- 3-level blocking over i, j, k dimensions.
- Reused sub-blocks to maximize data reuse.

Miss rate:
7.764% → 0.692%  
Improvement Ratio: 3.98×

---

## Experimental Results

| Algorithm | Miss Rate (Original → Optimized) | Speedup |
|-----------|-----------------------------------|---------|
| Transpose | 6.1% → 2.6% | ~1.7× |
| Multiply  | 7.6% → 0.5% | ~4.2× |

---

## Skills Demonstrated

- Cache architecture understanding
- Replacement policy implementation
- Memory locality optimization
- Performance modeling (cycle estimation)
- C/C++ systems programming
