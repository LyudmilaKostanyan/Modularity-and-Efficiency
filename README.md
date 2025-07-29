# Breaking Modularity for Efficiency

## Overview

This project benchmarks the performance of **modular** (`Multiply` and `Add`) versus **fused** (`FusedMulAdd`) floating-point operations. It aims to evaluate the impact of operation fusion on execution time, memory access efficiency, and computational throughput.

The benchmark can be run with configurable array size and number of repetitions, and results are presented in a neatly formatted performance comparison table.

---

## Problem Description

In performance-critical applications like scientific computing, real-time rendering, and finance, excessive function modularity can lead to redundant memory access and suboptimal instruction behavior. Even small inefficiencies per iteration can scale into significant slowdowns on large datasets.

The following execution styles are compared:

* **Separate operations**:
  `Multiply(a, b) → result`, followed by `Add(result, c) → result`
* **Fused operation**:
  `FusedMulAdd(a, b, c) → result` in a single loop pass

Execution time, speedup, and the underlying reasons for performance differences are measured and analyzed in both **batch (vector)** and **element-wise (single)** execution styles.

---

## Concepts Explained

### Memory Access Efficiency

**Memory access efficiency** refers to how effectively a program minimizes slow memory operations such as redundant reads/writes and cache misses. In modular execution:

```cpp
result[i] = a[i] * b[i];         // write to memory
result[i] = result[i] + c[i];    // read from and write to memory again
```

Each element is:

* Read multiple times (e.g., `a[i]`, `result[i]`, `c[i]`)
* Written to memory more than once

This increases:

* Cache pressure (L1/L2 cache fills quickly and flushes more often)
* Memory bus usage
* Total memory latency per iteration

In contrast, the fused version:

```cpp
result[i] = a[i] * b[i] + c[i];
```

performs all operations in one read-modify-write cycle, minimizing memory round-trips and improving cache locality. Especially for large arrays that exceed cache size, substantial gains in real-world performance are observed.

---

### Computational Throughput

**Computational throughput** is the number of useful operations a CPU can perform per second. Modern CPUs rely on:

* **SIMD vectorization** (e.g., AVX, SSE, NEON)
* **Pipelining** and **superscalar execution**
* **Specialized instructions**, like FMA (Fused Multiply-Add)

In modular design:

* Separate function calls introduce overhead (stack manipulation, control flow)
* Separate loops for `Multiply` and `Add` may inhibit vectorization
* Each operation involves memory access and instruction decoding

Fused operations enable:

* Reduced instruction count
* Improved instruction fusion by the compiler (`-O3`, `-ffast-math`)
* Efficient SIMD execution (processing 4–8 elements per register)
* Usage of `vfmaddps` (x86) or `vfma` (ARM) if supported

As a result, higher throughput with fewer instruction cycles and better hardware utilization is achieved.

---

### Execution Time and Speedup

Execution time is measured using `std::chrono::high_resolution_clock`. Each function is executed multiple times (based on `--repeats`) to reduce variance.

Speedup is computed as:

```
Speedup = (Multiply time + Add time) / FusedMulAdd time
```

Two execution styles are compared:

* **Batch mode**: processes full arrays in a loop (optimized, realistic for high-performance applications)
* **Single element mode**: calls operation functions element-by-element, simulating highly modularized or function-heavy code

Benchmark results demonstrate that:

* Fusion can improve performance by **up to 2x**
* Batch fusion reduces memory operations and benefits more from SIMD
* Single-element fusion minimizes the overhead of multiple function calls and instruction dispatching

---

## Example Output

```
== Benchmark Results ==
| Operation       | Batch Operations Time (ms)    | Single Element Operations Time (ms)|
|-----------------|-------------------------------|------------------------------------|
| Multiply        | 0.00163888                    | 0.28908929                         |
| Add             | 0.00101105                    | 0.28903351                         |
| FusedMulAdd     | 0.00159472                    | 0.28955610                         |
|-----------------|-------------------------------|------------------------------------|
| Separate Total  | 0.00264993                    | 0.57812280                         |
| Speedup (Fused) | 1.66169318                    | 1.99658304                         |
```

---

## Explanation of Output

* **"Multiply", "Add", "FusedMulAdd"** — average execution time across all repetitions
* **Batch operations** — vector-style processing via single loop
* **Single element operations** — invoking `Multiply`, `Add`, `FusedMulAdd` per element individually
* **Separate Total** — sum of `Multiply` and `Add`
* **Speedup (Fused)** — ratio of modular time to fused time (higher = better)

The fused version achieves lower execution time due to reduced memory pressure and enhanced compiler-level optimization.

---

## How to Compile and Run

### 1. Clone the Repository

```bash
git clone https://github.com/LyudmilaKostanyan/Modularity-and-Efficiency.git
cd Modularity-and-Efficiency
```

### 2. Build the Project

Use CMake:

```bash
cmake -S . -B build
cmake --build build
```

Requires:

* C++ compiler with C++17 support
* CMake ≥ 3.14

### 3. Run the Program

#### Example with arguments:

```bash
./build/main --size 10000 --repeats 10000
```

#### Or run with default settings:

```bash
./build/main
```

---

## Parameters

| Parameter   | Description                                          |
| ----------- | ---------------------------------------------------- |
| `--size`    | Number of elements in arrays (default: 10,000)       |
| `--repeats` | Number of benchmarking repetitions (default: 10,000) |

These parameters control workload size and statistical accuracy.