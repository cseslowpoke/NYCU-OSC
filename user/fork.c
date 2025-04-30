#include "delay.h"
#include "stdio.h"
#include "unistd.h"
#include "utils.h"

__attribute__((section(".start"))) void fork_test() {
  printf("Fork Test, pid %d\r\n", get_pid());
  int cnt = 1;
  int ret = 0;
  if ((ret = fork()) == 0) { // child
    long long cur_sp;
    asm volatile("mov %0, sp" : "=r"(cur_sp));

    printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\r\n", get_pid(), cnt,
           &cnt, cur_sp);
    ++cnt;

    if ((ret = fork()) != 0) {
      asm volatile("mov %0, sp" : "=r"(cur_sp));
      printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\r\n", get_pid(),
             cnt, &cnt, cur_sp);
      ++cnt;
    } else {
      // while (cnt < 5) {
      asm volatile("mov %0, sp" : "=r"(cur_sp));
      printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\r\n", get_pid(),
             cnt, &cnt, cur_sp);
      // delay(1000000);
      ++cnt;
      // }
    }
    exit();
  } else {
    printf("parent here, pid %d, child %d\r\n", get_pid(), ret);
    exit();
  }
  exit();
}
