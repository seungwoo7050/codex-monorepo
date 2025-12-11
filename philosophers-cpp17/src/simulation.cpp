#include "simulation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>

/**
 * [모듈] philosophers-cpp17/src/simulation.cpp
 * 설명:
 *   - 철학자 스레드와 모니터 스레드를 관리하며 교착 상태 데모와 회피 전략을 실행한다.
 *   - naive/ordered/waiter 전략을 선택적으로 수행하며 상태 로그, 식사 분포, 대기 시간 통계를 수집한다.
 * 버전: v0.3.0
 * 관련 설계문서:
 *   - design/philosophers-cpp17/v0.3.0-starvation-and-fairness.md
 * 변경 이력:
 *   - v0.1.0: 초기 교착 상태 데모 구현
 *   - v0.2.0: 전략 선택, 토큰 기반 웨이터, 요약 로그 추가
 *   - v0.3.0: 공정성 지표(식사 분포, 최대 대기 시간)와 지터 기반 시드 설정 추가
 * 테스트:
 *   - tests/deadlock_demo.sh
 *   - tests/ordered_strategy.sh
 *   - tests/waiter_strategy.sh
 *   - tests/fairness_metrics.sh
 */
DiningSimulation::DiningSimulation(const SimulationConfig& config)
    : config_(config),
      forks_(config.philosopher_count),
      meals_(config.philosopher_count),
      last_meal_ms_(config.philosopher_count),
      max_wait_ms_(config.philosopher_count),
      stop_requested_(false),
      deadlock_noted_(false),
      last_progress_ms_(0),
      ready_count_(0),
      waiter_permits_(config.philosopher_count > 1 ? config.philosopher_count - 1
                                                   : 0),
      rng_(config.random_seed),
      jitter_dist_(0, static_cast<std::uint32_t>(config.jitter_range.count())) {
  const std::int64_t now = nowMs();
  for (std::size_t i = 0; i < config_.philosopher_count; ++i) {
    meals_[i] = 0;
    last_meal_ms_[i] = now;
    max_wait_ms_[i] = 0;
  }
  last_progress_ms_ = now;
}

void DiningSimulation::logState(std::size_t id, const std::string& message) {
  std::lock_guard<std::mutex> lock(log_mutex_);
  std::cout << "[철학자 " << id << "] " << message << std::endl;
}

void DiningSimulation::logNotice(const std::string& message) {
  std::lock_guard<std::mutex> lock(log_mutex_);
  std::cout << "[안내] " << message << std::endl;
}

void DiningSimulation::logSummary() {
  std::lock_guard<std::mutex> lock(log_mutex_);
  std::cout << "[요약] 전략=" << strategyName()
            << " | 철학자별 식사/대기 기록" << std::endl;

  std::size_t total_meals = 0;
  std::size_t min_meals = meals_.empty() ? 0 : meals_[0].load();
  std::size_t max_meals = 0;
  double sum_square = 0.0;
  std::int64_t max_wait_overall = 0;

  for (std::size_t i = 0; i < meals_.size(); ++i) {
    const std::size_t meal_count = meals_[i].load();
    const std::int64_t max_wait = max_wait_ms_[i].load();
    total_meals += meal_count;
    min_meals = std::min(min_meals, meal_count);
    max_meals = std::max(max_meals, meal_count);
    sum_square += static_cast<double>(meal_count) * meal_count;
    if (max_wait > max_wait_overall) {
      max_wait_overall = max_wait;
    }
    std::cout << "  - 철학자 " << i << ": 식사 횟수=" << meal_count
              << ", 최대 대기=" << max_wait << "ms" << std::endl;
  }

  const double avg = meals_.empty()
                         ? 0.0
                         : static_cast<double>(total_meals) / meals_.size();
  double variance = meals_.empty()
                        ? 0.0
                        : (sum_square / meals_.size()) - (avg * avg);
  if (variance < 0.0) {
    variance = 0.0;
  }
  const double stddev = std::sqrt(variance);

  std::cout << "[요약] 식사 분포: 평균=" << avg << ", 최소=" << min_meals
            << ", 최대=" << max_meals << ", 표준편차=" << stddev
            << std::endl;
  std::cout << "[요약] 대기 지표: 최장 대기=" << max_wait_overall
            << "ms, 임계 대기 기준=" << config_.stuck_threshold.count() << "ms"
            << std::endl;
  if (min_meals == 0) {
    std::cout << "[주의] 일부 철학자가 한 번도 식사하지 못했습니다. 설정을 "
                 "조정하거나 전략을 바꾸어 공정성을 확인하세요."
              << std::endl;
  }
}

