#!/usr/bin/env bash
# minishell-cpp17 v0.2.0 테스트: exit 빌트인이 지정한 종료 코드를 반환하는지 확인한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: builtin_exit_status.sh <minishell_binary>" >&2
  exit 1
fi

binary="$1"

set +e
printf 'exit 7\n' | "$binary" >/dev/null 2>&1
status=$?
set -e

if [ "$status" -ne 7 ]; then
  echo "exit 빌트인이 종료 코드를 전달하지 않았습니다. ($status)" >&2
  exit 1
fi

echo "minishell v0.2.0 exit 빌트인 테스트 통과"
