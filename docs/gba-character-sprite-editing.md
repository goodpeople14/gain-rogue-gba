# GBA Character Sprite Editing Harness

## 목적

참고 이미지나 AI 생성 이미지를 그대로 GBA Sprite로 취급하지 않고, **승인된 캐릭터 정체성을 유지한 채 작은 Pixel Grid의 production asset으로 편집·정제하는 절차**를 정의한다.

이 문서는 Character의 정적 8방향 Sprite를 만드는 **이미지 편집 Harness**다. Walk / Attack / Hit 같은 Animation 제작 규칙은 아직 포함하지 않는다. Animation 규칙은 실제 Samurai Walk PoC를 성공시키고 mGBA에서 승인한 뒤 별도로 추가한다.

관련 규칙:

- 그래픽 공통 규칙: `docs/graphics-rules.md`
- OBJ Palette 규칙: `docs/gba-sprite-palette.md`
- 방향 순서 Source of Truth: `include/game/direction.h`

---

## 1. 왜 `생성`과 `편집`을 구분하는가

이 프로젝트에서 반복해서 확인한 문제는 다음과 같다.

```text
"16x16 Sprite를 만들어 달라"
        ↓
AI가 큰 새 이미지를 생성
        ↓
축소 / quantize
        ↓
원본 실루엣, 체형, 방향, palette가 흔들림
```

따라서 production 단계의 기본 작업은 새 그림을 자유롭게 만드는 `generation`이 아니라, 승인된 reference/base를 작은 Grid 안에서 유지·정제하는 `editing`으로 본다.

```text
Concept / Reference
        ↓
Base Sprite 후보
        ↓ 사용자 승인
Production Editing
        ↓
Direction / Palette / Pixel 검증
        ↓
Butano build
        ↓
mGBA 승인
```

AI 이미지 생성은 Concept 또는 Base 후보를 얻는 데 사용할 수 있지만, 그 결과를 production asset이라고 간주하지 않는다.

---

## 2. 현재 Samurai에서 실제로 성공한 경로

현재 Player Samurai는 PR #65에서 production asset으로 반영되었고 build와 mGBA 사용자 확인을 통과했다.

관련 파일:

```text
docs/reference/player_16color_samurai_reference.png
docs/reference/samurai-palette.png

graphics/characters/heroes/swordsman/swordsman_8dir.png
graphics/characters/heroes/swordsman/swordsman_8dir_sheet.bmp

tools/generate_samurai_player_sheet.py

docs/review/player_samurai_16color_8dir_preview.png
docs/review/player_samurai_16color_comparison.png
```

성공 과정에서 중요했던 흐름은 다음과 같다.

```text
원본 / 참고 Character 이미지
        ↓
작은 Sprite로 읽을 수 있는 기준 이미지 확보
        ↓
16x16 Pixel Grid 기준으로 정제
        ↓
8방향 Character identity 일치 확인
        ↓
고정된 shared palette로 매핑
        ↓
Direction index 정합성 확인
        ↓
자동 silhouette / center / baseline 검증
        ↓
확대 Preview / Comparison으로 시각 검토
        ↓
Butano build
        ↓
mGBA 사용자 승인
```

핵심은 **큰 이미지를 마지막에 단순 축소한 것이 아니라, 16x16 Grid에서 어떤 Pixel과 Palette index를 사용할지 통제한 것**이다.

현재 Samurai helper는 다음 production 계약을 코드로 잠근다.

- frame size: 16x16
- frame count: 8
- direction order: `DOWN, DOWN_LEFT, LEFT, UP_LEFT, UP, UP_RIGHT, RIGHT, DOWN_RIGHT`
- indexed image 사용
- 16-entry shared palette table 사용
- palette index 0을 transparent chroma key로 사용
- 모든 Pixel index가 0~15 범위
- 모든 방향의 발 baseline 정렬
- 방향별 높이 차이, 중심 이동, body mass가 과도하게 흔들리지 않도록 검사
- DOWN frame의 helmet / shoulder / torso / leg silhouette 최소 조건 검사
- 좌우 방향 수정 시 frame index 자체는 바꾸지 않고 필요한 lateral frame만 mirror

이 숫자와 silhouette 조건은 현재 Samurai용 검증값이다. 새 Character에 그대로 복사하지 말고, 새 Base Sprite의 실제 체형에 맞춰 별도 기준을 정한다.

---

## 3. Production 작업의 기본 원칙

### 3.1 먼저 Base Sprite를 승인한다

새 Character를 만들 때 8방향 전체를 한 번에 자유 생성하지 않는다.

```text
Reference
   ↓
대표 방향 Base Sprite
   ↓
사용자 승인
   ↓
나머지 방향 파생
```

Base 단계에서 먼저 확인한다.