std::int64_t DiningSimulation::nowMs() const {
  const std::chrono::milliseconds ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now().time_since_epoch());
  return ms.count();
}

void DiningSimulation::updateProgress(std::size_t id) {
  const std::int64_t now = nowMs();
  meals_[id].fetch_add(1);
  last_meal_ms_[id].store(now);
  last_progress_ms_.store(now);
}

void DiningSimulation::recordWaiting(std::size_t id, std::int64_t wait_ms) {
  std::int64_t prev = max_wait_ms_[id].load();
  while (wait_ms > prev && !max_wait_ms_[id].compare_exchange_weak(prev, wait_ms)) {
  }
}

void DiningSimulation::waitForStart() {
  std::unique_lock<std::mutex> lock(start_mutex_);
  ++ready_count_;
  if (ready_count_ >= config_.philosopher_count) {
    start_cv_.notify_all();
  } else {
    start_cv_.wait(lock, [this]() {
      return ready_count_ >= config_.philosopher_count || stop_requested_.load();
    });
  }
}

std::chrono::milliseconds DiningSimulation::applyJitter(
    std::chrono::milliseconds base) {
  if (config_.jitter_range.count() == 0) {
    return base;
  }
  const std::uint32_t extra = sampleJitter();
  return base + std::chrono::milliseconds(extra);
}

std::uint32_t DiningSimulation::sampleJitter() {
  std::lock_guard<std::mutex> lock(rng_mutex_);
  return jitter_dist_(rng_);
}

void DiningSimulation::philosopherLoop(std::size_t id) {
  const std::size_t left = id;
  const std::size_t right = (id + 1) % config_.philosopher_count;

  waitForStart();

  if (config_.jitter_range.count() > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(sampleJitter()));
  }

  while (!stop_requested_.load()) {
    logState(id, "생각 중");
    std::this_thread::sleep_for(applyJitter(config_.think_time));

    const std::int64_t wait_start = nowMs();
    std::unique_lock<std::timed_mutex> first_lock;
    std::unique_lock<std::timed_mutex> second_lock;
    if (!acquireForks(id, first_lock, second_lock)) {
      recordWaiting(id, nowMs() - wait_start);
      continue;
    }

    const std::int64_t wait_spent = nowMs() - wait_start;
    recordWaiting(id, wait_spent);

    logState(id, "식사 시작");
    updateProgress(id);
    std::this_thread::sleep_for(applyJitter(config_.eat_time));
    logState(id, "식사 종료, 포크 반환");

    if (config_.strategy == StrategyType::kWaiter) {
      waiterLeave();
    }
  }
}

void DiningSimulation::monitorLoop() {
  const std::int64_t start_ms = nowMs();
  const std::int64_t runtime_ms = config_.runtime.count();
  while (!stop_requested_.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    const std::int64_t now = nowMs();
    const std::int64_t last = last_progress_ms_.load();
    if (!deadlock_noted_.load() &&
        (now - last) >= config_.stuck_threshold.count()) {
      deadlock_noted_ = true;
      logNotice("잠재적 교착 상태 감지: 일정 시간 동안 식사가 진행되지 않았습니다.");
    }
    if (runtime_ms > 0 && (now - start_ms) >= runtime_ms) {
      stop_requested_ = true;
      waiter_cv_.notify_all();
    }
  }
}

