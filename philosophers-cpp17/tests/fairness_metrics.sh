#!/usr/bin/env bash
set -euo pipefail

# 공정성/기아 지표가 요약 로그에 포함되는지 확인하는 스크립트 (v0.3.0)
BIN_PATH="$1"
OUTPUT=$("${BIN_PATH}" \
  --strategy ordered \
  --duration-ms 1200 \
  --think-ms 30 \
  --eat-ms 30 \
  --lock-timeout-ms 300 \
  --stuck-threshold-ms 600 \
  --jitter-ms 10 \
  --random-seed 42)

echo "${OUTPUT}"

MEAL_LINES=$(grep -c "식사 횟수=" <<< "${OUTPUT}")
if [ "${MEAL_LINES}" -lt 5 ]; then
  echo "모든 철학자의 식사 기록이 포함되어야 한다" >&2
  exit 1
fi

grep -q "식사 분포" <<< "${OUTPUT}"
grep -q "대기 지표" <<< "${OUTPUT}"