- 전체 실루엣
- 머리 / 몸통 / 다리 비율
- 장비와 무기 정체성
- 발 baseline
- Sprite 중심
- outline
- 주요 색 구분
- 16x16에서 읽히는지 여부

Base가 마음에 들지 않으면 8방향으로 확장하지 않는다.

### 3.2 Production에서는 `새로 그리기`보다 `편집`을 우선한다

승인된 Base 이후 AI Agent의 역할은 다음과 같다.

```text
Reference 분석
+ Base Sprite 분석
        ↓
필요한 방향 차이 판단
        ↓
기존 Character identity를 유지하며 Pixel 편집
        ↓
검증 가능한 결과 생성
```

좋지 않은 요청:

```text
Generate a new samurai in eight directions.
```

권장 의도:

```text
Edit the approved 16x16 base sprite for the requested direction.
Preserve silhouette, proportions, baseline, center and palette constraints.
Do not redesign the character.
```

모델 이름 자체보다 이 작업 경계가 중요하다. 복잡한 방향/실루엣 판단에서는 높은 추론 설정을 사용할 수 있지만, 최종 규격은 Agent 판단이 아니라 Validator가 확인한다.

---

## 4. 8방향 규칙

현재 프로젝트의 Character direction 순서는 `include/game/direction.h`를 따른다.

1. `DOWN`
2. `DOWN_LEFT`
3. `LEFT`
4. `UP_LEFT`
5. `UP`
6. `UP_RIGHT`
7. `RIGHT`
8. `DOWN_RIGHT`

Sprite sheet와 lookup table이 이 순서에 의존하므로, 이미지가 보기 좋다는 이유로 frame index를 임의 재정렬하지 않는다.

좌우 대칭이 가능한 Character는 mirror를 우선 검토한다.

```text
LEFT       <-> RIGHT
DOWN_LEFT  <-> DOWN_RIGHT
UP_LEFT    <-> UP_RIGHT
```

단 다음과 같은 비대칭 정보가 있으면 단순 mirror 후 추가 편집이 필요하다.

- 한쪽에만 장착된 무기 / 방패
- 칼집 위치
- 장식
- 얼굴 흉터
- 방향에 따라 반드시 유지해야 하는 손잡이 / 날 방향

각 방향을 완전히 독립적인 새 그림으로 생성하여 체형이 달라지는 방식을 기본값으로 사용하지 않는다.

---

## 5. Pixel Grid 보호 규칙

Production 단계에서는 해당 Character가 사용하는 frame size를 먼저 확인하고 그 Grid를 고정한다.

현재 Samurai 기준:

```text
16 x 16 per frame
```

작업 중 금지:

- canvas resize로 문제를 숨기기
- bilinear / bicubic resampling
- 작은 Sprite를 고해상도 painting으로 다시 만든 뒤 축소하는 것을 최종 제작법으로 사용
- 방향마다 다른 scale 사용
- 한 방향만 baseline 변경

확대 Preview가 필요하면 nearest-neighbor 확대본을 review용으로만 만든다. 확대본은 production source가 아니다.

---

## 6. Palette 규칙

Palette의 Source of Truth는 `docs/gba-sprite-palette.md`다. 이 문서에서 OBJ palette 정책을 중복 정의하지 않는다.

Character editing 단계의 추가 원칙:

- 같은 Character의 모든 방향은 shared palette를 우선한다.
- 방향이 추가됐다는 이유로 새 palette를 만들지 않는다.
- 승인된 Base의 색으로 표현 가능하면 기존 색을 재사용한다.
- 새 색이 필요하면 Character identity나 가독성 때문에 정말 필요한지 먼저 검토한다.
- 실제 사용 색상 수와 palette entry/signature를 별도로 확인한다.

현재 Samurai helper는 16-entry palette table과 0~15 index만 사용하도록 검증한다. index 0은 transparent chroma key다.

현재 Samurai에서 사용한 구체 색 수를 새 Character의 목표값으로 일반화하지 않는다. **GBA 4bpp 한계와 프로젝트 Palette Harness 안에서 필요한 최소 색을 사용하는 것**이 규칙이다.

---

## 7. Character Identity 보호

방향을 파생할 때 최소한 다음 요소를 비교한다.

- 전체 높이와 몸의 질량감
- helmet / hair / head silhouette
- shoulder width
- torso width
- leg thickness / foot spread
- weapon identity
- 발 baseline
- Sprite center

AI 편집에서 반복적으로 발생할 수 있는 실패:

- 원본보다 마른 체형으로 변함
- 머리 크기가 방향마다 변함
- 어깨 폭이 크게 달라짐
- 무기가 다른 무기로 보임
- LEFT/RIGHT의 실제 방향이 뒤집힘
- 한 방향만 중심이 튐
- 색이 조금씩 늘어나 shared palette가 깨짐

