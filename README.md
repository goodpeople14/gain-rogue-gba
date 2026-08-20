# Gain Rogue GBA

Gain Ground식 전투를 기반으로 로그라이크 반복 구조를 더해 가는 Game Boy Advance 액션 게임 프로젝트입니다.

Butano / C++로 실제 GBA ROM을 만들고, 기능을 작은 Issue 단위로 구현한 뒤 `make`, mGBA, 실제 화면 확인을 거쳐 확장합니다.

## 현재 프로젝트 상태

현재 `main`은 초기 전투 실험 단계를 지나 **실제 근접/원거리 전투, Stage 진행, Spatial 기반 이동, Player HP/Death/HUD까지 구현된 상태**입니다.

완료된 큰 단계:

- [x] [M0 개발 기반 확인 — #1](https://github.com/goodpeople14/gain-rogue-gba/issues/1)
- [x] [M1 전투 실험장 — #3](https://github.com/goodpeople14/gain-rogue-gba/issues/3)
- [x] [M2 첫 번째 전투 — #4](https://github.com/goodpeople14/gain-rogue-gba/issues/4)
- [x] [M3 출구 돌파 규칙 — #5](https://github.com/goodpeople14/gain-rogue-gba/issues/5)

다음 핵심 단계는 [M4 로그라이크 반복 — #6](https://github.com/goodpeople14/gain-rogue-gba/issues/6)입니다.

## 현재 구현된 기능

### Player

- 16×16 기반 8방향 Samurai Sprite
- 8방향 자유 이동
- 마지막 바라본 방향 유지
- A 버튼 8방향 근접 공격
- 방향별 Slash Effect
- Player HP 3
- `SAMURAI` 이름 + 고정 HP HUD
- 피격 / 사망 처리
- HP 0 시 `YOU DIED` 표시 후 Title로 복귀
- 새 Run 시작 시 HP와 전투 상태 초기화

### Enemy / Combat

- 근접 Goblin AI
- Crossbow Goblin 원거리 공격
- Hitbox / Hurtbox / Pushbox 분리
- Projectile 충돌
- 공통 Hit Effect
- Enemy Health 기반 Damage / Death 처리
- 일반 Goblin / Crossbow Goblin HP 1 유지
- 다수 Goblin 독립 행동과 상호 Pushbox
- Enemy 상태 아이콘 및 Telegraph / Recovery 표현
- 전투 종료 / Scene 전환 시 transient effect 정리

### Stage / Spatial

- 실제 Stage1 Background asset
- Stage 진행 흐름
- Stage Intro / READY / GO / PLAYING / CLEARED 상태
- Stage 종료 후 다음 Stage 진행
- Stage2 battlefield skeleton
- `SpatialManager` 기반 위치 / 주변 후보 조회
- 8×8 cell 기반 broad-phase 공간 조회
- 지역 Cell 범위 조회 최적화
- `SpatialLayer { GROUND, UPPER }` 기반 floor-aware movement
- Character / Enemy 이동 시 Spatial / Pushbox 계약 적용

### Graphics / GBA Resource

- OBJ BPP4 palette exhaustion 문제 수정
- Sprite Palette Harness 적용
- Character Sprite Editing Harness 적용
- Samurai 8방향 production asset 검증 도구
- direction / palette / baseline / center / silhouette 검증
- 생성 이미지와 실제 GBA production asset 편집 과정을 분리

## 현재 구조 방향

현재 게임은 한 번에 범용 엔진을 만드는 대신 실제 게임 기능을 먼저 만들고, 반복해서 필요한 책임만 구조로 승격합니다.

주요 원칙:

```text
SpatialManager
= 위치 / 주변 후보 / 공간 정보 제공
= 게임 상태를 직접 변경하지 않음

Character / Combat / Game logic
= Damage / Health / Death / Effect 의미 처리
```

장기 구조는 다음 방향을 검토 중입니다.

```text
Game
├─ GameSession        # Run 진행 상태
├─ AudioSystem        # 실제 사운드 도입 시
├─ SaveSystem         # 저장 상태가 생길 때
└─ Scene
   ├─ TitleScene
   └─ GameScene
       └─ AreaRuntime
          └─ EncounterRuntime (필요할 때)
```

아직 필요하지 않은 Manager / ECS / 범용 Event framework는 선도입하지 않습니다.

## 다음 작업 후보

현재 가장 자연스러운 다음 Gameplay 범위는 M4입니다.

```text
전투
→ Stage Clear
→ 무작위 강화 선택
→ 다음 Stage
→ 강화 유지
→ Death 시 Run 초기화
```

구체적으로는:

- 무작위 강화 선택 UI
- 공격력 / 이동속도 등 최소 Upgrade
- 회복 선택지
- Stage 간 Upgrade 유지
- Death 시 Upgrade 초기화
- 최소 2~3 Stage Run 흐름 연결

을 작은 단위로 구현할 예정입니다.

## 장기 아이디어 / 아직 확정하지 않은 것

다음 항목은 설계 아이디어 또는 후속 검토 대상이며 현재 production 기능으로 보지 않습니다.

- `PlayableCharacter / EnemyCharacter` 중간 계층
- Character 교체 / Tag Team
- Leader + Assist Squad
- Team shared HP
- Elite / Boss 전용 고정 HP HUD
- Camera scroll / 큰 World map
- AudioSystem / SaveSystem
- Character Animation Harness

특히 Walk / Attack / Hit animation 제작법은 아직 검증된 공통 Harness로 등록하지 않았습니다.

## 문서와 하네스

프로젝트 작업 시 다음 문서를 기준으로 합니다.

- [작업 절차](docs/workflow.md)
- [현재 구조](docs/architecture.md)
- [전투 규칙](docs/combat-rules.md)
- [충돌 규칙](docs/collision-rules.md)
- [그래픽 규칙](docs/graphics-rules.md)
- [GBA Sprite Palette Harness](docs/gba-sprite-palette.md)
- [GBA Character Sprite Editing Harness](docs/gba-character-sprite-editing.md)
- [설계 결정 기록](docs/decisions/)
- [개발 Journal](docs/journal/)

초기 게임 기획은 [Issue #11](https://github.com/goodpeople14/gain-rogue-gba/issues/11)에 남아 있습니다. 다만 구현 과정에서 일부 방향이 발전했으므로 **현재 코드, 열린 Issue, 결정 문서와 Harness를 최신 상태의 우선 근거로 사용합니다.**

## 개발 방식

기본 흐름:

```text
Issue에서 목표 / 완료 조건 정의
→ 관련 Harness / Decision 확인
→ 기능 브랜치 생성
→ 구현
→ 자동 검증
→ make -j2
→ 필요 시 clean build
→ mGBA 확인
→ PR 검토
→ main merge
→ Issue 완료
```

기능을 만든 뒤 반복해서 얻은 교훈은 `docs/journal/`에 남기고, 재사용 가치가 검증된 규칙만 Harness로 승격합니다.

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
   ├─ src\
   ├─ graphics\
   ├─ tools\
   └─ docs\
```

```makefile
LIBBUTANO := ../butano/butano
```

다른 위치에 Butano를 설치했다면 실제 위치에 맞게 `LIBBUTANO`를 수정해야 합니다.

## 빌드 방법

프로젝트 루트에서:

```bash
cd C:/gba-dev/gain-rogue-gba
make -j2
```

정상적으로 완료되면 프로젝트 루트에 ROM이 생성됩니다.

처음부터 다시 검증하려면:

```bash
make clean
make -j2
```

`build/`, `.gba`, `.elf`, emulator save/state 파일은 Git에 커밋하지 않습니다.

## mGBA 확인

현재 main에서 최소 다음 흐름을 확인할 수 있습니다.

1. Title에서 START
2. Samurai 8방향 이동
3. Goblin / Crossbow Goblin과 전투
4. Slash / Projectile / Hit Effect 확인
5. Player HP 감소
6. HP 0 시 `YOU DIED`
7. Title 복귀
8. 새 게임 시작 시 상태 초기화
9. Stage 진행 및 Stage 전환 확인

Debug 기능을 사용할 때는 Palette / Collision Debug Overlay가 기존 Gameplay와 충돌하지 않는지도 함께 확인합니다.

## 프로젝트 목표

첫 번째 목표는 거대한 엔진이 아니라 **GBA에서 실제로 처음부터 끝까지 플레이 가능한 작은 게임**을 만드는 것입니다.

현재는 전투 기반과 Stage 기반을 확보했으며, 다음 단계에서 로그라이크 선택과 Run 반복을 연결해 vertical slice를 완성하는 방향으로 진행합니다.

장기적으로는 이 과정에서 검증된 구조와 Harness를 다음 GBA 게임에도 재사용할 수 있는 수준으로 축적하는 것을 목표로 합니다.
