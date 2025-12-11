#!/usr/bin/env bash
# webserv-cpp17 v0.1.0 테스트: 단일 요청 처리 후 종료 여부를 검증한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: test_webserv.sh <webserv_binary>" >&2
  exit 1
fi

binary="$1"
port=9090

"$binary" "$port" &
server_pid=$!

cleanup() {
  if kill -0 "$server_pid" 2>/dev/null; then
    kill "$server_pid"
    wait "$server_pid" || true
  fi
}
trap cleanup EXIT

sleep 0.2

response=$(curl -s --max-time 5 "http://127.0.0.1:${port}")

if ! grep -q "Hello from webserv v0.1.0" <<<"$response"; then
  echo "응답 본문이 예상과 다릅니다." >&2
  echo "$response" >&2
  exit 1
fi

# 서버가 단일 요청 후 종료되는지 확인
sleep 0.2
if kill -0 "$server_pid" 2>/dev/null; then
  echo "서버가 단일 요청 후 종료되지 않았습니다." >&2
  exit 1
fi

echo "webserv v0.1.0 단일 요청 테스트 통과"
