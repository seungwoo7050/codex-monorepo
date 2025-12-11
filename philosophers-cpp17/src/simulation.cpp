#include "simulation.hpp"

#include <cstdlib>
#include <iostream>

/**
 * [모듈] philosophers-cpp17/src/simulation.cpp
 * 설명:
 *   - 철학자 스레드와 모니터 스레드를 관리하며 기본적인 교착 상태 데모를 실행한다.
 *   - 포크를 좌측→우측 순서로 집는 단순한 전략을 사용해 교착 징후를 노출한다.
 * 버전: v0.1.0
 * 관련 설계문서:
 *   - design/philosophers-cpp17/v0.1.0-naive-deadlock.md
 * 변경 이력:
 *   - v0.1.0: 초기 교착 상태 데모 구현
 * 테스트:
 *   - tests/deadlock_demo.sh
 */
DiningSimulation::DiningSimulation(const SimulationConfig& config)
    : config_(config),
      forks_(config.philosopher_count),
      meals_(config.philosopher_count),
      last_meal_ms_(config.philosopher_count),
      stop_requested_(false),
      deadlock_noted_(false),
      last_progress_ms_(0),
      ready_count_(0) {
  const std::int64_t now = nowMs();
  for (std::size_t i = 0; i < config_.philosopher_count; ++i) {
    meals_[i] = 0;
    last_meal_ms_[i] = now;
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

void DiningSimulation::philosopherLoop(std::size_t id) {
  const std::size_t left = id;
  const std::size_t right = (id + 1) % config_.philosopher_count;

  waitForStart();

  while (!stop_requested_.load()) {
    logState(id, "생각 중");
    std::this_thread::sleep_for(config_.think_time);

    logState(id, "배고픔 → 왼쪽 포크 집기 시도");
    std::unique_lock<std::timed_mutex> left_lock(forks_[left]);
    logState(id, "왼쪽 포크 확보, 오른쪽 포크 대기 중");
    std::this_thread::sleep_for(config_.lock_timeout / 2);

    std::unique_lock<std::timed_mutex> right_lock(forks_[right],
                                                  std::defer_lock);
    if (!right_lock.try_lock_for(config_.lock_timeout)) {
      logState(id, "오른쪽 포크 대기 타임아웃 → 다시 시도 예정");
      left_lock.unlock();
      continue;
    }

    logState(id, "식사 시작");
    updateProgress(id);
    std::this_thread::sleep_for(config_.eat_time);
    logState(id, "식사 종료, 포크 반환");
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
  logNotice("시뮬레이션 종료.");
  return EXIT_SUCCESS;
}

/**
 * parseArguments
 * 설명:
 *   - CLI 인자를 단순히 파싱해 시뮬레이션 설정 구조체로 변환한다.
 * 입력:
 *   - argc/argv: 표준 main 인자 목록
 * 출력:
 *   - SimulationConfig: 기본값을 채운 뒤 인자로 덮어쓴 결과
 * 에러:
 *   - 잘못된 숫자 값이 들어오면 std::invalid_argument를 던진다.
 * 관련 설계문서:
 *   - design/philosophers-cpp17/v0.1.0-naive-deadlock.md
 * 관련 테스트:
 *   - tests/deadlock_demo.sh
 */
SimulationConfig parseArguments(int argc, char** argv) {
  SimulationConfig config;
  config.philosopher_count = 5;
  config.think_time = std::chrono::milliseconds(200);
  config.eat_time = std::chrono::milliseconds(300);
  config.lock_timeout = std::chrono::milliseconds(800);
  config.stuck_threshold = std::chrono::milliseconds(700);
  config.runtime = std::chrono::milliseconds(3000);

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
    } else {
      throw std::invalid_argument("알 수 없는 인자이거나 값이 누락되었습니다: " + arg);
    }
  }

  return config;
}
