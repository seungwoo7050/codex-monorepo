#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

/**
 * [모듈] philosophers-cpp17/include/simulation.hpp
 * 설명:
 *   - 교착 상태 시뮬레이션을 위한 설정과 실행 클래스 선언부를 제공한다.
 *   - v1.0.0에서 설정 파싱, 실행 제어, 보고 기능을 명확히 분리해 포트폴리오 버전의 구조를 정리한다.
 * 버전: v1.0.0
 * 관련 설계문서:
 *   - design/philosophers-cpp17/v1.0.0-overview.md
 * 변경 이력:
 *   - v0.1.0: 기본 설정 구조체와 시뮬레이션 클래스 선언 추가
 *   - v0.2.0: 데드락 회피 전략 선택 옵션 및 통계 요약 추가
 *   - v0.3.0: 최대 대기 시간, 식사 분포 통계, 랜덤 지터 설정 추가
 *   - v1.0.0: 설정 검증과 결과 보고 구조를 추가해 구성 요소 역할을 명확화
 * 테스트:
 *   - tests/deadlock_demo.sh
 *   - tests/ordered_strategy.sh
 *   - tests/waiter_strategy.sh
 *   - tests/fairness_metrics.sh
 *   - tests/usage_help.sh
*/
enum class StrategyType {
  kNaive,
  kOrdered,
  kWaiter,
};

struct SimulationConfig {
  std::size_t philosopher_count;
  std::chrono::milliseconds think_time;
  std::chrono::milliseconds eat_time;
  std::chrono::milliseconds lock_timeout;
  std::chrono::milliseconds stuck_threshold;
  std::chrono::milliseconds runtime;
  StrategyType strategy;
  std::chrono::milliseconds jitter_range;
  unsigned int random_seed;
};

/**
 * SimulationReport (v1.0.0)
 * 역할:
 *   - 실행이 끝난 뒤 철학자별 식사 횟수, 대기 시간, 분포 지표를 담아 리포트한다.
 * 설계:
 *   - design/philosophers-cpp17/v1.0.0-overview.md
 */
struct SimulationReport {
  std::size_t total_meals;
  std::size_t min_meals;
  std::size_t max_meals;
  double average_meals;
  double stddev_meals;
  std::int64_t max_wait_overall;
  std::vector<std::size_t> meals;
  std::vector<std::int64_t> max_waits;
};

struct ParseResult {
  SimulationConfig config;
  bool show_help;
};

/**
 * DiningSimulation (v1.0.0)
 * 역할:
 *   - 철학자 스레드 생성, 상태 모니터링, 종료 제어를 총괄한다.
 *   - 전략 선택에 따라 순차 잠금(ordered)과 웨이터 기반 접근 제어(waiter)를 통해 교착을 회피한다.
 *   - 실행 통계(SimulationReport)를 생성해 보고 단계와 핵심 실행 로직을 분리한다.
 * 설계:
 *   - design/philosophers-cpp17/v1.0.0-overview.md
 * 주의 사항:
 *   - stop_requested_가 설정되어도 try_lock_for 대기 시간만큼 지연될 수 있다.
 *   - waiter 전략은 kPhilosopherCount-1 토큰 정책으로 진입을 제한하므로 종료 시에는 웨이크업을 위해 알림이 필요하다.
 *   - 대기 시간 통계는 포크 확보 시도마다 측정하며, 실패/성공 여부와 관계없이 최장 대기 시간을 갱신한다.
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
  SimulationReport summarize() const;
  void logSummary(const SimulationReport& report);
  std::int64_t nowMs() const;
  void updateProgress(std::size_t id);
  void recordWaiting(std::size_t id, std::int64_t wait_ms);
  void waitForStart();
  std::chrono::milliseconds applyJitter(std::chrono::milliseconds base);
  std::uint32_t sampleJitter();
  bool acquireForks(std::size_t id,
                    std::unique_lock<std::timed_mutex>& first_lock,
                    std::unique_lock<std::timed_mutex>& second_lock);
  bool acquireNaive(std::size_t left,
                    std::size_t right,
                    std::unique_lock<std::timed_mutex>& left_lock,
                    std::unique_lock<std::timed_mutex>& right_lock);
  bool acquireOrdered(std::size_t left,
                      std::size_t right,
                      std::unique_lock<std::timed_mutex>& first_lock,
                      std::unique_lock<std::timed_mutex>& second_lock);
  bool acquireWaiter(std::size_t left,
                     std::size_t right,
                     std::unique_lock<std::timed_mutex>& first_lock,
                     std::unique_lock<std::timed_mutex>& second_lock);
  bool waiterEnter();
  void waiterLeave();
  std::string strategyName() const;

  SimulationConfig config_;
  std::vector<std::timed_mutex> forks_;
  std::vector<std::thread> threads_;
  std::vector<std::atomic<std::size_t> > meals_;
  std::vector<std::atomic<std::int64_t> > last_meal_ms_;
  std::vector<std::atomic<std::int64_t> > max_wait_ms_;
  std::atomic<bool> stop_requested_;
  std::atomic<bool> deadlock_noted_;
  std::atomic<std::int64_t> last_progress_ms_;
  std::mutex log_mutex_;
  std::mutex start_mutex_;
  std::condition_variable start_cv_;
  std::size_t ready_count_;
  std::mutex waiter_mutex_;
  std::condition_variable waiter_cv_;
  std::size_t waiter_permits_;
  std::mt19937 rng_;
  std::uniform_int_distribution<std::uint32_t> jitter_dist_;
  std::mutex rng_mutex_;
};

ParseResult parseArguments(int argc, char** argv);
bool validateConfig(const SimulationConfig& config, std::string& error_out);
void printUsage();