int DiningSimulation::run() {
  if (config_.philosopher_count < 2) {
    logNotice("철학자는 최소 2명 이상이어야 합니다.");
    return EXIT_FAILURE;
  }

  for (std::size_t i = 0; i < config_.philosopher_count; ++i) {
    threads_.push_back(std::thread(&DiningSimulation::philosopherLoop, this, i));
  }

  std::thread monitor(&DiningSimulation::monitorLoop, this);

  for (std::size_t i = 0; i < threads_.size(); ++i) {
    if (threads_[i].joinable()) {
      threads_[i].join();
    }
  }
  if (monitor.joinable()) {
    monitor.join();
  }

  if (deadlock_noted_.load()) {
    logNotice("교착 징후를 확인했으니 잠시 후 종료합니다.");
  }
  logSummary();
  logNotice("시뮬레이션 종료.");
  return EXIT_SUCCESS;
}

bool DiningSimulation::acquireForks(
    std::size_t id,
    std::unique_lock<std::timed_mutex>& first_lock,
    std::unique_lock<std::timed_mutex>& second_lock) {
  const std::size_t left = id;
  const std::size_t right = (id + 1) % config_.philosopher_count;

  if (config_.strategy == StrategyType::kNaive) {
    logState(id, "배고픔 → 왼쪽 포크 집기 시도");
    return acquireNaive(left, right, first_lock, second_lock);
  }

  if (config_.strategy == StrategyType::kOrdered) {
    logState(id, "낮은 번호 포크부터 확보 시도");
    return acquireOrdered(left, right, first_lock, second_lock);
  }

  logState(id, "웨이터 승인 요청 → 포크 확보 시도");
  return acquireWaiter(left, right, first_lock, second_lock);
}

bool DiningSimulation::acquireNaive(
    std::size_t left,
    std::size_t right,
    std::unique_lock<std::timed_mutex>& left_lock,
    std::unique_lock<std::timed_mutex>& right_lock) {
  left_lock = std::unique_lock<std::timed_mutex>(forks_[left]);
  logState(left, "왼쪽 포크 확보, 오른쪽 포크 대기 중");
  std::this_thread::sleep_for(config_.lock_timeout / 2);

  right_lock =
      std::unique_lock<std::timed_mutex>(forks_[right], std::defer_lock);
  if (!right_lock.try_lock_for(config_.lock_timeout)) {
    logState(left, "오른쪽 포크 대기 타임아웃 → 다시 시도 예정");
    left_lock.unlock();
    return false;
  }
  return true;
}

bool DiningSimulation::acquireOrdered(
    std::size_t left,
    std::size_t right,
    std::unique_lock<std::timed_mutex>& first_lock,
    std::unique_lock<std::timed_mutex>& second_lock) {
  const std::size_t first = std::min(left, right);
  const std::size_t second = std::max(left, right);
  first_lock = std::unique_lock<std::timed_mutex>(forks_[first]);
  second_lock = std::unique_lock<std::timed_mutex>(forks_[second],
                                                   std::defer_lock);
  if (!second_lock.try_lock_for(config_.lock_timeout)) {
    logNotice("순차 잠금 실패: 대기 시간 초과");
    first_lock.unlock();
    return false;
  }
  return true;
}

bool DiningSimulation::acquireWaiter(
    std::size_t left,
    std::size_t right,
    std::unique_lock<std::timed_mutex>& first_lock,
    std::unique_lock<std::timed_mutex>& second_lock) {
  if (!waiterEnter()) {
    return false;
  }

  if (!acquireOrdered(left, right, first_lock, second_lock)) {
    waiterLeave();
    return false;
  }
  return true;
}

bool DiningSimulation::waiterEnter() {
  std::unique_lock<std::mutex> lock(waiter_mutex_);
  waiter_cv_.wait(lock, [this]() {
    return stop_requested_.load() || waiter_permits_ > 0;
  });
  if (stop_requested_.load()) {
    return false;
  }
  --waiter_permits_;
  return true;
}

