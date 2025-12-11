#!/usr/bin/env bash
set -euo pipefail

# 도움말 옵션을 통해 포트폴리오용 CLI 요약이 출력되는지 확인한다. (v1.0.0)
BIN_PATH="$1"
OUTPUT=$("${BIN_PATH}" --help)

echo "${OUTPUT}"

grep -q "사용법: philosophers" <<< "${OUTPUT}"
grep -q -- "--strategy" <<< "${OUTPUT}"
