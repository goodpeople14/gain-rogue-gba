# Gain Rogue GBA

Gain Ground 스타일의 전투에 로그라이크 요소를 결합한 GBA 게임 프로젝트입니다.

## 프로젝트 목표

작은 전장에서 적을 제거하고, 전투가 끝날 때마다 무작위 강화를 선택해 다음 전투로 이어지는 짧고 반복 가능한 액션 게임을 만드는 것이 목표입니다.

첫 번째 목표는 복잡한 게임을 한 번에 만드는 것이 아니라, GBA 환경에서 실제로 실행되는 작은 플레이 가능한 버전을 완성하는 것입니다.

## 현재 구현 상태

- GBA 개발 환경 구성 및 ROM 빌드
- mGBA 실행 확인
- 타이틀 화면에 `GAIN ROGUE`와 `PRESS START` 표시
- START 입력 시 빈 게임 화면으로 전환
- 타이틀 화면의 문자 스프라이트 제거 확인

## 핵심 게임 흐름

1. 캐릭터 1명을 선택합니다.
2. 무작위로 배치된 적과 전투합니다.
3. 모든 적을 제거하면 전투가 종료됩니다.
4. 강화 3개 중 1개를 선택합니다.
5. 다음 전장으로 이동합니다.
6. 3개 전장을 클리어하거나 플레이어가 사망하면 한 번의 플레이가 종료됩니다.
7. 사망 시 해당 플레이의 강화와 진행 상태는 초기화됩니다.

## 첫 번째 완성 범위

- 플레이어 캐릭터 1명
- 이동
- 기본 공격
- 적 캐릭터
- 적 위치 무작위 배치
- 적 전멸 시 스테이지 클리어
- 전투 후 강화 3개 중 1개 선택
- 스테이지마다 적 수 증가
- 총 3개 스테이지
- 플레이어 사망 및 재시작

## 초기 강화 요소

- 공격력 증가
- 이동속도 증가
- 투사체 속도 증가
- 최대 체력 증가
- 공격 재사용 시간 감소

## 제외 범위

첫 번째 버전에서는 아래 기능을 구현하지 않습니다.

- 복잡한 랜덤 맵 생성
- 다수의 플레이어 캐릭터
- 장비 및 인벤토리
- 영구 성장
- 세이브 데이터
- 상점
- 스토리
- 보스전

## 개발 순서

- [x] GBA 개발 환경 구성
- [x] Butano 기본 예제 빌드
- [x] mGBA에서 예제 실행
- [x] 프로젝트 기본 구조 생성
- [x] 타이틀 화면과 START 입력 구현
- [x] 빌드·실행 절차 문서화
- [ ] 플레이어 이동 구현
- [ ] 공격 및 적 제거 구현
- [ ] 한 스테이지 클리어 구현
- [ ] 적 위치 무작위 배치
- [ ] 보상 선택 화면 구현
- [ ] 강화 효과 적용
- [ ] 3개 스테이지 반복
- [ ] 사망 및 재시작 구현

## 개발 환경

| 구분 | 사용 도구 |
|---|---|
| Language | C++ |
| Engine | Butano |
| Toolchain | devkitARM |
| Build | GNU Make |
| Emulator | mGBA |
| Target | Game Boy Advance |

필요한 도구가 정상 설치되었는지는 다음 명령으로 확인합니다.

```bash
arm-none-eabi-g++ --version
make --version
python --version
```

## 디렉터리 배치

현재 Makefile은 프로젝트와 Butano 저장소가 같은 상위 디렉터리에 있다고 가정합니다.

```text
C:\gba-dev\
├─ butano\
│  └─ butano\
└─ gain-rogue-gba\
   ├─ Makefile
   ├─ README.md
   ├─ include\
   └─ src\
```

Makefile의 Butano 경로는 다음과 같습니다.

```makefile
LIBBUTANO := ../butano/butano
```

다른 위치에 Butano를 설치했다면 실제 위치에 맞게 `LIBBUTANO`를 수정해야 합니다.

## 빌드 방법

프로젝트 루트에서 다음 명령을 실행합니다.

```bash
cd C:/gba-dev/gain-rogue-gba
make -j2
```

정상적으로 완료되면 프로젝트 루트에 다음 ROM 파일이 생성됩니다.

```text
gain-rogue-gba.gba
```

이전 산출물을 제거하고 처음부터 다시 확인하려면 clean build를 실행합니다.

```bash
make clean
make -j2
```

`build/`, `.gba`, `.elf` 파일은 빌드 산출물이므로 Git에 커밋하지 않습니다.

## mGBA 실행 및 확인

1. mGBA를 실행합니다.
2. `File → Load ROM`을 선택합니다.
3. `C:\gba-dev\gain-rogue-gba\gain-rogue-gba.gba`를 엽니다.
4. 타이틀과 입력 동작을 확인합니다.

현재 확인할 수 있는 동작은 다음과 같습니다.

- 실행 직후 `GAIN ROGUE`와 `PRESS START` 표시
- START 입력 전까지 타이틀 화면 유지
- START를 누르면 녹색의 빈 게임 화면으로 전환
- 전환 후 타이틀 문자가 남지 않음

코드를 수정한 뒤에는 ROM을 다시 빌드하고 mGBA에서 ROM을 다시 여는 방식으로 확인합니다.

## 반복 개발 절차

```text
소스 수정
→ make -j2
→ mGBA에서 ROM 실행
→ 화면과 입력 확인
→ git diff 확인
→ 변경 파일만 커밋
```

빌드 환경까지 다시 검증해야 할 때만 `make clean`을 먼저 실행합니다.

## 문제 해결

### mGBA 실행 중 make clean이 실패하는 경우

Windows에서는 실행 중인 mGBA가 ROM 파일을 사용하고 있어 삭제가 실패할 수 있습니다.

1. mGBA를 종료합니다.
2. 작업 관리자에서 mGBA 프로세스가 남아 있지 않은지 확인합니다.
3. 다시 clean build를 실행합니다.

```bash
make clean
make -j2
```

### VS Code에서 Butano 헤더를 찾지 못하는 경우

`make`는 성공하지만 VS Code에 `bn_core.h` 같은 헤더 오류가 표시된다면 실제 컴파일 오류가 아니라 IntelliSense 경로 문제일 가능성이 큽니다.

VS Code의 C/C++ Include path에 다음 경로를 추가합니다.

```text
C:/gba-dev/butano/butano/include
C:/gba-dev/butano/butano/hw/include
C:/gba-dev/butano/butano/hw/3rd_party/libtonc/include
```

그다음 아래 명령을 순서대로 실행합니다.

```text
C/C++: Reset IntelliSense Database
Developer: Reload Window
```

프로젝트를 다른 위치에 설치했다면 위 경로도 실제 Butano 위치에 맞게 변경합니다.

### 빌드는 성공했지만 변경 내용이 보이지 않는 경우

- mGBA에서 이전 ROM을 계속 실행 중인지 확인합니다.
- 빌드한 ROM 경로와 mGBA에서 연 ROM 경로가 같은지 확인합니다.
- 필요하면 mGBA를 종료한 뒤 clean build하고 ROM을 다시 엽니다.

## 1차 완료 조건

아래 흐름을 실제로 플레이할 수 있으면 첫 번째 목표를 완료한 것으로 봅니다.

```text
캐릭터 선택
→ 전투
→ 적 전멸
→ 강화 선택
→ 다음 전투
→ 3개 스테이지 클리어 또는 사망
→ 재시작
```