void DiningSimulation::waiterLeave() {
  {
    std::lock_guard<std::mutex> lock(waiter_mutex_);
    ++waiter_permits_;
  }
  waiter_cv_.notify_one();
}

std::string DiningSimulation::strategyName() const {
  switch (config_.strategy) {
    case StrategyType::kNaive:
      return "naive";
    case StrategyType::kOrdered:
      return "ordered";
    case StrategyType::kWaiter:
      return "waiter";
  }
  return "unknown";
}

/**
 * parseArguments
 * 설명:
 *   - CLI 인자를 단순히 파싱해 시뮬레이션 설정 구조체로 변환한다.
 *   - v0.3.0에서 지터 범위, 랜덤 시드를 추가로 받아 공정성 실험을 재현 가능하게 한다.
 * 입력:
 *   - argc/argv: 표준 main 인자 목록
 * 출력:
 *   - SimulationConfig: 기본값을 채운 뒤 인자로 덮어쓴 결과
 * 에러:
 *   - 잘못된 숫자 값이 들어오면 std::invalid_argument를 던진다.
 * 관련 설계문서:
 *   - design/philosophers-cpp17/v0.3.0-starvation-and-fairness.md
 * 관련 테스트:
 *   - tests/deadlock_demo.sh
 *   - tests/ordered_strategy.sh
 *   - tests/waiter_strategy.sh
 *   - tests/fairness_metrics.sh
 */
SimulationConfig parseArguments(int argc, char** argv) {
  SimulationConfig config;
  config.philosopher_count = 5;
  config.think_time = std::chrono::milliseconds(200);
  config.eat_time = std::chrono::milliseconds(300);
  config.lock_timeout = std::chrono::milliseconds(800);
  config.stuck_threshold = std::chrono::milliseconds(700);
  config.runtime = std::chrono::milliseconds(3000);
  config.strategy = StrategyType::kNaive;
  config.jitter_range = std::chrono::milliseconds(0);
  config.random_seed = static_cast<unsigned int>(
      std::chrono::steady_clock::now().time_since_epoch().count());

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    auto parseMs = [](const std::string& value) {
      return std::chrono::milliseconds(std::stoll(value));
    };
    if (arg == "--philosophers" && i + 1 < argc) {
      config.philosopher_count = static_cast<std::size_t>(std::stoul(argv[++i]));
    } else if (arg == "--think-ms" && i + 1 < argc) {
      config.think_time = parseMs(argv[++i]);
    } else if (arg == "--eat-ms" && i + 1 < argc) {
      config.eat_time = parseMs(argv[++i]);
    } else if (arg == "--lock-timeout-ms" && i + 1 < argc) {
      config.lock_timeout = parseMs(argv[++i]);
    } else if (arg == "--stuck-threshold-ms" && i + 1 < argc) {
      config.stuck_threshold = parseMs(argv[++i]);
    } else if (arg == "--duration-ms" && i + 1 < argc) {
      config.runtime = parseMs(argv[++i]);
    } else if (arg == "--jitter-ms" && i + 1 < argc) {
      config.jitter_range = parseMs(argv[++i]);
      if (config.jitter_range.count() < 0) {
        throw std::invalid_argument("--jitter-ms 값은 음수일 수 없습니다.");
      }
    } else if (arg == "--random-seed" && i + 1 < argc) {
      config.random_seed = static_cast<unsigned int>(std::stoul(argv[++i]));
    } else if (arg == "--strategy" && i + 1 < argc) {
      std::string strategy(argv[++i]);
      if (strategy == "naive") {
        config.strategy = StrategyType::kNaive;
      } else if (strategy == "ordered") {
        config.strategy = StrategyType::kOrdered;
      } else if (strategy == "waiter") {
        config.strategy = StrategyType::kWaiter;
      } else {
        throw std::invalid_argument("지원하지 않는 전략입니다: " + strategy);
      }
    } else {
      throw std::invalid_argument("알 수 없는 인자이거나 값이 누락되었습니다: " + arg);
    }
  }

  return config;
}
