# Collision 규칙

이 문서는 캐릭터와 월드 오브젝트의 충돌 영역을 설계할 때 사용하는 공통 규칙을 정의한다.

Collision 영역은 화면에 보이는 Sprite 크기와 동일하다고 가정하지 않는다.

핵심 원칙:

> Visual Bounds와 Gameplay Collision Bounds는 서로 다른 책임이다.

## 1. Collision 영역의 역할

현재 게임에서는 충돌 영역을 역할에 따라 구분한다.

### Pushbox

캐릭터가 이동할 때 다른 Actor와 서로 밀리거나 겹치지 않도록 사용하는 공간 점유 영역이다.

### Hurtbox

캐릭터가 공격을 받을 수 있는 영역이다.

### Hitbox

공격이 상대에게 피해를 줄 수 있는 영역이다.

### Static Obstacle

바위, 구조물 등 움직이지 않는 월드 오브젝트의 이동 충돌 영역이다.

각 영역은 목적이 다르므로 하나의 Box를 모든 판정에 공통으로 사용하지 않는다.

## 2. Pushbox 기본 원칙

Pushbox는 Sprite 전체 외곽을 감싸는 Bounding Box가 아니다.

캐릭터가 실제 공간을 점유한다고 느껴지는 몸통 중심 영역을 표현한다.

기본 시각 기준은 다음과 같다.

- 머리 끝까지 포함하지 않는다.
- 발끝까지 포함하지 않는다.
- 무기와 장식은 포함하지 않는다.
- 캐릭터의 중심 몸체를 기준으로 한다.
- 현재 인간형 캐릭터는 대략 어깨 아래부터 무릎 부근을 기준으로 삼는다.

Sprite의 크기와 Pushbox 크기가 다른 것은 정상이다.

## 3. Pushbox 크기를 Sprite 크기로부터 계산하지 않는다

다음과 같은 고정 비율 규칙을 만들지 않는다.

```text
Pushbox = Sprite width × 일정 비율
Pushbox = Sprite height × 일정 비율
```

각 캐릭터의 실제 시각적 체형과 플레이 감각을 기준으로 결정한다.

따라서 같은 16×16 Sprite를 사용하더라도 서로 다른 Pushbox를 가질 수 있다.

예를 들어 현재 값이 Player 8×8, Goblin 6×6이라고 해도 숫자 자체가 정책은 아니다.

정책은 다음이다.

> Sprite 전체가 아니라 실제 공간 점유 영역을 Pushbox로 사용한다.

## 4. Pushbox 위치

Pushbox 크기뿐 아니라 Sprite 내부에서의 위치도 별도로 결정한다.

작은 Pushbox라도 발쪽으로 지나치게 치우치거나 머리쪽으로 치우치면 이동 감각이 부자연스러울 수 있다.

따라서 Pushbox를 변경할 때는 항상 다음을 함께 확인한다.

- width
- height
- X offset
- Y offset

특히 인간형 캐릭터는 실제 Sprite의 몸통 위치를 기준으로 정렬한다.

## 5. Pushbox와 Hurtbox를 함께 변경하지 않는다

Pushbox를 줄였다고 Hurtbox를 자동으로 줄이지 않는다.

예를 들어 Pushbox를 줄여 캐릭터끼리 더 자연스럽게 접근하게 하면서 Hurtbox는 기존 공격 판정을 위해 유지할 수 있다.

Pushbox와 Hurtbox는 서로 다른 게임 플레이 목적을 가진다.

Pushbox 수정 작업에서 Hitbox/Hurtbox 변경이 필요하다면 별도 근거를 확인한다.

## 6. Static Obstacle도 Visual 크기와 다를 수 있다

바위와 같은 월드 오브젝트도 동일한 원칙을 따른다.

```text
Visual Rock Bounds != Static Obstacle Bounds
```

