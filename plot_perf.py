# import re
# import matplotlib.pyplot as plt
# import numpy as np

# # Define log file
# log_file = "perf_results_old.log"

# # Metrics to extract
# metrics = ["cpu-clock", "cache-misses", "context-switches", "block:block_rq_complete"]
# programs = ["cpu_bound", "memory_bound", "io_bound", "mixed_load"]
# fixed_thread_count = 4  # Change this if needed

# # Initialize data storage
# data = {prog: {metric: 0 for metric in metrics} for prog in programs}

# # Read log file
# with open(log_file, "r") as f:
#     lines = f.readlines()

# current_prog = None
# record_data = False

# for line in lines:
#     # Check if the line indicates a new workload
#     if "Testing" in line:
#         current_prog = line.strip().split()[-1]  # Extract program name
#     # Check if the line indicates the thread count
#     elif "Threads:" in line:
#         thread_count = int(line.strip().split(":")[1])  # Extract thread count
#         record_data = (thread_count == fixed_thread_count)  # Only record if thread count matches
#     # If we are in the correct workload and thread count, extract metrics
#     elif record_data and current_prog in programs:
#         for metric in metrics:
#             # Handle cpu-clock separately (it has "msec" in the line)
#             if metric == "cpu-clock":
#                 match = re.search(r"([\d,\.]+)\s+msec\s+" + metric, line)
#             else:
#                 match = re.search(r"([\d,]+)\s+" + metric, line)
#             if match:
#                 value = float(match.group(1).replace(",", ""))  # Convert to float after removing commas
#                 data[current_prog][metric] = value

# # Debug print extracted values
# print("Extracted Data at Threads =", fixed_thread_count)
# for prog in programs:
#     print(f"\n{prog}:")
#     for metric in metrics:
#         print(f"  {metric}: {data[prog][metric]}")

# # Convert data to lists for plotting
# labels = programs
# cpu_values = [data[prog]["cpu-clock"] for prog in programs]
# cache_miss_values = [data[prog]["cache-misses"] for prog in programs]
# context_switch_values = [data[prog]["context-switches"] for prog in programs]
# io_values = [data[prog]["block:block_rq_complete"] for prog in programs]

# # Set up bar width
# x = np.arange(len(labels))

# # Plot CPU Usage
# plt.figure(figsize=(8, 5))
# plt.bar(x, cpu_values, color=["red", "blue", "green", "purple"], alpha=0.7)
# plt.xticks(x, labels)
# plt.ylabel("CPU Clock (msec)")
# plt.title(f"CPU Usage for Different Workloads (Threads = {fixed_thread_count})")
# plt.savefig("cpu_usage_comparison.png")
# plt.show()

# # Plot Cache Misses
# plt.figure(figsize=(8, 5))
# plt.bar(x, cache_miss_values, color=["red", "blue", "green", "purple"], alpha=0.7)
# plt.xticks(x, labels)
# plt.ylabel("Cache Misses")
# plt.title(f"Cache Misses for Different Workloads (Threads = {fixed_thread_count})")
# plt.savefig("cache_misses_comparison.png")
# plt.show()

# # Plot Context Switches
# plt.figure(figsize=(8, 5))
# plt.bar(x, context_switch_values, color=["red", "blue", "green", "purple"], alpha=0.7)
# plt.xticks(x, labels)
# plt.ylabel("Context Switches")
# plt.title(f"Thread Switching for Different Workloads (Threads = {fixed_thread_count})")
# plt.savefig("context_switches_comparison.png")
# plt.show()

# # Plot I/O Requests
# plt.figure(figsize=(8, 5))
# plt.bar(x, io_values, color=["red", "blue", "green", "purple"], alpha=0.7)
# plt.xticks(x, labels)
# plt.ylabel("I/O Requests Completed")
# plt.title(f"I/O Activity for Different Workloads (Threads = {fixed_thread_count})")
# plt.savefig("io_comparison.png")
# plt.show()

import re
import matplotlib.pyplot as plt
import numpy as np

# Define log file
log_file = "perf_results.log"

