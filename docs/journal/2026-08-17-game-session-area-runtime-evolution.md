# GameSession / AreaRuntime 중심 구조 논의와 단계적 전환 시점

관련 이슈: #63, #64

## 배경

프로젝트가 커지면서 `GameScene`이 담당하는 일이 빠르게 늘기 시작했다.

현재 `GameScene`은 Stage 진행, Player/Enemy update, Spawn 구성, Projectile/HitEffect lifecycle, Spatial 동기화, EXIT/Stage 전환, Stage message 등을 조정한다.

현재 구조가 당장 잘못된 것은 아니다. 오히려 `Character`, `Combat`, `SpatialManager`, `StageData`, `ProjectilePool`, `HitEffectManager` 같은 하위 책임은 비교적 잘 분리되어 있다.

하지만 앞으로 다음 기능이 들어오면 `GameScene`이 계속 커질 가능성이 높다.

- HP / Death
- Reward / Upgrade
- Stage 증가
- Sound
- Save
- 더 다양한 Area / Encounter

그래서 지금 당장 전면 리팩터링을 하는 대신, **어떤 책임을 언제 분리할지**를 먼저 정리했다.

## 목표는 범용 엔진이 아니다

이번 논의에서 가장 먼저 정한 원칙은 "다음 게임까지 다 해결하는 범용 엔진"을 만드는 것이 아니다.

현재 게임을 만들면서 실제로 반복되고 분리 가치가 확인된 책임만 구조화한다.

장기적으로 다음 같은 장르에서도 일부 재사용할 수 있는 뼈대를 생각했지만, 지금부터 모두 구현하지 않는다.

- Gain Ground형 Stage 전투 + Roguelike 진행
- Belt-scroll / Wave형 게임
- Town / Field / Dungeon을 오가는 Action RPG
- Vampire Survivors형 Spawn 게임
- 2D Monster Hunter형 사냥 게임

> 게임을 만들기 전에 엔진을 만드는 것이 아니라, 게임을 만들면서 엔진의 일부를 얻는다.

## 현재 유지할 구조

다음 구성은 구조 개편의 제거 대상이 아니다.

- `Character`와 구체 캐릭터
- `PlayerController`
- `Hitbox / Hurtbox / Pushbox`
- `SwordsmanAttack`
- `CrossbowProjectilePool`
- `HitEffectManager`
- `SpatialManager`
- `SpatialLayer`
- `StageData`
- `StageStaticObstacleData`
- `Battlefield`
- Debug Overlay
- `bn::array`, fixed capacity, pool 중심 ownership

특히 SpatialManager 원칙은 유지한다.

> SpatialManager는 위치, 공간, 주변 후보를 제공하며 gameplay state를 직접 변경하지 않는다.

ECS, Event Bus, Service Container, 과도한 Manager 계층도 현재는 만들지 않는다.

## 장기 목표 구조

논의 끝에 다음 정도의 책임 분리를 장기 목표로 잡았다.

```text
Game
├─ GameSession
├─ AudioSystem
├─ SaveSystem
├─ PersistentData
└─ Scene
   ├─ TitleScene
   └─ GameScene
      ├─ AreaRuntime
      │  └─ EncounterRuntime*  필요할 때만
      ├─ HUD / View
      └─ Debug / View
```

그리고 AreaRuntime 아래에는 현재의 StageData, Actor, SpatialManager, Projectile, Effect 같은 실제 runtime 요소가 배치될 수 있다.

`EncounterRuntime`처럼 아직 실제 책임 분리가 확실하지 않은 구조는 이름부터 만들지 않는다.

## GameSession

GameSession은 한 번의 현재 플레이, 즉 Run의 진행 상태를 표현한다.

후보 책임:

- 현재 Stage / 진행도
- Run state
- 선택한 Upgrade
- Reward 진행
- 필요하면 RNG seed / random state
- Run Clear / Failed
- 다음 Stage 결정

