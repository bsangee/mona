/*!
  \file  syscalls.cpp
  \brief syscalls

  Copyright (c) 2003 HigePon
  WITHOUT ANY WARRANTY

  \author  HigePon
  \version $Revision$
  \date   create:2003/03/22 update:$Date$
*/

#include<syscalls.h>
#include<global.h>
#include<io.h>

extern "C" char pos_x;
extern "C" char pos_y;

extern "C"     void arch_set_stack_view();

void syscall_entrance() {

    int x, y;
    dword eflags;

    switch(g_current_process->ebx) {

    case SYSTEM_CALL_PROCESS_SLEEP:

        g_process_manager->sleep(g_current_process, g_current_process->esi);

        /* return code */
        g_current_process->eax = 0x12345678;

        break;

    case SYSTEM_CALL_HEAVEY:

        eflags = get_eflags();
        disableInterrupt();

        x = pos_x;
        y = pos_y;

        pos_x = 1, pos_y = 2;

        g_console->printf("heavy start\n");

        pos_x = x;
        pos_y = y;
        set_eflags(eflags);

        enableInterrupt();

        for (dword i = 0; i < 0xffff; i++) {

            i++;
            i--;
            i++;
            i--;
        }

        eflags = get_eflags();
        disableInterrupt();

        x = pos_x;
        y = pos_y;

        pos_x = 2, pos_y = 4;

        g_console->printf("heavy end\n");

        pos_x = x;
        pos_y = y;


        /* return code */
        g_current_process->eax = 0x12345678;

        set_eflags(eflags);

        break;

    default:
        g_console->printf("syscall:default");
        break;
    }
    return;
}

int syscall_sleep(dword tick) {

    int result;

    asm volatile("movl $%c2, %%ebx \n"
                 "movl %1  , %%esi \n"
                 "int  $0x80       \n"
                 "movl %%eax, %0   \n"
                 :"=m"(result)
                 :"m"(tick), "g"(SYSTEM_CALL_PROCESS_SLEEP)
                 );

    return result;
}

int syscall_heavy() {

    int result;

    asm volatile("movl $%c1, %%ebx \n"
                 "int  $0x80       \n"
                 "movl %%eax, %0   \n"
                 :"=m"(result)
                 :"g"(SYSTEM_CALL_HEAVEY)
                 );

    return result;
}
