#!/usr/bin/env bash
# minishell-cpp17 v0.3.0 테스트: 파이프라인 실행과 종료 코드 전달을 검증한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: pipeline_basic.sh <minishell_binary>" >&2
  exit 1
fi

binary="$1"
portable_tmp=$(mktemp)
trap 'rm -f "$portable_tmp"' EXIT

echo "echo one two | wc -w" | "$binary" >"$portable_tmp"

if ! grep -q "2" "$portable_tmp"; then
  echo "파이프라인 결과가 예상과 다릅니다." >&2
  cat "$portable_tmp" >&2
  exit 1
fi

if ! grep -q "exit status: 0" "$portable_tmp"; then
  echo "파이프라인 종료 코드가 0이 아닙니다." >&2
  cat "$portable_tmp" >&2
  exit 1
fi

echo "minishell v0.3.0 파이프라인 테스트 통과"
