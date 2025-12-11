#!/usr/bin/env bash
set -euo pipefail

# 교착 상태 재현을 위한 간단한 실행 스크립트 (v0.2.0)
# - 기본 설정(naive)으로 실행 후 출력에 교착 징후 안내 문구가 포함되는지 확인한다.

BIN_PATH="$1"
OUTPUT=$("${BIN_PATH}" --duration-ms 2200 --lock-timeout-ms 1000 --stuck-threshold-ms 900)

echo "${OUTPUT}"

grep -q "잠재적 교착 상태 감지" <<< "${OUTPUT}"
grep -q "전략=naive" <<< "${OUTPUT}"
grep -q "시뮬레이션 종료" <<< "${OUTPUT}"
