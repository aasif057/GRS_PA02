#!/bin/bash

# Define programs to test
programs=("cpu_bound" "memory_bound" "io_bound" "mixed_load")

# Define thread counts to test
threads=(1 2 4 8 16 50 100)

# Define perf metrics
metrics="cache-references,cache-misses,LLC-load-misses,LLC-store-misses,cpu-clock,context-switches,block:block_rq_complete,sched:sched_stat_sleep"

# Create a log file
log_file="perf_results.log"
echo "Performance Test Results" > "$log_file"
echo "------------------------" >> "$log_file"

# Run tests
for prog in "${programs[@]}"; do
    echo "Testing $prog" | tee -a "$log_file"
    for t in "${threads[@]}"; do
        echo "Threads: $t" | tee -a "$log_file"

        # Run perf stat (No -M)
        { time sudo perf stat -e "$metrics" ./"$prog" "$t"; } 2>> "$log_file"

        # # Run perf record (for later analysis)
        # sudo perf record -o "perf_${prog}_${t}.data" -e "$metrics" ./"$prog" "$t"
        # echo "Perf record saved: perf_${prog}_${t}.data" | tee -a "$log_file"
        echo "" >> "$log_file"
    done
    echo "---------------------------------" >> "$log_file"
done

echo "Tests completed. Results saved in $log_file"
echo "Use 'sudo perf report -i perf_<program>_<threads>.data"
