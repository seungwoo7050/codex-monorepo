-- 기본 데모 데이터베이스와 테이블을 초기화한다.
CREATE TABLE IF NOT EXISTS health_check (
  id INT AUTO_INCREMENT PRIMARY KEY,
  note VARCHAR(255) NOT NULL
);

INSERT INTO health_check (note) VALUES ('infra-inception v0.1.0 ready');
