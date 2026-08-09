# GBA Sprite Palette Harness

## 목적

GBA의 제한된 OBJ palette 자원을 안정적으로 사용하면서 게임 디자인과 그래픽 표현의 자유도를 가능한 한 유지한다.

새로운 Sprite Asset은 개별적으로 palette를 소비하는 것을 기본값으로 하지 않는다. 기존 palette 재사용 가능성을 먼저 검토한다.

## 1. 기본 형식

일반 Sprite Asset은 기본적으로 4bpp를 사용한다.

- 4bpp Sprite는 16-entry palette를 사용하며, index 0은 투명색이므로 일반적으로 가시색은 최대 15개이다.
- 8bpp 사용은 예외로 취급한다.
- 8bpp가 필요한 경우 기술적 이유를 먼저 검토한다.

아래의 16-bank budget과 권장 Peak 기준은 4bpp OBJ palette bank 기준이다. 8bpp는 256-entry OBJ palette를 사용하므로 4bpp bank 하나와 동일하게 계산하지 않고, OBJ palette 전체와 4bpp Sprite의 공존 영향 및 Butano 변환·할당 방식을 별도로 검토한다.

## 2. Palette Group

Sprite Asset은 다음 논리 그룹 중 하나에 속하도록 관리한다.

- `PLAYER`
- `ENEMY`
- `EFFECT`
- `STATUS_UI`
- `DEBUG`
- `SPECIAL`

이 그룹은 GBA 하드웨어 palette slot 번호를 의미하지 않는다. 예를 들어 `PLAYER = hardware palette 0`과 같은 고정 slot 배정 정책을 만들지 않는다.

Palette Group의 목적은 같은 용도의 Asset이 가능한 한 동일한 16-entry palette를 공유하도록 관리하는 것이다.

Palette Group은 분류와 공유 가능성 검토 단위다. 그룹 전체가 반드시 하나의 Palette만 사용해야 한다는 의미가 아니며, 안전하다면 서로 다른 Group 사이의 Palette 공유도 허용한다.

## 3. 신규 Asset 생성 규칙

새 Sprite Asset을 만들기 전에 반드시 다음 순서로 판단한다.

1. 이 Asset은 어느 Palette Group에 속하는가?
2. 기존 Palette로 표현 가능한가?
3. 기존 Palette의 시각적 품질을 해치지 않고 공유 가능한가?
4. 공유가 어렵다면 새로운 Palette가 게임 디자인상 필요한가?
5. 새 Palette 추가 후 peak OBJ palette 사용량은 안전한가?

새 Palette 생성 자체를 금지하지 않는다. 단, 새 Palette는 16개뿐인 OBJ palette budget 중 하나를 소비하는 설계 결정으로 취급한다.

## 4. Palette 공유 우선 원칙

다음 Asset은 특히 Palette 공유를 우선 검토한다.

- 같은 캐릭터의 방향별 Sprite
- 같은 캐릭터의 Animation Frame
- 같은 계열의 Enemy
- 같은 Stage의 일반 Enemy
- Slash / Hit / Spark 등의 작은 Effect
- Status Icon
- Debug Sprite

방향 또는 Frame이 다르다는 이유만으로 별도 Palette를 생성하지 않는다.

## 5. 게임 디자인 우선 원칙

Palette 절약 때문에 Gameplay에서 중요한 시각적 정보를 훼손하지 않는다.

다음 경우에는 새로운 Palette 사용을 허용할 수 있다.

- 적 종류를 명확히 구분해야 하는 경우
- Boss의 시각적 정체성이 필요한 경우
- 장비 또는 상태 변화가 Gameplay 정보인 경우
- Stage Theme 구분에 색 차이가 중요한 경우

이 경우 Palette Budget을 확인하고 사용한다.

## 6. Palette Swap 활용

Sprite 모양은 동일하고 색만 달라지는 경우 새 Sprite Set 제작보다 Palette Swap을 우선 검토한다.

예:

- 일반 적 / 강화 적
- 속성 적
- 독 / 화염 / 빙결 상태
- 플레이어 피격 / 무적 표현
- 장비 색상 Variant

단, 공유 Palette 자체를 runtime에서 변경하면 그 Palette를 공유하는 모든 Sprite가 영향을 받을 수 있으므로 영향 범위를 먼저 확인한다.

## 7. OBJ Palette Budget

GBA OBJ 4bpp palette의 하드웨어 최대치는 16 bank이다. 하드웨어 최대치를 정상적인 프로젝트 목표로 사용하지 않는다.

