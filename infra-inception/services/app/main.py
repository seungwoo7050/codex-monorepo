"""
단순 데모용 HTTP 서버.
- DB, Redis 연결 설정을 환경 변수로 받아 상태를 노출한다.
- GET 요청마다 현재 설정과 버전 정보를 반환한다.
버전: v0.2.0
설계: design/infra-inception/v0.2.0-nginx-redis-stack.md
"""
import http.server
import os
import socketserver
import redis

PORT = 8085


def build_body():
    """환경 변수와 Redis 상태를 포함한 응답 본문을 생성한다."""
    db_host = os.environ.get("DB_HOST", "unset")
    redis_host = os.environ.get("REDIS_HOST", "unset")
    redis_port = int(os.environ.get("REDIS_PORT", "6379"))

    redis_status = "unknown"
    try:
        client = redis.Redis(host=redis_host, port=redis_port, socket_connect_timeout=0.5)
        client.ping()
        redis_status = "pong"
    except Exception as exc:  # noqa: BLE001 - 테스트용 간단 처리
        redis_status = f"error:{exc.__class__.__name__}"

    message = (
        "infra-inception v0.2.0 app running\n"
        f"DB_HOST={db_host}\n"
        f"REDIS={redis_host}:{redis_port} status={redis_status}\n"
    )
    return message.encode("utf-8")


class HealthHandler(http.server.SimpleHTTPRequestHandler):
    """헬스 체크와 백엔드 상태를 반환하는 핸들러."""

    def do_GET(self):
        body = build_body()
        self.send_response(200)
        self.send_header("Content-Type", "text/plain; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, format, *args):
        # 기본 로그는 stdout으로 간단히 남긴다.
        return super().log_message(format, *args)


if __name__ == "__main__":
    with socketserver.TCPServer(("0.0.0.0", PORT), HealthHandler) as httpd:
        httpd.serve_forever()
