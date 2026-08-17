# Elite Enemy HP HUD Reference

이 문서는 Elite / Boss HP HUD의 시각적 방향을 기록하기 위한 reference note다.

## 핵심 UX 규칙

- 일반 Enemy는 HP를 화면에 표시하지 않는다.
- Elite / Boss 이상만 고정 HUD 형태로 HP를 표시한다.
- Enemy 머리 위 HP bar는 사용하지 않는다.
- Player HUD와 Elite/Boss HUD는 화면의 반대쪽 가장자리에 배치한다.
  - Player HUD가 아래쪽이면 Elite/Boss HUD는 위쪽.
  - Player HUD가 위쪽이면 Elite/Boss HUD는 아래쪽.
- HUD 위치는 Actor의 World Y가 아니라 UI anchor 기준으로 결정한다.
- Elite/Boss HUD는 이름 + 긴 가로 HP bar를 기본 형태로 한다.
- 고전 벨트스크롤 액션 게임의 고정 적 HP HUD 개념을 참고하되, 현재 게임의 GBA 해상도와 UI 밀도에 맞게 단순화한다.

## 현재 선택 시안

ChatGPT 대화에서 생성한 `엘리트_적_hp_hud_비교안.png` 중 **BOTTOM HUD 버전의 시각적 방향을 우선 reference**로 사용한다.

단, 실제 게임에서는 고정 Bottom이 아니라 Player HUD의 반대쪽이라는 배치 규칙을 적용한다.

실제 이미지 파일은 GitHub Issue 또는 향후 asset/design reference 경로에 별도 첨부한다.
