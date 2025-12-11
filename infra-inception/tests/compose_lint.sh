#!/usr/bin/env bash
# infra-inception v0.3.0 구성 검증: 한국어 웹 튜닝(TZ/charset/gzip) 반영 여부를 확인한다.
set -euo pipefail

if ! grep -q "app:" docker-compose.yml; then
  echo "app 서비스 정의가 없습니다." >&2
  exit 1
fi

if ! grep -q "db:" docker-compose.yml; then
  echo "db 서비스 정의가 없습니다." >&2
  exit 1
fi

if [ ! -f services/db/init.sql ]; then
  echo "DB 초기화 스크립트가 없습니다." >&2
  exit 1
fi

if ! grep -q "utf8mb4" services/db/init.sql; then
  echo "DB 초기화 스크립트에 UTF-8 설정이 없습니다." >&2
  exit 1
fi

if ! grep -q "Asia/Seoul" docker-compose.yml; then
  echo "모든 서비스에 TZ=Asia/Seoul 설정이 필요합니다." >&2
  exit 1
fi

if ! grep -q "redis:" docker-compose.yml; then
  echo "redis 서비스 정의가 없습니다." >&2
  exit 1
fi

if ! grep -q "nginx:" docker-compose.yml; then
  echo "nginx 서비스 정의가 없습니다." >&2
  exit 1
fi

if [ ! -f services/nginx/nginx.conf ]; then
  echo "nginx 설정 파일이 없습니다." >&2
  exit 1
fi

if [ ! -d services/nginx/static ]; then
  echo "nginx 정적 파일 디렉터리가 없습니다." >&2
  exit 1
fi

if ! grep -q "gzip on" services/nginx/nginx.conf; then
  echo "nginx gzip 설정이 없습니다." >&2
  exit 1
fi

if ! grep -q "log_format timed" services/nginx/nginx.conf; then
  echo "nginx 로그 포맷 튜닝이 없습니다." >&2
  exit 1
fi

if ! grep -q "prometheus:" docker-compose.yml; then
  echo "prometheus 서비스 정의가 없습니다." >&2
  exit 1
fi

if ! grep -q "grafana:" docker-compose.yml; then
  echo "grafana 서비스 정의가 없습니다." >&2
  exit 1
fi

if [ ! -f services/monitoring/prometheus.yml ]; then
  echo "Prometheus 설정 파일이 없습니다." >&2
  exit 1
fi

echo "infra-inception v0.4.0 구성 테스트 통과"