프로젝트 기준:

- 절대 최대: 16
- 권장 Peak: 12~14 이하
- 최소 2 Palette 이상의 여유 확보를 목표로 한다.

`12~14`는 하드웨어 규칙이 아니라 본 프로젝트의 안전 마진 정책이다. 신규 Sprite 또는 Effect 추가로 peak 사용량이 증가하면 반드시 Palette 영향을 확인한다.

권장 Peak는 디자인 승인을 막는 고정 기준이 아니다. 12 이하를 우선 목표로 하고 필요하면 14까지 허용하되, 15 이상은 예외 사유와 영향을 기록한다.

Peak는 한 실행 장면 또는 전환 구간에서 동시에 resident인 서로 다른 OBJ palette의 최대치다. Gameplay Sprite뿐 아니라 Effect, Status UI, Debug Sprite와 전환 중 일시적으로 겹치는 Palette도 포함한다. 정적 Asset의 Palette Signature 목록만으로 runtime Peak 검증을 대체하지 않는다.

## 8. DEBUG 정책

Debug용 Sprite는 가능한 한 하나의 공용 4bpp Palette를 공유한다. Debug 기능 때문에 Gameplay Asset이 사용할 Palette Budget을 불필요하게 소비하지 않는다.

## 9. Asset 내부 Palette 주의사항

현재 프로젝트의 indexed BMP Asset은 이미지와 Palette 정보를 함께 포함할 수 있다.

따라서 겉보기 RGB 결과가 동일하더라도 BMP 내부의 Palette Entry 배열이 다르면 별도 Palette로 취급될 수 있다. 특히 실제 픽셀에서 사용하지 않는 Palette Entry 차이도 Palette Signature 차이를 발생시킬 수 있으므로 공유 대상 Asset은 원본 BMP에서 변환에 관련된 Palette Entry 전체와 Butano 변환 결과를 기준으로 검증한다.

## 10. 자동 검증 원칙

Sprite Asset 생성 또는 변경 시 가능한 범위에서 다음을 확인한다.

- indexed 형식이며 원칙상 4bpp인지, 8bpp라면 예외 사유와 자원 영향을 기록했는지
- 실제 사용 색상 수
- Palette Entry / Signature
- 기존 공용 Palette와 동일 여부
- 불필요한 신규 Palette 발생 여부
- 최대 동시 OBJ Palette 사용량

겉보기 이미지 비교만으로 Palette 공유 여부를 판단하지 않는다.

현재 자동 검사 도구가 없다면 대규모 검사 시스템을 한 작업에 함께 구현하지 않는다. 향후 자동화할 수 있도록 정책과 작업 결과를 명확하게 남긴다.

## 11. 기존 Asset 보호

기존 Asset을 향후 Palette 정책에 맞춰 정리할 때는 다음을 보존한다.

- 화면의 RGBA Pixel 결과
- Animation Frame
- 방향 Index
- 기능적 외형

전체 Asset을 일괄 수정하지 않는다. Palette Group 단위로 조사 → 계획 → 변경 → 검증 순서로 수행한다.

Issue #32에서는 기존 Asset을 변경하지 않는다.

## 12. 예외 처리

새로운 Palette가 필요하면 허용한다. 단, 다음 내용을 작업 결과에 기록한다.

- 대상 Asset
- Palette Group
- 기존 Palette를 공유할 수 없는 이유
- 예상 Peak 사용량
- 새로운 Palette가 필요한 Gameplay / Art 이유

## 13. Harness 갱신 원칙

Palette 관련 새로운 장애나 설계 경험이 생기면 이 Harness를 갱신한다.

Issue #31에서 얻은 교훈:

- Resource Leak과 Peak Resource Exhaustion을 구분한다.
- Palette Reference가 정상적으로 해제되어도 최대 동시 사용량 때문에 crash가 발생할 수 있다.
- 동일하게 보이는 Asset도 실제 Palette Signature가 다르면 별도 Palette를 소비할 수 있다.
- 신규 Palette 추가는 Asset 하나 추가가 아니라 제한된 OBJ Palette Budget을 사용하는 결정이다.

## 다음 단계

기존 Asset에 이 정책을 적용하기 전에 별도 작업으로 `Existing Sprite Palette Inventory & Classification`을 수행한다. 실제 Palette 종류, Asset별 사용 관계, 기존 공유 상태와 불필요한 분리를 조사한 뒤 변경 여부를 결정한다.
