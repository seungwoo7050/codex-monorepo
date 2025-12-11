#!/usr/bin/env bash
# webserv-cpp17 v0.3.0 테스트: 다중 요청 처리와 타임아웃 동작을 검증한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: test_webserv_multi.sh <webserv_binary>" >&2
  exit 1
fi

binary="$1"
port=9091
max_requests=3

"$binary" "$port" "$max_requests" &
server_pid=$!

cleanup() {
  if kill -0 "$server_pid" 2>/dev/null; then
    kill "$server_pid"
    wait "$server_pid" || true
  fi
}
trap cleanup EXIT

sleep 0.2

resp1=$(mktemp)
resp2=$(mktemp)
trap 'cleanup; rm -f "$resp1" "$resp2"' EXIT

curl -s --max-time 5 "http://127.0.0.1:${port}" >"$resp1" &
curl -s --max-time 5 "http://127.0.0.1:${port}" >"$resp2" &

python - <<PY
import socket
import time
import sys

s = socket.create_connection(("127.0.0.1", ${port}))
time.sleep(2)
data = s.recv(1)
if data != b"":
    print("타임아웃으로 연결이 닫히지 않았습니다.", file=sys.stderr)
    sys.exit(1)
PY

wait

if ! grep -q "Hello from webserv v0.3.0" "$resp1"; then
  echo "첫 번째 응답 본문이 예상과 다릅니다." >&2
  cat "$resp1" >&2
  exit 1
fi

if ! grep -q "Hello from webserv v0.3.0" "$resp2"; then
  echo "두 번째 응답 본문이 예상과 다릅니다." >&2
  cat "$resp2" >&2
  exit 1
fi

sleep 0.5
if kill -0 "$server_pid" 2>/dev/null; then
  echo "다중 연결 처리 후 서버가 종료되지 않았습니다." >&2
  exit 1
fi

echo "webserv v0.3.0 다중 연결/타임아웃 테스트 통과"
