# 운영체제 Assignment 2

### 이름 : 이준휘

### 학번 : 2018202046

### 교수 : 최상호 교수님

### 강의 시간 : 금 1, 2


## 1. Introduction

```
해당 과제는 두 가지 모듈로 나누어서 테스트를 진행한다. iotracehooking 모듈에서는
open, close, read, write, lseek 모듈을 hijack하여 ftrace_open, ftrace_close, ftrace_read,
ftrace_write, ftrace_lseek 모듈로 대체한다. 또한 대체한 모듈에서 얻은 정보를 바탕으로
ftracehooking 모듈에서는 프로세스에서 수행한 systemcall의 명령을 tracing한다. 이 때
기존의 system Call 336 번에 ftrace를 미리 할당하여 만들어 두며, 이를 hijack하는 방식으
로 구현한다. 두 모듈은 하나의 헤더를 공유하며 하나의 Makefile로 동시에 생성된다.
```
## 2. Conclusion & Analysis


해당 사진은 linux 커널 파일에서 /arch/x86/entry/syscalls/syscall_64.tbl에 저장되어있는 system
call table에 ftrace를 추가한 모습이다. Ftrace에 336 번을 할당하고 이를 __x64_sys_ftrace함수와
연결시킨다. 해당 프로그램이 window 64-bit에서 수행되기 때문에 syscall_64 파일을 수정하며,
함수명 또한 __x64가 포함되어 있다.


해당 파일 또한 linux 커널 파일에 존재하며, /include/linux/syscalls.h에 저장되어 있다. 해당 파
일은 어떠한 system이든 사용이 가능하기 때문에 __x64가 붙지 않으며, 커널에 만들 ftrace 함
수를 미리 정의해준다.


해당 파일은 linux kernel Directory에서 새롭게 만든 /ftrace/ftrace.c 파일이다. 해당 파일 내에는
SYSCALL_DEFINE1(ftrace, pid_t, pid)로 함수가 선언되어 있다. 위의 함수는 자동으로 ftrace
system call을 만들어주는 메크로로 인자 하나를 받기에 DEFINE1으로 되어있다.


해당 파일은 linux kernel directory에서 ftrace/Makefile로 존재하며, core-y를 통해 해당 파일의
만들 모듈명을 정의해준다.


다음은 커널의 Makefile 중 core-y의 부분을 찾아 ftrace/를 추가함으로써 해당 모듈을 같이 빌
드를 수행하도록 도움을 준다. 위의 과정을 모두 마치고 $sudo make -j4 -> $sudo make
modules_install -> $sudo make install 순으로 수행하여 현재 커널에 적용시켰다.


해당 파일은 ftracehooking.c와 iotracehooking.c가 사용할 header로 header file과 함께 ftrace를
사용할 시 주고받을 데이터가 구조체 형식으로 정의되어 있다.


위의 파일는 ftracehooking.c로 ftrace를 hijacking하는 Function이 정의되어 있다. Ftrace_data
변수는 EXPORT_DEFINE()함수를 통해 전역 변수로 사용된다. 기존의 System call을 저장할 함수
가 존재하며 syscall_table에 kallsyms_lookup_name(“sys_call_table”)의 값을 대입하여 system call
의 table을 가져와 이를 ftrace system call을 hijacking한다. 이 때 table에는 쓰기 권한이 없음으
로 임시로 쓰기 권한을 부여한 뒤 바꾼 후 권한을 반환한다. Hijack을 수행하는 함수에서는
register들의 값인 struct pt_regs *reg를 인자로 받는다. 우선 기존 함수를 그대로 수행하여 pid
값을 받은 후 해당 pid값에 따라 다음 명령을 수행한다. 0 이 아닐 경우 해당 pid에 대한 ftrace
를 수행하기 위해 메모리를 초기화 하고 task_stuct에서 pid 값을 통해 현재에 해당하는
task_struct를 찾아 해당 구조체에서 이름을 가져온다. 만약 pid가 0 일 경우에는 기존에 ftrace
를 수행한 모든 데이터를 출력한다.


