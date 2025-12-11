# 베이스 이미지는 가벼운 파이썬 런타임을 사용한다.
FROM python:3.11-slim

# Redis 클라이언트를 포함해 캐시 계층 검증이 가능하도록 한다.
RUN pip install --no-cache-dir redis==5.0.1

# 애플리케이션 코드 추가
WORKDIR /app
COPY main.py /app/main.py

# 단순 HTTP 서버를 실행한다.
CMD ["python", "main.py"]
