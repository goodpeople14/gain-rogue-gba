# 원거리 AI를 최적 위치 탐색에서 거리 영역 기반으로 단순화한 과정

관련 이슈: #53, #54, #55

## 배경

Stage2에 Crossbow Goblin 수를 늘린 뒤 체감 성능 저하가 나타났다. 처음에는 단순히 적 수가 많아서 느려졌다고 볼 수도 있었지만, 실제로는 원거리 AI가 매 update에서 반복하는 의사결정 구조가 문제 후보였다.

기존 Crossbow CHASE는 다음 작업을 계속 수행했다.

- Player 거리 판정
- 8방향 공격 위치 후보 생성과 비교
- Commit Box 반복 검사
- 최적 공격 위치 계산
- 해당 위치로 이동
- Spatial movement query / resolve

적 수가 늘어나면 이 비용도 그대로 반복된다. 특히 이미 공격 가능한 거리에서도 계속 "더 좋은 사격 위치"를 찾는 구조는 GBA에서 필요 이상으로 비쌀 수 있었다.

## 처음 검토했지만 버린 방향

Stage2 UPPER Crossbow를 고정 포탑처럼 만드는 방법도 생각했다. 구현은 단순해질 수 있지만 GROUND 원거리 적에서도 같은 문제가 다시 생긴다. 특정 Stage를 위한 우회일 뿐 원거리 적의 공통 문제를 해결하지 못한다.

별도의 Ranged Navigation, A*, BFS, NavMesh 같은 시스템을 만드는 것도 현재 게임 규모에 비해 과했다.

그래서 새로운 이동 시스템을 만들기보다 이미 잘 동작하는 Melee Goblin의 이동 방식을 재사용하는 쪽으로 방향을 바꿨다.

## 이미 검증된 Melee 이동을 재사용한다

Melee Goblin은 다음처럼 단순한 구조를 사용한다.

```text
Player 방향으로 직접 이동
→ 잠깐 막히면 그대로 시도
→ 일정 frame 이상 막히면 local detour
→ 직접 경로가 열리면 즉시 원래 추적 복귀
```

A*/BFS 없이도 작은 전장에서 충분히 자연스럽게 동작했다.

여기서 중요한 생각이 나왔다.

> 원거리 적은 이동 방법이 다른 적이 아니라, 원하는 거리가 다른 적이다.

따라서 원거리 적도 같은 이동 기반을 사용하고 거리 정책만 다르게 두면 된다.

## SEARCH + CHASE BOX + FLEE BOX

최종적으로 원거리 적의 이동 의도를 세 영역으로 단순화했다.

```text
SEARCH
  ↓ Player 발견

CHASE BOX 밖
→ APPROACH
→ Player를 target으로 기존 Seek + Local Avoidance

CHASE BOX 안 + FLEE BOX 밖
→ HOLD
→ TELEGRAPH → FIRE → RECOVERY

FLEE BOX 안
→ RETREAT
→ Player 반대편의 가상 target으로 이동
```

핵심은 **최적 사격 위치를 매 frame 찾지 않는 것**이다.

- 너무 멀면 가까이 간다.
- 적정 거리면 멈춰서 공격한다.
- 너무 가까우면 뒤로 빠진다.

이 정도의 규칙이면 원거리 적의 역할을 충분히 표현할 수 있다.

## 공통 이동 구조

```text
Melee
- target = Player
- APPROACH

Ranged APPROACH
- target = Player

Ranged RETREAT
- target = Player 반대편 가상점

공통
- desired direction
- local obstacle avoidance
- resolve_movement
```

이 구조의 장점은 검병, 창병, 궁수, 마법사마다 전혀 다른 Navigation을 만들지 않아도 된다는 점이다. 행동 차이는 이동 엔진보다 거리 정책과 공격 방식에서 만들 수 있다.

## TDD로 잠근 행동 계약

구현 세부보다 다음 행동을 계약으로 잡는 것이 중요했다.

- SEARCH 밖에서는 추적과 공격을 시작하지 않는다.
- CHASE BOX 밖이면 APPROACH한다.
- CHASE BOX 안이면서 FLEE BOX 밖이면 HOLD하고 공격할 수 있다.
- FLEE BOX 안이면 RETREAT한다.
- CHASE/FLEE 경계에서는 불필요한 진동을 피하기 위해 HOLD를 우선한다.
- 짧은 block은 즉시 detour로 전환하지 않는다.
- 일정 frame 이상 막혔을 때만 local detour를 시작한다.
- 직접 경로가 다시 열리면 즉시 detour를 종료한다.
- TELEGRAPH 이후 projectile은 한 번만 발사하고 RECOVERY를 거친다.

테스트의 목적은 특정 구현을 고정하는 것이 아니라 이 행동을 보장하는 것이다.

## 40마리 사고 실험에서 드러난 두 번째 병목

AI를 단순화해도 SpatialManager가 actor마다 Stage 전체 cell을 훑으면 적 수가 많아질 때 다른 병목이 남는다.

Stage2가 20×20이면 400 cells다.

```text
400 cells × 40 actors
= 16,000 cell checks / frame
```

이 숫자는 CPU 실측값이 아니라 기존 루프 구조를 확장했을 때의 단순 반복 수다. 하지만 movement query가 실제로 필요한 것은 actor 주변 몇 칸뿐인데 Stage 전체를 스캔하는 것은 구조적으로 낭비였다.

그래서 SpatialManager의 책임은 그대로 두고 후보 탐색 방식만 다음처럼 바꾸는 방향을 잡았다.

```text
기존
movement_area
→ Stage 전체 scan
→ 겹치는 BLOCKED cell 선택

개선
movement_area
→ min/max cell 계산
→ 실제 닿는 지역 cell만 검사
```

이 후속 최적화는 #55에서 별도 작업으로 분리되었고 이후 main에 반영되었다.

## 성능 문제를 분리해서 본다

40 Crossbow는 실제 Stage에 40마리를 넣겠다는 기획이 아니다. 구조의 확장성을 보기 위한 stress target이다.

```text
AI only
AI + movement
AI + movement + projectile
Render / sprite
Debug OFF / ON
```

을 분리해서 봐야 한다.

AI만 안정적인데 projectile이나 sprite 한계에서 문제가 생긴다면 그것은 AI 실패가 아니다. 반대로 Render를 끈 상태에서도 느리다면 AI/Spatial을 더 조사해야 한다.

## 이번 경험에서 얻은 원칙

1. 적 수가 늘어 느려졌을 때 개별 적을 약화시키기 전에 반복되는 의사결정 구조를 본다.
2. 이미 잘 동작하는 단순한 시스템을 재사용하는 것이 새로운 범용 시스템을 만드는 것보다 싸고 안전할 수 있다.
3. 원거리 AI의 본질은 "최적 위치 탐색"이 아니라 "거리 정책"일 수 있다.
4. GBA에서는 정교한 최적해보다 계산량이 작고 예측 가능한 규칙이 더 가치 있을 수 있다.
5. 성능은 AI, Spatial, Projectile, Render를 분리해 측정한다.
6. Stress test 수치는 실제 콘텐츠 목표와 분리한다.

> 가장 좋은 최적화는 복잡한 계산을 더 빠르게 만드는 것이 아니라, 필요하지 않은 계산 자체를 없애는 것일 수 있다.