중요한 점은 GameSession이 `GameScene`보다 오래 살아야 할 수 있다는 것이다.

예를 들어 Stage1에서 얻은 Upgrade가 Stage2에도 유지된다면 그 상태는 특정 Stage runtime의 소유물이 아니다.

따라서 장기적으로 `Game`이 GameSession을 소유하는 방향이 자연스럽다.

GameSession과 SaveData도 구분한다.

```text
GameSession
= 현재 실행 중인 Run 상태

PersistentData
= 전원을 꺼도 남아야 하는 데이터
```

## AreaRuntime

현재 게임에서는 Area와 Stage를 같은 의미로 봐도 된다.

따라서 지금 당장 `stage1.h`, `stage2.h` 같은 이름을 Area로 바꿀 필요는 없다.

AreaRuntime이 필요한 시점이 오면 다음 책임이 후보가 된다.

- 현재 Stage/Area data
- Battlefield / Map visual
- SpatialManager
- Actor pool
- Projectile / HitEffect 등 공간 수명에 묶인 runtime object
- EXIT / Area 이동 요청

중요한 원칙은 Area가 다음 Area를 스스로 결정하지 않는 것이다.

```text
AreaRuntime
→ EXIT 사용 사실 전달

GameSession
→ 진행/보상/조건 판단
→ 다음 Area 결정
```

이렇게 해야 Stage의 공간 책임과 Run 진행 규칙이 섞이지 않는다.

## EncounterRuntime

현재 `StagePhase`의 일부는 공간 자체보다 전투 lifecycle에 가깝다.

```text
INTRO
READY
GO
PLAYING
CLEARED
```

하지만 전투가 CLEARED가 되었다고 해서 Slash, Projectile, Effect 같은 모든 공간 객체를 동시에 없애야 하는 것은 아니다.

실제로 #62에서는 `PLAYING → CLEARED` 전환 시 transient visual lifecycle을 너무 일찍 종료하는 문제가 있었다.

이 경험은 "Stage/Area의 수명"과 "Encounter phase"가 반드시 같은 것은 아니라는 점을 보여줬다.

다만 현재는 이 한 사례만으로 큰 Encounter framework를 만들지 않는다. Vertical Slice를 완성한 뒤 실제 책임이 반복되는지 보고 도입한다.

## AudioSystem

Sound가 처음 들어오는 시점부터는 Butano audio 호출을 여러 gameplay class에 흩뿌리지 않는 방향을 잡았다.

초기 AudioSystem은 매우 작아도 된다.

```text
play_bgm(BgmId)
stop_bgm()
play_sfx(SfxId)
```

필요할 때만 volume, fade, SFX priority 같은 기능을 추가한다.

AudioSystem은 gameplay state를 변경하지 않는다.

```text
Area / Combat / UI
→ AudioSystem에 재생 요청
```

정도로 유지한다.

## PersistentData / SaveSystem

실제로 저장할 데이터가 생기기 전까지 Save framework를 만들지 않는다.

장기적으로는 다음처럼 구분한다.

```text
GameSession
= 현재 플레이 상태

PersistentData
= 전원을 꺼도 남아야 할 정보

SaveSystem
= PersistentData를 실제 save memory에 기록/복구
```

SaveSystem은 "언제 저장할지"를 결정하지 않는다. 게임 규칙이 저장 시점을 결정하고 SaveSystem은 기록과 복구만 담당한다.

실제 구현 시점에는 version, magic, checksum, 필요 시 dual slot 등을 검토한다.

## 구조는 기능이 필요해지는 순간에 도입한다

이번 논의에서 가장 중요한 것은 최종 구조보다 **전환 시점**이었다.

### 1. 지금

구조 방향만 기록한다. 전면 리팩터링하지 않는다.

### 2. HP / Death 구현 시

공통 `Health`와 최소 Damage 계약을 도입한다.

