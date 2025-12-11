"""
단순 데모용 HTTP 서버.
- DB 연결 정보는 환경 변수로만 노출한다.
- GET 요청마다 기본 헬스 체크 메시지를 반환한다.
"""
import http.server
import os
import socketserver

PORT = 8085

class HealthHandler(http.server.SimpleHTTPRequestHandler):
    """헬스 체크와 DB 설정 노출 여부를 확인하는 핸들러."""

    def do_GET(self):
        message = (
            "infra-inception v0.1.0 app running\n"
            f"DB_HOST={os.environ.get('DB_HOST', 'unset')}\n"
        )
        body = message.encode("utf-8")
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
