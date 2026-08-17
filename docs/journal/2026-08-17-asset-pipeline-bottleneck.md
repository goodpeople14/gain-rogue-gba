# 개인 개발자의 의외의 병목 — 코딩보다 Asset이 어렵다

관련 이슈: #60

## 시작은 단순했다

16×16 크기의 8방향 Samurai 캐릭터가 필요했다. AI 이미지 생성으로 마음에 드는 디자인을 얻는 일 자체는 생각보다 빨랐다.

하지만 곧 중요한 차이를 알게 됐다.

> 픽셀아트처럼 보이는 그림과 GBA에서 바로 사용할 수 있는 production Sprite Asset은 전혀 다른 결과물이다.

실제로 게임에 넣으려면 단순히 "16×16 느낌"이 아니라 다음 조건을 만족해야 했다.

- 방향당 정확히 16×16 pixels
- 8방향
- GBA BPP4
- 투명색 포함 16색 이하
- 모든 방향이 하나의 palette 공유
- 동일한 체형과 중심점
- 방향별 일관된 silhouette
- 올바른 좌우 방향
- Butano asset pipeline 통과

## AI 이미지 생성에서 반복된 문제

처음에는 AI에게 곧바로 16×16 8방향 캐릭터를 만들어 달라고 했다. 화면에서 보기에는 그럴듯했지만 실제 Asset으로 검사하면 문제가 반복됐다.

- 실제 프레임 크기가 16×16이 아니었다.
- 방향마다 체형과 비율이 달라졌다.
- 기준 캐릭터의 묵직한 체형이 다른 방향에서는 가늘어졌다.
- LEFT / RIGHT가 의도와 반대로 나오기도 했다.
- 픽셀처럼 보이지만 실제 pixel grid가 깔끔하지 않았다.
- 여러 방향이 동일 palette로 안정적으로 정리되지 않았다.

즉 AI가 잘못 그렸다기보다, **Concept 생성과 production asset 제작을 같은 문제로 취급한 것이 잘못된 접근**이었다.

## Asset 제작을 파이프라인으로 보기 시작했다

이후 작업을 단계로 분리했다.

```text
Concept / Reference
        ↓
기준 Sprite 설계
        ↓
실제 16×16 Pixel Grid
        ↓
Palette 제한
        ↓
8방향 파생
        ↓
Sprite Sheet Packing
        ↓
자동 Asset 검증
        ↓
Butano Build
        ↓
mGBA 실제 확인
```

이렇게 나누니 어느 단계에서 무엇을 검증해야 하는지가 명확해졌다.

AI는 Concept과 후보 생성에 강하고, pixel editor는 실제 grid 정리에 강하며, Codex는 규격 자동 검증에 강하다. 하나의 도구가 전체 과정을 모두 해결할 필요는 없다.

## 8방향을 한 번에 만들지 않는다

가장 큰 시행착오 중 하나는 처음부터 8방향 전체를 동시에 완성하려 한 것이었다.

앞으로는 다음 순서를 기본으로 한다.

```text
대표 방향 1개 제작
→ 체형 / silhouette / 중심점 확정
→ 승인된 기준 Sprite 고정
→ LEFT / RIGHT는 가능한 mirror 활용
→ 나머지 방향을 기준 Sprite에서 파생
→ 전체 시트 검증
```

기준 Sprite가 확정되기 전에 8개 방향을 생성하면 잘못된 체형을 8번 수정하게 된다.

## 사람이 볼 것과 자동으로 볼 것을 나눈다

사람이 봐야 하는 부분:

- 캐릭터가 멋있는가
- 원래 디자인과 같은 인물처럼 보이는가
- 방향별 silhouette가 자연스러운가
- 움직였을 때 중심이 흔들리지 않는가

자동으로 검사할 수 있는 부분:

```text
frame width        = 16
frame height       = 16
frame count        = 8
palette entries    <= 16
BPP                = 4
shared palette     = true
transparent index  = fixed
```

Asset 작업에서도 게임 코드의 TDD와 비슷한 원칙을 적용할 수 있었다.

> 감각은 사람이 승인하고, 규격은 자동으로 검사한다.

## 도구를 역할별로 나눈다

이슈에서 검토한 후보 도구는 다음과 같다.

### SpriteGen

AI 결과를 Sprite 제작 과정으로 연결하는 후보 도구다. 실제 프로젝트 적용 가능성은 별도 #61에서 검토한다.

### Lospec

제한된 Pixel Art palette를 선택하고 기준으로 삼는 데 사용할 수 있다.

### Piskel / LibreSprite

픽셀 단위 수정, mirror, frame 편집, Sprite Sheet 정리에 적합한 편집 도구 후보로 봤다.

### Codex

크기, frame 수, palette, BPP, 투명 index 같은 반복 검증을 자동화하는 역할로 사용한다.

### Butano + mGBA

최종적으로 Asset이 실제 GBA pipeline을 통과하고 ROM에서 정상적으로 보이는지 확인하는 단계다.

## 현재 프로젝트에서 생각한 현실적인 조합

```text
ChatGPT / Image AI
        ↓
Concept / Reference

Sprite 제작 도구
        ↓
실제 Pixel Grid 후보

Palette 도구
        ↓
16색 제한 정리

Pixel Editor
        ↓
Mirror / 정렬 / 미세 수정

Codex
        ↓
규격 자동 검증

Butano
        ↓
Production Asset 변환

mGBA
        ↓
실제 품질 확인
```

핵심은 하나의 AI에게 모든 단계를 맡기지 않는 것이다.

## 개인 개발에서 Asset은 독립적인 개발 영역이다

팀이라면 Programmer, Character Artist, Animator, Technical Artist가 나눠 맡을 수 있는 일을 개인 개발자는 혼자 해결해야 한다.

그래서 개인 GBA 게임 개발에서는 코드 외에도 다음이 실제 개발 업무가 된다.

- Asset 탐색
- Concept 제작
- Sprite 규격화
- Palette 관리
- Animation
- Sprite sheet 정리
- 빌드 변환
- Hardware 제약 검증

처음에는 코딩이 가장 큰 장벽일 것이라고 생각하기 쉽지만 AI와 Codex를 이용하니 코드 구현 속도는 상당히 빨라졌다. 반대로 사람이 보기에 일관되고 동시에 GBA 규격을 만족하는 Asset을 만드는 과정은 예상보다 많은 반복 작업을 요구했다.

## 얻은 교훈

1. AI 이미지 생성과 production Sprite 제작을 같은 작업으로 보지 않는다.
2. 대표 방향 하나를 먼저 완성한 뒤 나머지 방향을 파생한다.
3. 대칭 가능한 방향은 새로 생성하기보다 mirror를 우선 검토한다.
4. Palette와 frame 규격은 눈이 아니라 자동 검증으로 관리한다.
5. Asset도 Concept → 제작 → 정제 → 검증의 별도 파이프라인으로 관리한다.
6. GBA에서는 보기 좋은 이미지보다 실제 제약을 통과한 Asset이 최종 결과물이다.

> AI는 개인 개발자의 Artist를 완전히 대체하기보다, 혼자서는 시작하기 어려웠던 Asset 제작 과정을 시작할 수 있게 해준다. 그 결과를 실제 게임 Asset으로 만드는 과정은 여전히 별도의 기술 작업이다.
