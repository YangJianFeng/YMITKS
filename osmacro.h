/*开关interrupt*/
#define OS_OFF_INTERRUPT()  asm ("cli")
#define OS_ON_INTERRUPT()   asm ("sei")

/*嵌套中断，在中断使全局中断使能置1*/
#define OS_I_INTERRUPT() asm("sei")

/*当前栈地址*/
#if !defined(OS_STK)
	#define OS_STK char
#endif
#define OS_SP ((OS_STK *) (* (int *) 0x05d | * (int *) 0x5e << 8))

/*堆栈指针*/
#define OS_SP_L (* (char *) 0x5d)
#define OS_SP_H (* (char *) 0x5e)


/*任务控制模块扩展*/
#define OS_TASK_CREATE_EXT_EN 0

/*删除任务返回值常量*/
#define OS_DEL_TASK_NOT_EXIST 12  /*删除的任务不存在*/
#define OS_DEL_TASK_YES       11  /*已被请求删除*/
#define OS_DEL_TASK_NO        10  /*未被请求删除*/

/*任务统计时间*/
#define OS_TASK_STAT_EN   0

/*延时时钟节拍常量*/
#define OS_TICKS_PER_SEC  1

/*最大任务数 与 最后任务数*/
#define OS_MAX_TASKS     10
#define OS_LAST_TASKS     9

/*任务状态常量*/
#define OS_STAT_DORMANT   0x01	/*休眠态 */
#define OS_STAT_READY     0x02	/*就绪态 */
#define OS_STAT_RUNNING   0x04	/*运行态 */
#define OS_STAT_SUSPEND   0x08	/*挂起态 */
#define OS_STAT_INTERRUPT 0x10	/*被中断态 */

/*最低优先级常量*/
#define OS_LOWEST_PRIO_A  63
#define OS_LOWEST_PRIO_B  62
#define OS_LOWEST_PRIO_C  61
#define OS_LOWEST_PRIO_D  60

/*空任务常量*/
#define OS_IDLE_PRIO      63

/*堆空间分配大小*/
#define OS_ALLOC_SIZE     36

/*空指针*/
#ifndef NULL
#define NULL ((void *) 0)
#endif

/*临时的非空指针*/
#define NO_NULL ((void *) 1)

/*调试*/
#define BUGNO 0

/*调试用端口*/
#define PBB (*(char*)0x38)
#define PAA (*(char*)0x3b)

/*真假常量*/
#define TRUE  1
#define FALSE 0

/*返回常量*/
#define OS_RIGHT  0
#define OS_ERROR -1

/*消息、事件、同步等*/
#define OS_Q_EN       0
#define OS_MAX_QS     0
#define OS_MBOX_EN    0
#define OS_SEM_EN     0

/*版本号*/
#define OS_VERSION 1

/*函数摸拟中断*/
#define OS_ISRSW() os_sched_sw()

/*空操作*/
#define OS_NOP() os_nop_function()

/*任务堆栈大小*/
#define OS_STACK_MAX 80

/*保存寄存器宏*/
#define PUSH_REGISTER "push r0\n"       \
                      "push r1\n"       \
                      "push r2\n"       \
                      "push r3\n"       \
                      "push r4\n"       \
                      "push r5\n"       \
                      "push r6\n"       \
                      "push r7\n"       \
                      "push r8\n"       \
                      "push r9\n"       \
                      "push r10\n"      \
                      "push r11\n"      \
                      "push r12\n"      \
                      "push r13\n"      \
                      "push r14\n"      \
                      "push r15\n"      \
                      "push r16\n"      \
                      "push r17\n"      \
                      "push r18\n"      \
                      "push r19\n"      \
                      "push r20\n"      \
                      "push r21\n"      \
                      "push r22\n"      \
                      "push r23\n"      \
                      "push r24\n"      \
                      "push r25\n"      \
                      "push r26\n"      \
                      "push r27\n"      \
                      "push r28\n"      \
                      "push r29\n"      \
                      "push r30\n"      \
                      "push r31\n"      \
		      "in   r16, 0x3f\n" \
		      "push r16\n"

#define POP_REGISTER  "pop r16\n"      \
                      "out 0x3f,r16\n" \
                      "pop r31\n"      \
                      "pop r30\n"      \
                      "pop r29\n"      \
                      "pop r28\n"      \
                      "pop r27\n"      \
                      "pop r26\n"      \
                      "pop r25\n"      \
                      "pop r24\n"      \
                      "pop r23\n"      \
                      "pop r22\n"      \
                      "pop r21\n"      \
                      "pop r20\n"      \
                      "pop r19\n"      \
                      "pop r18\n"      \
                      "pop r17\n"      \
                      "pop r16\n"      \
                      "pop r15\n"      \
                      "pop r14\n"      \
                      "pop r13\n"      \
                      "pop r12\n"      \
                      "pop r11\n"      \
                      "pop r10\n"      \
                      "pop r9\n"       \
                      "pop r8\n"       \
                      "pop r7\n"       \
                      "pop r6\n"       \
                      "pop r5\n"       \
                      "pop r4\n"       \
                      "pop r3\n"       \
                      "pop r2\n"       \
                      "pop r1\n"       \
                      "pop r0\n"       \
		      "reti\n"
