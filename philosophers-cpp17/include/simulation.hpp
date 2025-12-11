#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

/**
 * [모듈] philosophers-cpp17/include/simulation.hpp
 * 설명:
 *   - 교착 상태 시뮬레이션을 위한 설정과 실행 클래스 선언부를 제공한다.
 *   - v0.1.0에서 사용할 CLI 파싱 인터페이스와 스레드 루프 진입점을 정의한다.
 * 버전: v0.1.0
 * 관련 설계문서:
 *   - design/philosophers-cpp17/v0.1.0-naive-deadlock.md
 * 변경 이력:
 *   - v0.1.0: 기본 설정 구조체와 시뮬레이션 클래스 선언 추가
 * 테스트:
 *   - tests/deadlock_demo.sh
 */
struct SimulationConfig {
  std::size_t philosopher_count;
  std::chrono::milliseconds think_time;
  std::chrono::milliseconds eat_time;
  std::chrono::milliseconds lock_timeout;
  std::chrono::milliseconds stuck_threshold;
  std::chrono::milliseconds runtime;
};

/**
 * DiningSimulation (v0.1.0)
 * 역할:
 *   - 철학자 스레드 생성, 상태 모니터링, 종료 제어를 총괄한다.
 *   - 좌측→우측 포크 순서를 강제해 교착 상태가 발생하기 쉬운 상황을 만든다.
 * 설계:
 *   - design/philosophers-cpp17/v0.1.0-naive-deadlock.md
 * 주의 사항:
 *   - stop_requested_가 설정되어도 try_lock_for 대기 시간만큼 지연될 수 있다.
 */
class DiningSimulation {
 public:
  explicit DiningSimulation(const SimulationConfig& config);
  int run();

 private:
  void philosopherLoop(std::size_t id);
  void monitorLoop();
  void logState(std::size_t id, const std::string& message);
  void logNotice(const std::string& message);
  std::int64_t nowMs() const;
  void updateProgress(std::size_t id);
  void waitForStart();

  SimulationConfig config_;
  std::vector<std::timed_mutex> forks_;
  std::vector<std::thread> threads_;
  std::vector<std::atomic<std::size_t> > meals_;
  std::vector<std::atomic<std::int64_t> > last_meal_ms_;
  std::atomic<bool> stop_requested_;
  std::atomic<bool> deadlock_noted_;
  std::atomic<std::int64_t> last_progress_ms_;
  std::mutex log_mutex_;
  std::mutex start_mutex_;
  std::condition_variable start_cv_;
  std::size_t ready_count_;
};

SimulationConfig parseArguments(int argc, char** argv);
