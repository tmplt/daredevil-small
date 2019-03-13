#!/usr/bin/env bash
# Verifies binary stack usage using cargo call-stack <https://github.com/japaric/cargo-call-stack>
set -eou pipefail

# Get the memory usage from built binary
echo "Compiling in release to read memory usage from binary..."
size=$(cargo +nightly size --bin daredevil-small --release 2>&1)
[[ $size == *"error"* ]] && echo $size && exit 1
mem=$(echo $size | tail -n 1 | awk '{print $2+$3}')

# Ugly hack to enable inline-asm features for cargo-call-stack
sed -i 's/# features/features/g' Cargo.toml

echo "Compiling in nightly to generate new cg.dot file for stack usage."
cargo +nightly call-stack --bin daredevil-small > cg.dot

# Get RAM size
RAM=$(grep "RAM" memory.x | awk '{print $8}' | sed 's/K/000/g')

# Assumed interrupt stack usage
interrupts=1000

# DMA0 stack usage
dma=$(grep "DMA0" cg.dot | awk '{print $4}' | sed 's/\\nlocal//g')

# Stack usage in main
main=$(grep "main" cg.dot | awk '{print $4}' | sed 's/\\nlocal//g')

# Stack usage in hardfaults
hf=$(grep "HardFaultTrampoline" cg.dot | awk '{print $4}' | sed 's/\\nlocal//g')

total=$((dma + main + interrupts + hf + mem))
echo "Total memory usage: $total"

# if $total >= $RAM
if [[ $total -ge $RAM ]]; then
    echo "Total memory usage has exceeded RAM!"
    exit 1
else
    exit 0
fi
