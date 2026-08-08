# Gain Rogue GBA

Gain Ground식 한 화면 전투에 로그라이크 강화와 보스 공략을 결합한 GBA 액션 게임 프로젝트입니다.

## 프로젝트 목표

두 캐릭터 중 하나를 선택해 적과 거리를 조절하며 싸우고, 위험한 출구를 돌파한 뒤 무작위 강화를 골라 보스까지 공략하는 10분 내외의 고전 판타지 로그라이크 액션 게임을 만드는 것이 목표입니다.

처음부터 모든 기능을 구현하지 않고, GBA에서 실제로 실행되는 작은 전투 단위부터 완성하고 검증하면서 확장합니다.

상세 기획과 범위의 기준은 [GitHub Issue #11](https://github.com/goodpeople14/gain-rogue-gba/issues/11)입니다. README와 다른 문서가 충돌하면 #11을 우선합니다.

## 현재 구현 상태

현재 `main`에는 다음 기반이 구현되어 있습니다.

- Butano 기반 GBA 개발 환경과 ROM 빌드
- mGBA 실행 확인
- `GAIN ROGUE` 타이틀과 `PRESS START` 표시
- START 입력을 통한 타이틀에서 게임 화면 전환
- 좌측 UI 40px, 중앙 전장 160px, 우측 UI 40px의 한 화면 구조
- 검사 캐릭터의 8방향 이동과 전장 경계 제한
- 마지막으로 바라본 방향 저장
- A 버튼 8방향 근접 공격
- 프레임 기반 `Hitbox`와 `Hurtbox` 충돌
- `Pushbox` 기반 이동 충돌
- 허수아비 무작위 생성, 체력, 피격과 제거
- 한 공격의 동일 대상 중복 피해 방지
- 8방향 검사 참격 이펙트와 공통 피격 이펙트
- 그래픽 생성·검증 도구와 프로젝트 작업 규칙

현재 단계는 [M1 전투 실험장 #3](https://github.com/goodpeople14/gain-rogue-gba/issues/3)의 최종 검증 단계입니다. 실제 적 AI, 플레이어 체력, 스테이지 클리어와 로그라이크 반복은 아직 구현되지 않았습니다.

## 목표 게임 흐름

1. 캐릭터 2명 중 1명을 선택합니다.
2. 일반 전장 1에서 적과 싸우거나 피해서 출구에 도달합니다.
3. 무작위 강화 2개 중 1개를 선택합니다.
4. 일반 전장 2를 플레이합니다.
5. 무작위 강화 2개 중 1개를 선택합니다.
6. 보스전을 진행합니다.
7. 승리 또는 패배 결과를 확인합니다.
8. 처음부터 다시 시작합니다.

목표 플레이 시간은 약 10분입니다.

## 첫 완성 범위

- 서로 다른 특성을 가진 캐릭터 2명
- 8방향 이동과 기본 공격
- 일반 전장 2개
- 근접 적, 고정 원거리 적, 순찰 적과 출구 수비 적
- 출구 도달 방식의 전장 클리어
- 적 처치 수에 따른 보상 품질과 회복 아이템
- 무작위 강화 2개 중 1개 선택
- 강화 효과 유지와 플레이 종료 시 초기화
- 이동과 원거리 공격 패턴을 사용하는 보스
- 플레이어 체력, 사망과 재시작
- 승리·패배 결과 화면

## 강화 예시

- 공격력 증가
- 이동속도 증가
- 연사속도 증가
- 체력 회복
- 관통탄
- 다중탄

## 첫 작품 제외 범위

- 장비와 인벤토리
- 상점과 재화
- 영구 성장
- 세이브 데이터
- 스토리 장면
- 난이도 선택
- 온라인 기능
- 타일 단위 절차적 맵 생성
- 수십 개의 캐릭터

## 개발 단계

- [x] [M0 개발 기반 확인 — #1](https://github.com/goodpeople14/gain-rogue-gba/issues/1)
- [ ] [M1 전투 실험장 — #3](https://github.com/goodpeople14/gain-rogue-gba/issues/3) — 최종 검증 단계
- [ ] [M2 첫 번째 전투 — #4](https://github.com/goodpeople14/gain-rogue-gba/issues/4)
- [ ] [M3 출구 돌파 규칙 — #5](https://github.com/goodpeople14/gain-rogue-gba/issues/5)
- [ ] [M4 로그라이크 반복 — #6](https://github.com/goodpeople14/gain-rogue-gba/issues/6)
- [ ] [M5 캐릭터와 전장 확장 — #7](https://github.com/goodpeople14/gain-rogue-gba/issues/7)
- [ ] [M6 보스와 엔딩 — #8](https://github.com/goodpeople14/gain-rogue-gba/issues/8)
- [ ] [M7 공격 방식과 완성도 확장 — #9](https://github.com/goodpeople14/gain-rogue-gba/issues/9)
- [ ] [M8 배포 준비 — #10](https://github.com/goodpeople14/gain-rogue-gba/issues/10)

각 단계의 세부 범위와 완료 여부는 해당 이슈에서 관리합니다. 실제 ROM에서 완료 기준을 확인한 뒤 단계를 닫습니다.

## 개발 환경

| 구분 | 사용 도구 |
|---|---|
| Language | C++ |
| Engine | Butano |
| Toolchain | devkitARM |
| Build | GNU Make |
| Emulator | mGBA |
| Target | Game Boy Advance |

도구 확인:

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

```makefile
LIBBUTANO := ../butano/butano
```

다른 위치에 Butano를 설치했다면 실제 위치에 맞게 `LIBBUTANO`를 수정해야 합니다.

## 빌드 방법

프로젝트 루트에서 실행합니다.

```bash
cd C:/gba-dev/gain-rogue-gba
make -j2
```

정상적으로 완료되면 프로젝트 루트에 `gain-rogue-gba.gba`가 생성됩니다.

처음부터 다시 확인하려면 clean build를 실행합니다.

```bash
make clean
make -j2
```

`build/`, `.gba`, `.elf` 파일은 빌드 산출물이므로 Git에 커밋하지 않습니다.

## mGBA 실행 및 M1 확인

1. mGBA를 실행합니다.
2. `File → Load ROM`을 선택합니다.
3. `C:\gba-dev\gain-rogue-gba\gain-rogue-gba.gba`를 엽니다.
4. 다음 동작을 확인합니다.

- 실행 직후 타이틀 화면이 유지됩니다.
- START를 누르면 중앙 전장으로 전환됩니다.
- 검사 캐릭터가 8방향으로 이동하고 전장 밖으로 나가지 않습니다.
- A 버튼을 누르면 마지막으로 바라본 방향으로 공격합니다.
- 방향에 맞는 참격 이펙트와 공격 판정이 표시됩니다.
- 허수아비가 피해를 받고 체력이 0이 되면 제거됩니다.
- 공격이 적중한 위치에 공통 피격 이펙트가 표시됩니다.
- 허수아비가 플레이어와 겹치지 않는 위치에 다시 생성됩니다.

## 반복 개발 절차

```text
이슈에서 범위와 완료 기준 확인
→ 관련 프로젝트 규칙 확인
→ 기능 브랜치에서 구현
→ make -j2
→ mGBA에서 ROM 실행
→ git diff 확인
→ PR 검토와 main 병합
→ 이슈 체크리스트 갱신
```

빌드 환경이나 생성 자산까지 다시 검증할 때는 `make clean`을 먼저 실행합니다.

## 문제 해결

### mGBA 실행 중 make clean이 실패하는 경우

실행 중인 mGBA가 ROM 파일을 사용하고 있을 수 있습니다.

1. mGBA를 종료합니다.
2. 작업 관리자에서 mGBA 프로세스가 남아 있지 않은지 확인합니다.
3. 다시 clean build를 실행합니다.

### VS Code에서 Butano 헤더를 찾지 못하는 경우

`make`는 성공하지만 `bn_core.h` 같은 헤더 오류가 표시되면 IntelliSense 경로 문제일 가능성이 큽니다.

```text
C:/gba-dev/butano/butano/include
C:/gba-dev/butano/butano/hw/include
C:/gba-dev/butano/butano/hw/3rd_party/libtonc/include
```

프로젝트를 다른 위치에 설치했다면 실제 Butano 위치에 맞게 변경합니다.

## 최종 완료 조건

- mGBA에서 캐릭터 선택부터 보스전과 결과 화면까지 정상 진행됩니다.
- 한 판이 약 10분 안에 끝납니다.
- 두 캐릭터의 플레이 감각이 구분됩니다.
- 일반 전장 2회, 강화 선택 2회와 보스전이 연결됩니다.
- 사망과 재도전 흐름이 정상 동작합니다.
- 지인이 별도 설명 없이 플레이할 수 있습니다.
- 소스, ROM, 빌드와 조작 설명을 GitHub에 공개할 수 있습니다.