둥근 바위를 Visual 크기 전체와 같은 사각형 충돌 영역으로 처리하면 모서리에서 시각적으로 닿지 않았는데도 이동이 막힐 수 있다.

따라서 오브젝트의 실제 점유 면적에 맞춰 Collision 영역을 Visual보다 작게 둘 수 있다.

## 7. StageData와 Static Obstacle의 역할

큰 지형 구조와 개별 오브젝트의 충돌을 구분한다.

### StageData BLOCKED

다음과 같은 구조에 사용한다.

- 전장 외곽
- 벽
- 큰 고정 구조물
- 이동할 수 없는 지형

### Static Obstacle

다음과 같은 개별 오브젝트에 사용한다.

- 바위
- 상자
- 기둥
- 개별 배치 구조물

개별 오브젝트를 단순히 StageData의 여러 BLOCKED Cell로 표현하여 Visual보다 지나치게 큰 사각 충돌이 생기지 않도록 한다.

## 8. Movement collision과 slide

Pushbox 또는 Static Obstacle의 크기는 "언제 충돌이 시작되는가"를 결정한다.

movement resolver의 slide 정책은 "충돌이 시작된 뒤 어떤 이동 성분을 유지하는가"를 결정한다.

두 책임을 혼동하지 않는다.

현재 movement collision은 대각 이동 중 충돌 시 dominant-axis를 우선하는 slide 방식을 사용한다.

Pushbox 문제를 해결하기 위해 slide 알고리즘을 임의로 변경하지 않고 각각의 원인을 분리해서 검증한다.

## 9. Debug 규칙

Collision Debug에서 표시하는 Pushbox는 실제 movement collision에 사용하는 Pushbox와 동일해야 한다.

```text
Debug Pushbox == Gameplay Pushbox
```

시각화를 위해 별도의 가짜 Box 크기를 만들지 않는다.

Debug는 문제 확인을 위한 수단이며 단순한 Collision 튜닝을 위해 별도의 복잡한 Debug framework를 만들지 않는다.

## 10. 검증 원칙

Pushbox 변경은 가능한 경우 행동 기준으로 검증한다.

- 캐릭터 머리 영역만 가까워졌다고 이동이 막히지 않는다.
- 발끝만 가까워졌다고 이동이 막히지 않는다.
- 몸통끼리 겹치려 하면 이동이 차단된다.
- 두 Actor가 옆을 스쳐 지나갈 수 있다.
- Pushbox가 작아져도 Actor가 지나치게 관통하지 않는다.
- Static Obstacle 모서리를 자연스럽게 통과한다.

숫자를 먼저 정하고 테스트를 숫자에 맞추지 않는다.

테스트는 원하는 플레이 행동을 계약으로 표현해야 한다.

## 11. TDD 사용 원칙

Collision 문제는 가능한 경우 다음 순서로 해결한다.

```text
mGBA에서 이상한 움직임 발견
→ 재현 가능한 행동 규칙으로 변환
→ RED test
→ 최소 수정
→ GREEN
→ mGBA 플레이 감각 확인
```

이미 현재 구현이 원하는 행동을 만족한다면 실패 테스트를 인위적으로 만들지 않는다.

TDD의 목적은 실패를 만드는 것이 아니라 아직 보장되지 않은 행동을 실행 가능한 계약으로 만드는 것이다.

## 12. Stage2 이후 확장

Stage2에서 Floor 개념이 도입되면 이 문서에 층별 Collision 규칙을 추가한다.

현재 예정된 기본 규칙은 다음과 같다.

```text
Movement / Pushbox
→ 같은 floor만 충돌

Melee Hit/Hurt
→ 같은 floor만 유효

Projectile Hit/Hurt
→ 다른 floor도 공격 가능
```

SpatialManager는 공격 의미를 해석하지 않는다.

각 Gameplay 시스템이 Collision 후보와 Floor 정보를 바탕으로 자신의 규칙을 판단한다.
