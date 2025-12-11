# 베이스 이미지는 가벼운 파이썬 런타임을 사용한다.
FROM python:3.11-slim

# 애플리케이션 코드 추가
WORKDIR /app
COPY main.py /app/main.py

# 단순 HTTP 서버를 실행한다.
CMD ["python", "main.py"]