이 항목은 자동 Validator와 시각 Review를 함께 사용한다.

---

## 8. 자동 Validator

가능한 범위에서 production 적용 전에 다음을 자동 검사한다.

```text
frame width / height
frame count
direction order
indexed format
pixel palette-index range
used color count
shared palette
transparent index / chroma key
baseline
frame bounds / center drift
```

Character 실루엣을 수치로 잠글 수 있다면 다음도 Character-specific validator로 둔다.

```text
occupied pixel range
helmet width / height
shoulder width
torso width
leg thickness
foot spread
weapon bounds
```

모든 Character가 Samurai와 같은 수치를 가져야 하는 것은 아니다. **승인된 Base에서 반드시 유지해야 할 특징만 계약으로 만든다.**

생성/정제 script가 있는 경우 결과 PNG/BMP만 수동 편집하지 않고 script와 validator를 함께 갱신한다.

---

## 9. Visual Review

자동 검증만 통과했다고 Character Art가 승인되는 것은 아니다.

최소 두 형태를 권장한다.

```text
8-direction contact / sheet preview
before vs after comparison
```

확인은 다음 질문으로 한다.

- 8방향이 같은 Character로 보이는가?
- 어느 방향을 보고 있는지 즉시 읽히는가?
- 좌우가 잘못 뒤집히지 않았는가?
- 중심과 발 위치가 불필요하게 흔들리지 않는가?
- 무기와 장비의 정체성이 유지되는가?
- 실제 16x16 크기에서도 실루엣을 구분할 수 있는가?

확대본만 보고 승인하지 않는다. 실제 크기도 함께 본다.

---

## 10. Production Acceptance

최종 적용 순서:

```text
Reference / approved Base
        ↓
Pixel editing / direction derivation
        ↓
Validator
        ↓
Review preview
        ↓
Butano asset pipeline
        ↓
clean build
        ↓
mGBA
        ↓
사용자 승인
```

build 성공과 Art 승인을 같은 것으로 취급하지 않는다.

보고할 때 구분한다.

- 자동 규격 검증 결과
- Butano build 결과
- mGBA 시각 확인 결과
- 사용자가 승인하지 않은 남은 항목

---

## 11. sprite-gen PoC에서 얻은 경계

Issue #61에서 `sprite-gen` 설치와 실제 Codex provider generation까지 검증했다.

도구 자체는 정상 동작했다.

확인된 활용 가능 영역:

- existing sheet import
- curation webview
- alpha / frame 정리 보조
- atlas / manifest
- size / palette 검사 보조

하지만 현재 Samurai 16x16 reference로 Walk / Attack generation을 시도했을 때 AI raw는 작은 Pixel Grid 편집이 아니라 큰 새 이미지를 생성했다. Work PoC에서 확인된 예시는 다음과 같다.

```text
Original Samurai : 16x16
Walk raw         : 1774x887
Attack raw       : 1254x1254
```

그 뒤 extractor가 결과를 16x16으로 축소했기 때문에 다음 계약을 보장하지 못했다.

- 승인된 Samurai silhouette 유지
- 원본 Pixel 위치의 최소 변경
- 기존 palette만 사용
- 방향 전체 shared palette 유지
- GBA production asset으로 직접 채택 가능한 결과

따라서 이 프로젝트에서는 `sprite-gen AI generation -> production Character Sprite`를 기본 제작 경로로 채택하지 않는다.

이는 sprite-gen 자체의 실패가 아니라 **일반 이미지 generation 파이프라인과 작은 GBA Pixel editing의 목적 차이**에 따른 결정이다.

---

## 12. 이번 Harness의 범위 밖

아직 다음 규칙은 정의하지 않는다.

- Walk frame 제작법
- Attack body animation
- Hit / recoil animation
- Breathe / idle animation
- animation frame count
- frame timing
- loop 규칙
- animation별 허용 Pixel 변경량

다음 순서로 실제 경험을 먼저 만든다.

```text
현재 승인된 Samurai
        ↓
한 방향 Walk PoC
        ↓
실제 Sprite / build / mGBA 검증
        ↓
8방향 확장 여부 판단
        ↓
성공한 방법만 Animation Harness로 기록
```

검증하지 않은 애니메이션 제작법을 미리 프로젝트 규칙으로 만들지 않는다.

---

## 13. Harness 갱신 원칙

새 Character를 만들면서 반복 가능한 더 좋은 방법이 확인되면 이 문서를 갱신한다.

단 다음을 구분한다.

```text
한 번 성공한 우연한 편집
vs
다른 Character에도 반복 가능한 제작 규칙
```

두 번째 Character에서도 검증된 규칙은 공통 Harness로 승격할 가치가 크다. Character 전용 수치나 스타일은 해당 Character의 생성 script / reference / review 자료에 남긴다.
