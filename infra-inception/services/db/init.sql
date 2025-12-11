-- 기본 데모 데이터베이스와 테이블을 초기화한다. UTF-8 설정을 강제하여 한국어 데이터를 안전하게 저장한다.
SET time_zone = 'Asia/Seoul';
ALTER DATABASE demo_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS health_check (
  id INT AUTO_INCREMENT PRIMARY KEY,
  note VARCHAR(255) NOT NULL
) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

INSERT INTO health_check (note) VALUES ('infra-inception v0.3.0 ready');
