# Project Structure

## Header / Source Layers

```text
src/main.c
|-- include/fs.h
|   `-- include/minios.h
|-- include/shell.h
|   `-- include/minios.h
`-- include/storage.h
    `-- include/fs.h
        `-- include/minios.h

src/minios.c
`-- include/fs.h
    `-- include/minios.h

src/fs.c
|-- include/fs.h
|   `-- include/minios.h
`-- include/storage.h
    `-- include/fs.h
        `-- include/minios.h

src/storage.c
`-- include/storage.h
    `-- include/fs.h
        `-- include/minios.h

src/shell.c
|-- include/shell.h
|   `-- include/minios.h
|-- include/commands/basic.h
|   `-- include/minios.h
`-- include/commands/even.h
    `-- include/minios.h

src/threading.c
`-- include/threading.h
    `-- include/minios.h

src/commands/ls.c
src/commands/cd.c
src/commands/mkdir.c
src/commands/cat.c
src/commands/pwd.c
`-- include/commands/basic.h
    `-- include/minios.h

src/commands/chmod.c
src/commands/clear.c
src/commands/cp.c
src/commands/rmdir.c
`-- include/commands/even.h
    `-- include/minios.h
```

이 문서는 구현을 시작할 때 어느 파일을 수정하면 되는지 빠르게 찾기 위한 안내입니다.

## Core

- `src/main.c`: Mini OS 시작점
- `src/minios.c`: 전체 컨텍스트 초기화/종료
- `src/shell.c`: 입력 루프, 명령어 등록, 명령어 디스패치
- `src/fs.c`: 링크 트리 기반 가상 파일 시스템 공용 함수
- `src/storage.c`: 파일 입출력을 통한 트리 저장/복원
- `src/threading.c`: 다중 작업 발생 시 사용할 스레딩 진입점

## Required Commands

- `src/commands/ls.c`: `ls`, `ls -a`, `ls -l`, `ls -al`
- `src/commands/cd.c`: `cd`, `.`, `..`
- `src/commands/mkdir.c`: `mkdir`, `mkdir -p`, 다중 폴더 생성
- `src/commands/cat.c`: `cat`, `cat -n`, `cat >`
- `src/commands/pwd.c`: `pwd`

## Even-Team Commands

- `src/commands/chmod.c`: 권한 변경
- `src/commands/clear.c`: 화면 지우기
- `src/commands/cp.c`: 복사
- `src/commands/rmdir.c`: `rmdir`, `rmdir -p`

## Suggested Implementation Order

1. `fs.c`의 트리 탐색/추가/삭제 공용 함수
2. `storage.c`의 저장/복원 포맷
3. `pwd`, `cd`, `ls`
4. `mkdir`, `cat`
5. `chmod`, `cp`, `rmdir`, `clear`
6. `threading.c`를 이용한 동시 작업 처리
