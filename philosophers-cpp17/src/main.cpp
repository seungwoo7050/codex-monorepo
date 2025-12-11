#include <exception>
#include <iostream>

#include "simulation.hpp"

/**
 * [모듈] philosophers-cpp17/src/main.cpp
 * 설명:
 *   - v0.1.0 기준으로 교착 상태 데모용 시뮬레이션을 실행한다.
 *   - CLI 인자를 받아 기본 설정을 조정하고, 실행 결과를 표준 출력에 남긴다.
 * 버전: v0.1.0
 * 관련 설계문서:
 *   - design/philosophers-cpp17/v0.1.0-naive-deadlock.md
 * 변경 이력:
 *   - v0.1.0: 초기 메인 엔트리 추가
 * 테스트:
 *   - tests/deadlock_demo.sh
 */
int main(int argc, char** argv) {
  try {
    SimulationConfig config = parseArguments(argc, argv);
    DiningSimulation simulation(config);
    return simulation.run();
  } catch (const std::exception& ex) {
    std::cerr << "[오류] 설정 파싱 중 예외 발생: " << ex.what() << std::endl;
    return 1;
  }
}
