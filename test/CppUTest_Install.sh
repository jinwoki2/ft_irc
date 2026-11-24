#!/usr/bin/env bash
set -euo pipefail

# 설치 대상 경로 (실행 시 인자를 넘기지 않으면 기본 경로 사용)
TARGET_DIR="${1:-$HOME/goinfre/cpputest}"
REPO_URL="https://github.com/cpputest/cpputest.git"

echo "=================================================="
echo " CppUTest 자동 설치 스크립트"
echo " 대상 경로: ${TARGET_DIR}"
echo "=================================================="

# 1. 기존 디렉터리 존재 여부 확인
if [ -d "${TARGET_DIR}" ]; then
    echo "오류: 대상 디렉터리가 이미 존재합니다: ${TARGET_DIR}"
    echo "기존 디렉터리를 삭제하거나 다른 경로를 지정해 주세요."
    exit 1
fi

# 2. Git clone (빠른 빌드를 위해 depth 1 적용)
echo "[1/4] CppUTest 저장소 복제 중..."
git clone "${REPO_URL}" "${TARGET_DIR}"

# 3. CMake 설정 및 빌드
echo "[2/4] CMake 빌드 디렉터리 구성 및 컴파일..."
mkdir -p "${TARGET_DIR}/build"
cd "${TARGET_DIR}/build"
cmake ..
make

# 4. 대상 경로를 prefix로 지정하여 include/lib 설치
echo "[3/4] 헤더 및 라이브러리 설치 (--install --prefix)..."
cmake --install . --prefix "${TARGET_DIR}"

# 5. 설치 결과 검증
echo "[4/4] 설치 검증 진행 중..."
HEADER_CHECK="${TARGET_DIR}/include/CppUTest/TestHarness.h"
LIB_CHECK="${TARGET_DIR}/lib/libCppUTest.a"

if [ -f "${HEADER_CHECK}" ] && [ -f "${LIB_CHECK}" ]; then
    echo ""
    echo "=================================================="
    echo " CppUTest 설치 완료!"
    echo "=================================================="
    echo "Makefile에서 참조할 수 있도록 쉘 설정 파일(~/.bashrc 또는 ~/.zshrc)에"
    echo "아래 환경 변수를 등록하세요:"
    echo ""
    echo "export CPPUTEST_HOME=\"${TARGET_DIR}\""
    echo "=================================================="
else
    echo "오류: 설치 파일 검증 실패 (라이브러리 또는 헤더가 누락되었습니다)."
    exit 1
fi