`HealthManager`, `DeathManager`는 만들지 않는다.

이 단계는 이후 #67 작업을 통해 실제로 시작되었다.

### 3. Reward / Upgrade 직전

Stage를 넘어 유지되는 상태가 실제로 생기므로 최소 `GameSession`을 도입한다.

이 시점이 GameSession의 가장 자연스러운 시작점이다.

### 4. Stage 증가 / Random Spawn 전후

Stage 구성을 `StageDefinition` 계열 데이터로 정리한다.

후보 데이터:

- visual
- ground/upper StageData
- obstacle
- player spawn
- enemy type/count/spawn
- SpatialLayer
- exit
- 필요 시 BGM id

Stage가 늘어날수록 `GameScene`의 if/switch가 늘지 않도록 한다.

### 5. 첫 Vertical Slice 완성 후

전체 게임 흐름이 실제로 동작한 뒤 `GameScene` 책임을 다시 분류한다.

```text
Game Start
→ Stage Combat
→ Clear
→ Reward
→ Upgrade
→ Next Stage
→ Death 또는 Run Clear
→ Restart
```

이때 실제 코드에서 확인된 책임만 `GameSession`, `AreaRuntime`, 필요 시 `EncounterRuntime`, HUD/View 쪽으로 이동한다.

이 시점을 현재 게임의 주요 Architecture Refactoring 시점으로 본다.

### 6. Sound 첫 구현 시

작은 AudioSystem을 도입한다.

### 7. 영구 데이터가 실제로 생길 때

PersistentData + SaveSystem을 도입한다.

### 8. 다음 게임

현재 게임에서 실제로 검증된 Stage 구조만 필요에 따라 Area 구조로 일반화한다.

## 다음 게임에서의 재사용 기준

현재 구조가 잘 동작했다고 해서 모든 장르 규칙을 공통 엔진으로 만들 필요는 없다.

재사용 가치가 높은 것은 다음 같은 부분이다.

- ownership
- lifecycle
- data/runtime 분리
- fixed pool
- Spatial 책임
- Scene과 Run 진행 분리

반대로 장르의 재미 자체를 만드는 규칙은 게임 전용으로 남긴다.

예:

```text
Vampire Survivors형
→ SpawnDirector / Time Curve는 해당 게임 전용

2D Monster Hunter형
→ Monster AI / Parts / Hunt rule은 해당 게임 전용
```

## 변경 시 판단 기준

```text
실제 기능 요구가 생겼는가?
        ↓
같은 책임이 반복되고 있는가?
        ↓
현재 owner가 부자연스러운가?
        ↓
최소한의 경계만 분리
```

반대로 다음과 같다면 보류한다.

```text
언젠가 필요할 것 같음
→ 이름부터 class 생성
→ manager/framework 추가
```

이 패턴은 피한다.

## 최종 원칙

이번 논의를 통해 얻은 구조 원칙은 다음과 같다.

1. 현재 잘 동작하는 Character / Combat / Spatial 구조를 먼저 유지한다.
2. Stage를 넘어 유지되는 상태가 생기는 시점에 GameSession을 도입한다.
3. Stage가 늘어날 때 정적 구성과 runtime 상태를 분리한다.
4. Vertical Slice 이전에는 GameScene을 대규모로 다시 쓰지 않는다.
5. Audio와 Save는 실제 요구가 처음 생기는 순간 최소 구조로 시작한다.
6. 이름만 있는 미래 framework를 만들지 않는다.
7. 현재 게임에서 검증된 구조만 다음 게임에서 일반화한다.

```text
현재 게임 구현
        ↓
실제 반복 책임 발견
        ↓
최소 구조 분리
        ↓
Vertical Slice로 검증
        ↓
다음 게임에서 재사용 여부 판단
```

> 현재 게임에서 반복적으로 나타난 책임만 분리하고, 첫 게임에서 검증된 구조를 다음 게임에서 일반화한다.
