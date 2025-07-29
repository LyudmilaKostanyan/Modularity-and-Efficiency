#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cstring>
#include <string>
#include <iomanip>

void Multiply(const float a, const float b, float* result)
{
    if (result)
        *result = a * b;
}

void Multiply(const float* a, const float* b, float* result, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        result[i] = a[i] * b[i];
}

void Add(const float a, const float b, float* result)
{
    if (result)
        *result = a + b;
}

void Add(const float* a, const float* b, float* result, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        result[i] = a[i] + b[i];
}

void FusedMulAdd(const float a, const float b, const float c, float* result)
{
    if (result)
        *result = a * b + c;
}

void FusedMulAdd(const float* a, const float* b, const float* c, float* result, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        result[i] = a[i] * b[i] + c[i];
}

template <typename Func>
double Benchmark(Func f, int repeats = 3)
{
    double total_ms = 0;
    for (int i = 0; i < repeats; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        f();
        auto end = std::chrono::high_resolution_clock::now();
        total_ms += std::chrono::duration<double, std::milli>(end - start).count();
    }
    return total_ms / repeats;
}

int main(int argc, char** argv)
{
    size_t size = 10'000;
    int repeats = 10'000;

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--size" && i + 1 < argc)
            size = std::stoull(argv[++i]);
        else if (std::string(argv[i]) == "--repeats" && i + 1 < argc)
            repeats = std::stoi(argv[++i]);
    }

    std::vector<float> a(size), b(size), c(size), result(size);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(1.0f, 2.0f);
    for (size_t i = 0; i < size; ++i) {
        a[i] = dist(rng);
        b[i] = dist(rng);
        c[i] = dist(rng);
    }

    double mul_time = Benchmark([&] {
        Multiply(a.data(), b.data(), result.data(), size);
    }, repeats);

    double add_time = Benchmark([&] {
        Add(result.data(), c.data(), result.data(), size);
    }, repeats);

    double fused_time = Benchmark([&] {
        FusedMulAdd(a.data(), b.data(), c.data(), result.data(), size);
    }, repeats);

    double mul_time_single = 0.0;
    double add_time_single = 0.0;
    double fused_time_single = 0.0;

    for (size_t i = 0; i < size; ++i) {
        mul_time_single += Benchmark([&] {
            Multiply(a[i], b[i], &result[i]);
        }, repeats);

        add_time_single += Benchmark([&] {
            Add(result[i], c[i], &result[i]);
        }, repeats);

        fused_time_single += Benchmark([&] {
            FusedMulAdd(a[i], b[i], c[i], &result[i]);
        }, repeats);
    }

    double separate_total = mul_time + add_time;
    double separate_single_total = mul_time_single + add_time_single;

    double speedup_batch = separate_total / fused_time;
    double speedup_single = separate_single_total / fused_time_single;

std::cout << "\n== Benchmark Results ==\n\n";

std::cout << std::fixed << std::setprecision(8);

std::cout << std::left
          << "| " << std::setw(16) << "Operation"
          << "| " << std::setw(30) << "Batch Operations Time (ms)"
          << "| " << std::setw(34) << "Single Element Operations Time (ms)"
          << "|\n";

std::cout << "|-----------------|-------------------------------|------------------------------------|\n";

std::cout << "| " << std::setw(16) << "Multiply"
          << "| " << std::setw(30) << mul_time
          << "| " << std::setw(35) << mul_time_single << "|\n";

std::cout << "| " << std::setw(16) << "Add"
          << "| " << std::setw(30) << add_time
          << "| " << std::setw(35) << add_time_single << "|\n";

std::cout << "| " << std::setw(16) << "FusedMulAdd"
          << "| " << std::setw(30) << fused_time
          << "| " << std::setw(35) << fused_time_single << "|\n";

std::cout << "|-----------------|-------------------------------|------------------------------------|\n";

std::cout << "| " << std::setw(16) << "Separate Total"
          << "| " << std::setw(30) << separate_total
          << "| " << std::setw(35) << separate_single_total << "|\n";

std::cout << "| " << std::setw(16) << "Speedup (Fused)"
          << "| " << std::setw(30) << speedup_batch
          << "| " << std::setw(35) << speedup_single << "|\n";

    return 0;
}