#!/usr/bin/env bash
# minishell-cpp17 v0.3.0 테스트: 입력/출력 리다이렉션 및 append 동작을 검증한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: redirection_basic.sh <minishell_binary>" >&2
  exit 1
fi

binary="$1"
out_file=$(mktemp)
trap 'rm -f "$out_file"' EXIT

echo "echo first > $out_file" | "$binary" >/dev/null
if ! grep -q "first" "$out_file"; then
  echo "> 리다이렉션 출력이 기록되지 않았습니다." >&2
  exit 1
fi

echo "echo second >> $out_file" | "$binary" >/dev/null
count=$(grep -c "second" "$out_file" || true)
if [ "$count" -ne 1 ]; then
  echo ">> 리다이렉션으로 두 번째 줄이 append되지 않았습니다." >&2
  cat "$out_file" >&2
  exit 1
fi

echo "cat < $out_file" | "$binary" >"${out_file}.read"
if ! grep -q "first" "${out_file}.read" || ! grep -q "second" "${out_file}.read"; then
  echo "< 리다이렉션으로 파일을 읽지 못했습니다." >&2
  cat "${out_file}.read" >&2 || true
  exit 1
fi

if ! grep -q "exit status: 0" "${out_file}.read"; then
  echo "리다이렉션 명령 종료 코드가 0이 아닙니다." >&2
  cat "${out_file}.read" >&2 || true
  exit 1
fi

echo "minishell v0.3.0 리다이렉션 테스트 통과"
