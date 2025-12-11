#!/usr/bin/env bash
set -euo pipefail

# 웨이터(waiter) 전략이 토큰 기반 접근 제어로 교착을 방지하는지 확인하는 스크립트 (v0.2.0)
BIN_PATH="$1"
OUTPUT=$("${BIN_PATH}" \
  --strategy waiter \
  --duration-ms 1500 \
  --think-ms 40 \
  --eat-ms 50 \
  --lock-timeout-ms 200 \
  --stuck-threshold-ms 500)

echo "${OUTPUT}"

grep -q "전략=waiter" <<< "${OUTPUT}"
if grep -q "교착" <<< "${OUTPUT}"; then
  echo "waiter 전략에서 교착 메시지가 발생하면 안 된다" >&2
  exit 1
fi

MEAL_LINES=$(grep -c "식사 횟수=" <<< "${OUTPUT}")
if [ "${MEAL_LINES}" -lt 5 ]; then
  echo "모든 철학자의 식사 횟수 요약이 출력되어야 한다" >&2
  exit 1
fi
