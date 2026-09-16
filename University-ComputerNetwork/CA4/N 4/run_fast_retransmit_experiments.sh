#!/usr/bin/env bash
set -euo pipefail

RESULTS_DIR="${1:-fast_retransmit_results}"
INPUT_FILE="${2:-input.txt}"
BASE_PORT="${BASE_PORT:-18180}"
PACKET_SIZE="${PACKET_SIZE:-1024}"
TIMEOUT_MS="${TIMEOUT_MS:-1000}"
DROP_SEQUENCE="${DROP_SEQUENCE:-20}"
DELAY_MS="${DELAY_MS:-10}"

if [[ ! -f "$INPUT_FILE" ]]; then
    printf 'Input file not found: %s\n' "$INPUT_FILE" >&2
    exit 1
fi

mkdir -p "$RESULTS_DIR"
printf '%s\n' "mode,file_bytes,original_packets,total_packets_sent,retransmitted_packets,retransmission_events,timeouts,fast_retransmits,fast_retransmit_enabled,total_time_ms,average_throughput_kbps,final_cwnd_packets,final_ssthresh_packets,transfer_complete" > "$RESULTS_DIR/comparison.csv"

cleanup_receiver() {
    if [[ -n "${receiver_pid:-}" ]]; then
        kill "$receiver_pid" 2>/dev/null || true
        wait "$receiver_pid" 2>/dev/null || true
        receiver_pid=""
    fi
}
trap cleanup_receiver EXIT

run_mode() {
    local mode="$1"
    local port="$2"
    local sender_option="$3"
    local mode_dir="$RESULTS_DIR/$mode"
    local output_file="$mode_dir/output.txt"

    mkdir -p "$mode_dir"
    printf 'Running %s...\n' "$mode"
    ./receiver "$port" "$output_file" --loss 0 --delay "$DELAY_MS" \
        --packet-size "$PACKET_SIZE" --seed 4242 --drop-seq-once "$DROP_SEQUENCE" \
        > "$mode_dir/receiver.log" 2>&1 &
    receiver_pid=$!
    sleep 0.3

    if ! timeout 180 ./sender 127.0.0.1 "$port" "$INPUT_FILE" \
        --packet-size "$PACKET_SIZE" --timeout "$TIMEOUT_MS" $sender_option \
        > "$mode_dir/sender.log" 2>&1; then
        cleanup_receiver
        printf 'Sender failed or timed out in mode %s\n' "$mode" >&2
        return 1
    fi

    sleep 0.2
    cleanup_receiver
    mv cwnd.csv throughput.csv events.log receiver_events.log "$mode_dir/"
    python3 plot_metrics.py --cwnd "$mode_dir/cwnd.csv" \
        --throughput "$mode_dir/throughput.csv" --output-dir "$mode_dir" \
        --title-prefix "$mode"

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
    total_packets_sent="$(awk -F': ' '/Total Packets Sent:/ {print $2}' "$mode_dir/sender.log")"
    retransmitted_packets="$(awk -F': ' '/Retransmitted Packets:/ {print $2}' "$mode_dir/sender.log")"
    retransmission_events="$(awk -F': ' '/Total Retransmission Events:/ {print $2}' "$mode_dir/sender.log")"
    timeouts="$(awk -F': ' '/Total Timeouts:/ {print $2}' "$mode_dir/sender.log")"
    fast_retransmits="$(awk -F': ' '/Fast Retransmits:/ {print $2}' "$mode_dir/sender.log")"
    if [[ "$sender_option" == "--no-fast-retransmit" ]]; then
        fast_retransmit_enabled="no"
    else
        fast_retransmit_enabled="yes"
    fi
    total_time_ms="$(awk -F': ' '/Total Transfer Time:/ {print $2}' "$mode_dir/sender.log" | awk '{print $1}')"
    average_throughput_kbps="$(awk -F': ' '/Average Throughput:/ {print $2}' "$mode_dir/sender.log" | awk '{print $1}')"
    final_cwnd_packets="$(awk -F': ' '/Final cwnd:/ {print $2}' "$mode_dir/sender.log" | awk '{print $1}')"
    final_ssthresh_packets="$(awk -F': ' '/Final ssthresh:/ {print $2}' "$mode_dir/sender.log" | awk '{print $1}')"
    printf '%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n' \
        "$mode" "$file_bytes" "$original_packets" "$total_packets_sent" \
        "$retransmitted_packets" "$retransmission_events" "$timeouts" \
        "$fast_retransmits" "$fast_retransmit_enabled" "$total_time_ms" \
        "$average_throughput_kbps" "$final_cwnd_packets" "$final_ssthresh_packets" \
        "$transfer_complete" >> "$RESULTS_DIR/comparison.csv"
    [[ "$transfer_complete" == "yes" ]]
}

run_mode "without_fast_retransmit" "$BASE_PORT" "--no-fast-retransmit"
run_mode "with_fast_retransmit" "$((BASE_PORT + 1))" ""

python3 - "$RESULTS_DIR/comparison.csv" "$RESULTS_DIR/report.txt" <<'PY'
import csv
import sys

comparison_file, report_file = sys.argv[1:3]
with open(comparison_file, newline="", encoding="utf-8") as csv_file:
    rows = {row["mode"]: row for row in csv.DictReader(csv_file)}
without_fast = rows["without_fast_retransmit"]
with_fast = rows["with_fast_retransmit"]
without_time = float(without_fast["total_time_ms"])
with_time = float(with_fast["total_time_ms"])
improvement = ((without_time - with_time) / without_time * 100.0) if without_time else 0.0
report = (
    "Fast Retransmit comparison\n"
    "==========================\n"
    f"Without Fast Retransmit: time={without_time:.3f} ms, timeouts={without_fast['timeouts']}, fast_retransmits={without_fast['fast_retransmits']}\n"
    f"With Fast Retransmit:    time={with_time:.3f} ms, timeouts={with_fast['timeouts']}, fast_retransmits={with_fast['fast_retransmits']}\n"
    f"Transfer-time improvement: {improvement:.2f}%\n"
)
with open(report_file, "w", encoding="utf-8") as output_file:
    output_file.write(report)
print(report, end="")
PY
printf 'Comparison written to %s\n' "$RESULTS_DIR"
