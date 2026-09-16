#!/usr/bin/env bash

set -euo pipefail

RESULTS_DIR="${1:-phase5_results}"
INPUT_FILE="${2:-input.txt}"
BASE_PORT="${BASE_PORT:-18080}"
PACKET_SIZE="${PACKET_SIZE:-1024}"
TIMEOUT_MS="${TIMEOUT_MS:-1000}"

if [[ ! -f "$INPUT_FILE" ]]; then
    printf 'Input file not found: %s\n' "$INPUT_FILE" >&2
    exit 1
fi

mkdir -p "$RESULTS_DIR"
printf '%s\n' \
    "scenario,loss_percent,delay_ms,file_bytes,original_packets,total_packets_sent,retransmitted_packets,retransmission_events,timeouts,fast_retransmits,fast_retransmit_enabled,total_time_ms,average_throughput_kbps,final_cwnd_packets,final_ssthresh_packets,transfer_complete" \
    > "$RESULTS_DIR/results.csv"

cleanup_receiver() {
    if [[ -n "${receiver_pid:-}" ]]; then
        kill "$receiver_pid" 2>/dev/null || true
        wait "$receiver_pid" 2>/dev/null || true
        receiver_pid=""
    fi
}

trap cleanup_receiver EXIT

run_scenario() {
    local scenario="$1"
    local loss="$2"
    local delay="$3"
    local seed="$4"
    local port="$5"
    local scenario_dir="$RESULTS_DIR/$scenario"
    local output_file="$scenario_dir/output.txt"

    mkdir -p "$scenario_dir"
    printf 'Running %s (loss=%s, delay=%s ms, seed=%s)\n' "$scenario" "$loss" "$delay" "$seed"

    ./receiver "$port" "$output_file" --loss "$loss" --delay "$delay" \
        --packet-size "$PACKET_SIZE" --seed "$seed" \
        > "$scenario_dir/receiver.log" 2>&1 &
    receiver_pid=$!
    sleep 0.3

    if ! timeout 180 ./sender 127.0.0.1 "$port" "$INPUT_FILE" \
        --packet-size "$PACKET_SIZE" --timeout "$TIMEOUT_MS" \
        > "$scenario_dir/sender.log" 2>&1; then
        cleanup_receiver
        printf 'Sender failed or timed out in scenario %s\n' "$scenario" >&2
        return 1
    fi

    sleep 0.3
    cleanup_receiver

    mv cwnd.csv throughput.csv events.log receiver_events.log "$scenario_dir/"
    python3 plot_metrics.py \
        --cwnd "$scenario_dir/cwnd.csv" \
        --throughput "$scenario_dir/throughput.csv" \
        --output-dir "$scenario_dir" \
        --title-prefix "$scenario"

    local transfer_complete="no"
    if cmp -s "$INPUT_FILE" "$output_file"; then
        transfer_complete="yes"
    fi

    local file_bytes
    local original_packets
    local total_packets_sent
    local retransmitted_packets
    local retransmission_events
    local timeouts
    local fast_retransmits
    local fast_retransmit_enabled
    local total_time_ms
    local average_throughput_kbps
    local final_cwnd_packets
    local final_ssthresh_packets
    file_bytes="$(wc -c < "$INPUT_FILE")"
    original_packets="$(( (file_bytes + PACKET_SIZE - 1) / PACKET_SIZE ))"
    total_packets_sent="$(awk -F': ' '/Total Packets Sent:/ {print $2}' "$scenario_dir/sender.log")"
    retransmitted_packets="$(awk -F': ' '/Retransmitted Packets:/ {print $2}' "$scenario_dir/sender.log")"
    retransmission_events="$(awk -F': ' '/Total Retransmission Events:/ {print $2}' "$scenario_dir/sender.log")"
    timeouts="$(awk -F': ' '/Total Timeouts:/ {print $2}' "$scenario_dir/sender.log")"
    fast_retransmits="$(awk -F': ' '/Fast Retransmits:/ {print $2}' "$scenario_dir/sender.log")"
    fast_retransmit_enabled="yes"
    total_time_ms="$(awk -F': ' '/Total Transfer Time:/ {print $2}' "$scenario_dir/sender.log" | awk '{print $1}')"
    average_throughput_kbps="$(awk -F': ' '/Average Throughput:/ {print $2}' "$scenario_dir/sender.log" | awk '{print $1}')"
    final_cwnd_packets="$(awk -F': ' '/Final cwnd:/ {print $2}' "$scenario_dir/sender.log" | awk '{print $1}')"
    final_ssthresh_packets="$(awk -F': ' '/Final ssthresh:/ {print $2}' "$scenario_dir/sender.log" | awk '{print $1}')"
    printf '%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n' \
        "$scenario" "$(awk -v loss="$loss" 'BEGIN { printf "%.0f", loss * 100 }')" "$delay" \
        "$file_bytes" "$original_packets" "$total_packets_sent" "$retransmitted_packets" \
        "$retransmission_events" "$timeouts" "$fast_retransmits" "$fast_retransmit_enabled" \
        "$total_time_ms" "$average_throughput_kbps" \
        "$final_cwnd_packets" "$final_ssthresh_packets" "$transfer_complete" \
        >> "$RESULTS_DIR/results.csv"

    if [[ "$transfer_complete" != "yes" ]]; then
        printf 'File verification failed in scenario %s\n' "$scenario" >&2
        return 1
    fi
}

run_scenario "no_loss" "0.00" "0" "1001" "$BASE_PORT"
run_scenario "medium_loss" "0.05" "50" "2002" "$((BASE_PORT + 1))"
run_scenario "high_loss" "0.20" "100" "3003" "$((BASE_PORT + 2))"

printf 'All experiments completed. Results: %s/results.csv\n' "$RESULTS_DIR"
