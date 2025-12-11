#include <exception>
#include <iostream>

#include "simulation.hpp"

/**
 * [모듈] philosophers-cpp17/src/main.cpp
 * 설명:
 *   - v1.0.0 기준으로 설정 파싱, 실행 제어, 결과 보고 단계를 분리해 포트폴리오용 CLI를 제공한다.
 *   - CLI 인자를 받아 기본 설정을 조정하고, 실행 결과(요약 통계 포함)를 표준 출력에 남긴다.
 * 버전: v1.0.0
 * 관련 설계문서:
 *   - design/philosophers-cpp17/v1.0.0-overview.md
 * 변경 이력:
 *   - v0.1.0: 초기 메인 엔트리 추가
 *   - v0.2.0: 전략 선택 옵션 추가 및 주석 업데이트
 *   - v0.3.0: 공정성 통계와 시드 기반 지터 옵션을 반영
 *   - v1.0.0: 설정 검증 및 도움말 출력을 추가해 사용자 흐름을 단순화
 * 테스트:
 *   - tests/deadlock_demo.sh
 *   - tests/ordered_strategy.sh
 *   - tests/waiter_strategy.sh
 *   - tests/fairness_metrics.sh
 *   - tests/usage_help.sh
 */
int main(int argc, char** argv) {
  try {
    const ParseResult parsed = parseArguments(argc, argv);
    if (parsed.show_help) {
      printUsage();
      return 0;
    }

    std::string error_message;
    if (!validateConfig(parsed.config, error_message)) {
      std::cerr << "[오류] 설정 검증 실패: " << error_message << std::endl;
      return 1;
    }

    DiningSimulation simulation(parsed.config);
    return simulation.run();
  } catch (const std::exception& ex) {
    std::cerr << "[오류] 설정 파싱 중 예외 발생: " << ex.what() << std::endl;
    return 1;
  }
}
