#!/usr/bin/env bash
# infra-inception v0.1.0 구성 검증: 필수 서비스와 초기화 스크립트를 확인한다.
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

echo "infra-inception v0.1.0 구성 테스트 통과"
