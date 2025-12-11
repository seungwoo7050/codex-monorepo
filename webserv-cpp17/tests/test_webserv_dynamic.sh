#!/usr/bin/env bash
# webserv-cpp17 v0.4.0 테스트: 동적 핸들러(/health, /metrics) 응답을 검증한다.
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "사용법: test_webserv_dynamic.sh <webserv_binary>" >&2
  exit 1
fi

binary="$1"
port=9093
max_requests=4

"$binary" "$port" "$max_requests" &
server_pid=$!

cleanup() {
  if kill -0 "$server_pid" 2>/dev/null; then
    kill "$server_pid"
    wait "$server_pid" || true
  fi
}
trap cleanup EXIT

sleep 0.3

health=$(curl -s -w "%{http_code}" --max-time 5 "http://127.0.0.1:${port}/health")
health_body=${health::-3}
health_code=${health: -3}

if [ "$health_code" != "200" ] || ! grep -q "status: ok" <<<"$health_body"; then
  echo "health 응답이 예상과 다릅니다." >&2
  exit 1
fi

metrics=$(curl -s -w "%{http_code}" --max-time 5 "http://127.0.0.1:${port}/metrics")
metrics_body=${metrics::-3}
metrics_code=${metrics: -3}

if [ "$metrics_code" != "200" ] || ! grep -q "requests_total" <<<"$metrics_body"; then
  echo "metrics 응답이 예상과 다릅니다." >&2
  exit 1
fi

echo "webserv v0.4.0 동적 핸들러 테스트 통과"
