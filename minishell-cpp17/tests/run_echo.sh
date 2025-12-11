#!/usr/bin/env bash
# minishell-cpp17 v0.2.0 테스트: echo 명령이 실행되고 종료 코드가 표시되는지 확인한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: run_echo.sh <minishell_binary>" >&2
  exit 1
fi

binary="$1"

tmp_output=$(mktemp)
trap 'rm -f "$tmp_output"' EXIT

echo "echo test-message" | "$binary" >"$tmp_output"

if ! grep -q "test-message" "$tmp_output"; then
  echo "출력에 echo 결과가 없습니다." >&2
  cat "$tmp_output" >&2
  exit 1
fi

if ! grep -q "exit status: 0" "$tmp_output"; then
  echo "종료 코드가 0으로 표시되지 않았습니다." >&2
  cat "$tmp_output" >&2
  exit 1
fi

echo "minishell v0.2.0 echo 테스트 통과"
