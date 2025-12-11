#!/usr/bin/env bash
# infra-inception v0.2.0 구성 검증: 필수 서비스와 역방향 프록시/캐시 구성을 확인한다.
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

echo "infra-inception v0.2.0 구성 테스트 통과"
