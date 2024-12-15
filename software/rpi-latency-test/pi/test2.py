import time

# Direct measurement verification
start = time.perf_counter_ns()
time.sleep(1)
end = time.perf_counter_ns()

elapsed = end - start
print(f"Expected: 1,000,000,000 ns")
print(f"Measured: {elapsed} ns")
print(f"Ratio: {elapsed / 1_000_000_000}")
