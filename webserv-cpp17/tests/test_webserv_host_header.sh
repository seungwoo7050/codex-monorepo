#!/usr/bin/env bash
# webserv-cpp17 v0.4.0 테스트: Host 헤더가 없을 때 400을 반환하는지 검증한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: test_webserv_host_header.sh <webserv_binary>" >&2
  exit 1
fi

binary="$1"
port=9092
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
req = b"GET / HTTP/1.1\r\n\r\n"
s.sendall(req)
data = s.recv(4096)
s.close()
if b"400 Bad Request" not in data or b"Missing Host" not in data:
    print("Host 헤더 누락 응답이 예상과 다릅니다.", file=sys.stderr)
    sys.exit(1)
PY

response=$(curl -s --max-time 5 "http://127.0.0.1:${port}")
if ! grep -q "Hello from webserv v0.4.0" <<<"$response"; then
  echo "정상 요청 응답 본문이 예상과 다릅니다." >&2
  echo "$response" >&2
  exit 1
fi

echo "webserv v0.4.0 Host 헤더 검증 테스트 통과"
