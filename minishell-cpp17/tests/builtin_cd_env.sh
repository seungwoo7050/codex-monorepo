#!/usr/bin/env bash
# minishell-cpp17 v0.2.0 테스트: cd와 env 빌트인이 동작하는지 확인한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: builtin_cd_env.sh <minishell_binary>" >&2
  exit 1
fi

binary="$1"
tmp_dir=$(mktemp -d)
tmp_output=$(mktemp)
tmp_env=$(mktemp)
trap 'rm -rf "$tmp_dir" "$tmp_output" "$tmp_env"' EXIT

printf 'cd %s\n' "$tmp_dir" | "$binary" >"$tmp_output"

if ! grep -q "현재 디렉터리: $tmp_dir" "$tmp_output"; then
  echo "cd 빌트인이 현재 디렉터리를 출력하지 않았습니다." >&2
  cat "$tmp_output" >&2
  exit 1
fi

printf 'env\n' | "$binary" >"$tmp_env"

if ! grep -q "PATH=" "$tmp_env"; then
  echo "env 빌트인이 환경 변수를 출력하지 않았습니다." >&2
  cat "$tmp_env" >&2
  exit 1
fi

echo "minishell v0.2.0 cd/env 빌트인 테스트 통과"