해당 파일은 iotracehooking.c 파일을 나타낸다. 해당 파일도 이전과 마찬가지로 System Call 번
호와 기존 함수를 저장할 함수, syscall_table이 선언되어 있으며, ftracehooking.c에서 선언한 전
역 변수 extern struct ftrace_data ft_data가 존재한다. 모듈에서도 비슷하게 syscall_table을 조작
하게 된다. Open을 hijack하는 함수에서는 copy_from_user를 통해 user space의 파일명 데이터
에 해당하는 레지스터인 regs->di를 kernel space로 복사해온다. Read/Write를 hijack하는 함수
에서는 ssize_t size에 기존 system call 함수를 수행한 결과인 크기를 받아 이를 xx_byte에 늘려
주고 반환하는 형식을 취한다. 이외의 함수도 마찬가지로 기존의 system call을 거의 그대로 사
용하며, 반환 전에 해당하는 count를 늘려준다.


해당 파일은 ftracehooking 모듈과 iotracehooking 모듈을 한 번에 만들어주는 모듈이다. Obj-m
에는 만들 모듈들의 이름이 들어가며, KDR에는 module builder의 위치, PWD에는 현재 경로가
들어간다. CFLAG에는 EXPORT_DEFINE을 사용할 시 필요한 flag인 - DEXPORT_SYMTAB이 들어가
게 된다. 해당 파일에서는 과제에서 준 test.c도 같이 컴파일 되도록 임시로 만들었다.


해당 파일은 기존에 주어진 파일인 test.c 파일과 해당 파일에서 사용하는 abc.txt 파일이 다음
과 같이 구성되어 있음을 보인다. 해당 test.c에서는 ftrace를 활성화시킨 후, open을 통해
abc.txt를 연다. 이후 4 번의 반복동안 read 5 byte 1회, write 5byte 1회, lseek 2회를 수행한다. 반
복문을 탈출한 후 lseek 1회와 write 6byte 1 회를 추가로 수행한 후 close를 수행한다. 그 후
trace를 종료한다. 위의 결과를 예상했을 때 open 1회, close 1회, read 20byte 4회, write 26byte
5 회, lseek 9회를 수행해야 한다.


위의 동작은 기존의 system call을 사용하여 ftrace를 진행하지 않고 파일을 수행시킨 결과를 보
인다. Cat을 통해 abc.txt를 출력하였을 때 다음과 같은 결과를 보이고 dmesg에는 아무 결과가
찍히지 않는다.


위의 결과는 모듈을 적용하여 ./test 파일을 수행한 후 결과를 보인다. 해당 결과에서 dmesg의
마지막에 기존에 없던 출력이 생겼으며, 이 곳에 실행 파일명, 연 파일명, 읽기&쓰기 byte 수,
각 함수 사용 횟수가 기록되어 있다. 해당 결과가 test.c의 예상 결과와 일치하는 것을 보인다.
이를 통해 해당 모듈이 정상적으로 구현되었음을 알 수 있다.

## 3. Consideration

```
해당 과제를 통해 Linux에서 System call이 어떠한 과정을 거치며 호출되는지 순차적으로
```

```
알 수 있었다. 처음 접근할 때 ftrace hooking을 수행할 때 SYSCALL_DEFINEx 메크로를
사용하였다가 해당 작업은 메크로를 생성하는 것이 아니라 메크로를 가져오는 것이기에
부적절하다는 것을 알게 되었다. 또한 파일에서의 해당 함수를 찾아보았을 때와 다르게
실제로는 함수로의 직접 접근으로 인한 오류를 방지하기 위해 struct pt_reg* 인자를 통해
값을 전달한 후 해당 함수가 다시 실제 함수로 mapping된다는 것을 알 수 있었다. 또한
해당 pt_reg에는 각 system call마다 사용하는 reg의 위치가 다르다는 것을 알 수 있었다.
마지막으로 전역 변수를 사용하기 위해 EXPORT_DEFINE() 사용해야 한다는 사실을 공부
할 수 있던 과제였다.
```
## 4. Reference

#### - 강의자료만을 참고


