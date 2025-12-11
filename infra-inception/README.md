# infra-inception v1.0.0

## 개요
Docker Compose 기반으로 app, db, redis, nginx, prometheus, grafana를 묶은 포트폴리오용 인프라 예제이다. v1.0.0에서는 이전 버전의 스택을 정리하고 기본 튜닝(Asia/Seoul 타임존, utf8mb4 DB 설정, Nginx gzip/로그 포맷)과 모니터링 구성을 완성했다.

## 서비스 구성
- **app**: 간단한 예제 애플리케이션 컨테이너, DB/Redis에 의존
- **db**: MariaDB, 초기화 스크립트로 스키마/계정 설정, `utf8mb4`/`utf8mb4_unicode_ci`
- **redis**: 캐시/세션 저장소
- **nginx**: 8080 포트로 app을 프록시하고 정적 파일(`services/nginx/static/`)을 제공, gzip/로그 포맷 튜닝 포함
- **prometheus**(9090), **grafana**(3000): 기본 모니터링 스택

## 실행
```bash
cd infra-inception
docker compose up -d
```

## 중지
```bash
cd infra-inception
docker compose down
```

## 설계 문서
- 최종 개요: `design/infra-inception/v1.0.0-overview.md`
- 버전별 상세 설계: `design/infra-inception/` 이하 파일 참조

## 운영 메모
- 모든 서비스는 `TZ=Asia/Seoul`을 사용하며, Nginx 설정에 gzip과 응답 시간 로그 포맷이 포함되어 있다.
- Prometheus 설정은 `services/monitoring/prometheus.yml`에서 스크랩 대상을 정의하며, Grafana는 기본 포트 3000으로 접근한다.
