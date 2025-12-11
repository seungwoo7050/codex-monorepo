#!/usr/bin/env bash
# webserv-cpp17 v0.3.0 테스트: 동일 연결에서 두 개의 요청을 keep-alive로 처리하는지 확인한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: test_webserv_keepalive.sh <webserv_binary>" >&2
  exit 1
fi

binary="$1"
port=9093
max_requests=2

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

python - <<PY
import socket
import sys
s = socket.create_connection(("127.0.0.1", ${port}))
req = (
    b"GET / HTTP/1.1\r\nHost: keep.test\r\nConnection: keep-alive\r\n\r\n"
    b"GET / HTTP/1.1\r\nHost: keep.test\r\nConnection: close\r\n\r\n"
)
s.sendall(req)
received = b""
while True:
    chunk = s.recv(4096)
    if not chunk:
        break
    received += chunk
s.close()
if received.count(b"Hello from webserv v0.3.0") != 2:
    print("keep-alive 응답 횟수가 2가 아닙니다.", file=sys.stderr)
    sys.exit(1)
if b"Connection: keep-alive" not in received:
    print("첫 번째 응답에 keep-alive 헤더가 없습니다.", file=sys.stderr)
    sys.exit(1)
PY

echo "webserv v0.3.0 keep-alive 테스트 통과"