# Metrics to extract
metrics = ["cpu-clock", "cache-misses", "context-switches", "block:block_rq_complete"]
programs = ["cpu_bound", "memory_bound", "io_bound", "mixed_load"]
thread_counts = [1, 2, 4, 8, 16, 50, 100]  # All thread counts in the log file

# Initialize data storage
data = {prog: {metric: [] for metric in metrics} for prog in programs}
execution_times = {prog: [] for prog in programs}  # To store execution times

# Read log file
with open(log_file, "r") as f:
    lines = f.readlines()

current_prog = None
current_threads = None

for line in lines:
    # Check if the line indicates a new workload
    if "Testing" in line:
        current_prog = line.strip().split()[-1]  # Extract program name
    # Check if the line indicates the thread count
    elif "Threads:" in line:
        current_threads = int(line.strip().split(":")[1])  # Extract thread count
    # If we are in a workload section, extract metrics
    elif current_prog in programs and current_threads in thread_counts:
        for metric in metrics:
            # Handle cpu-clock separately (it has "msec" in the line)
            if metric == "cpu-clock":
                match = re.search(r"([\d,\.]+)\s+msec\s+" + metric, line)
            else:
                match = re.search(r"([\d,]+)\s+" + metric, line)
            if match:
                value = float(match.group(1).replace(",", ""))  # Convert to float after removing commas
                data[current_prog][metric].append(value)
        # Extract execution time
        if "seconds time elapsed" in line:
            match = re.search(r"([\d,\.]+)\s+seconds time elapsed", line)
            if match:
                execution_time = float(match.group(1).replace(",", ""))
                execution_times[current_prog].append(execution_time)

# Debug print extracted values
print("Extracted Data:")
for prog in programs:
    print(f"\n{prog}:")
    for metric in metrics:
        print(f"  {metric}: {data[prog][metric]}")
    print(f"  Execution Times: {execution_times[prog]}")

# Plot CPU Clock vs Threads
plt.figure(figsize=(10, 6))
for prog in programs:
    plt.plot(thread_counts, data[prog]["cpu-clock"], label=prog, marker='o')
plt.xlabel("Threads")
plt.ylabel("CPU Clock (msec)")
plt.title("CPU Clock vs Threads for Different Workloads")
plt.legend()
plt.grid(True)
plt.savefig("cpu_clock_vs_threads.png")
plt.show()

# Plot Cache Misses vs Threads
plt.figure(figsize=(10, 6))
for prog in programs:
    plt.plot(thread_counts, data[prog]["cache-misses"], label=prog, marker='o')
plt.xlabel("Threads")
plt.ylabel("Cache Misses")
plt.title("Cache Misses vs Threads for Different Workloads")
plt.legend()
plt.grid(True)
plt.savefig("cache_misses_vs_threads.png")
plt.show()

# Plot Context Switches vs Threads
plt.figure(figsize=(10, 6))
for prog in programs:
    plt.plot(thread_counts, data[prog]["context-switches"], label=prog, marker='o')
plt.xlabel("Threads")
plt.ylabel("Context Switches")
plt.title("Context Switches vs Threads for Different Workloads")
plt.legend()
plt.grid(True)
plt.savefig("context_switches_vs_threads.png")
plt.show()

# Plot I/O Requests vs Threads
plt.figure(figsize=(10, 6))
for prog in programs:
    plt.plot(thread_counts, data[prog]["block:block_rq_complete"], label=prog, marker='o')
plt.xlabel("Threads")
plt.ylabel("I/O Requests Completed")
plt.title("I/O Requests vs Threads for Different Workloads")
plt.legend()
plt.grid(True)
plt.savefig("io_requests_vs_threads.png")
plt.show()

# Plot Execution Time vs Threads
plt.figure(figsize=(10, 6))
for prog in programs:
    plt.plot(thread_counts, execution_times[prog], label=prog, marker='o')
plt.xlabel("Threads")
plt.ylabel("Execution Time (seconds)")
plt.title("Execution Time vs Threads for Different Workloads")
plt.legend()
plt.grid(True)
plt.savefig("execution_time_vs_threads.png")
plt.show()