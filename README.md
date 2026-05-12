# Mini OS Project

Linux Shell 환경에서 동작하는 가상 파일 시스템 시뮬레이터입니다.

이 저장소는 기능 구현 전 단계의 구조 뼈대만 포함합니다. 명령어별 세부 로직은 각 `src/commands/*.c` 파일의 `TODO` 영역에 작성하면 됩니다.

## 구현 대상

필수 기능:

- `ls`: `-a`, `-l`, `-al`
- `cd`: `.`, `..`
- `mkdir`: `-p`, 다중 폴더 생성 시 멀티스레딩 적용
- `cat`: `>`, `-n`
- `pwd`

짝수 조 기능:

- `chmod`
- `clear`
- `cp`
- `rmdir`: `-p`

## 구조

- `include/`: 공용 헤더
- `include/commands/`: 명령어별 함수 선언
- `src/`: Mini OS 초기화, 가상 파일 시스템, 셸 루프, 저장/로드, 스레딩
- `src/commands/`: 명령어별 구현 파일

## 빌드

```sh
make
```

## 실행

```sh
make run
```

현재 명령어들은 모두 스텁이며, 호출 시 `TODO` 메시지를 출력합니다.
