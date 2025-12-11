#!/usr/bin/env bash
# minishell-cpp17 v0.2.0 테스트: 환경 변수 확장이 동작하는지 확인한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: env_expansion.sh <minishell_binary>" >&2
  exit 1
fi

binary="$1"
tmp_output=$(mktemp)
cmd_file=$(mktemp)
trap 'rm -f "$tmp_output" "$cmd_file"' EXIT

echo 'echo $FOO' >"$cmd_file"

env -i FOO="expanded" HOME="/tmp" PATH="/usr/bin:/bin" \
  "$binary" <"$cmd_file" >"$tmp_output"

if ! grep -q "expanded" "$tmp_output"; then
  echo "환경 변수 확장 결과가 없습니다." >&2
  cat "$tmp_output" >&2
  exit 1
fi

echo "minishell v0.2.0 환경 변수 확장 테스트 통과"
