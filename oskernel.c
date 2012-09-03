#ifndef DATA_TYPE
	#include "datatype.h"
#endif

#ifndef OS_TASK_CREATE_EXT_EN
	#include "osmacro.h"
#endif

#ifndef OS_BLOCKS
	#include "osblocks.h"
#endif

/***********************************************
                  任务调度
 ***********************************************/
void
os_task_sched(void)
{
	OS_OFF_INTERRUPT();
	if (!(vie_os_isr_number | vie_os_locksched)) {	/*没有中断嵌套和没有锁定任务调度 */
		/*求出最高任务 */
		vie_os_isrexit = array_cn_un_taskmap[vie_os_ready];
		vie_os_prio_high = (INT8U) (vie_os_isrexit << 3)
		    + array_cn_un_taskmap[array_os_tbl[vie_os_isrexit]];

		/*最高任务不是当前任务，切换最高任务 */
		if (vie_os_prio_high != vie_os_prio_cur) {
			vie_os_prio_cur = vie_os_prio_high;
			p_task_high = p_task_array[vie_os_prio_cur];
			OS_ISRSW();	/*调用宏来实现任务转换 */
		}
	}
	OS_ON_INTERRUPT();
}

/*任务调度锁*/
void
os_ched_lock(void)
{
	if (vie_os_locksched == 0xff)
		return;
	OS_OFF_INTERRUPT();
	++vie_os_locksched;
	OS_ON_INTERRUPT();
}

void
os_ched_unlock(void)
{
	if (vie_os_locksched == 0)
		return;
	OS_OFF_INTERRUPT();
	if (--vie_os_locksched == 0) {
		OS_ON_INTERRUPT();
		os_task_sched();
	}
	OS_ON_INTERRUPT();
}

/*中断嵌套层数加1*/
/*用于中断服务函中调用*/
void
os_isr_add(void)
{
	OS_OFF_INTERRUPT();
	++vie_os_isrnumber;
	OS_ON_INTERRUPT();
}

/*脱离中断服务*/
/*中断嵌套层数减1*/
/*如有嵌套数为0，查看有没有更高的任务就绪*/
void
os_isr_exit()
{
	OS_OFF_INTERRUPT();

	/*如果嵌套中断为0，任务调度没有上锁，则计算最高任务 */
	/*代码与os_task_sched()相差不大，但有区别 */
	if ((--vie_os_isrnumber | vie_os_locksched) == 0) {
		vie_os_isrexit = array_cn_un_taskmap[vie_os_ready];
		vie_os_prio_high = (INT8U) (vie_os_isrexit << 3)
		    + array_cn_un_taskmap[array_os_tbl[vie_os_isrexit]];

#if 0
		PBB = vie_os_ready;
		PAA = array_os_tbl[vie_os_isrexit];
		while (1) ;
#endif

		/*当前任务不等于最高任务，就切换成最高任务 */
		if (vie_os_prio_cur != vie_os_prio_high) {
			vie_os_prio_cur = vie_os_prio_high;
			p_task_high = p_task_array[vie_os_prio_cur];
			p_task_cur = p_task_high;
			os_isr_sw();
		}
	}
	OS_ON_INTERRUPT();
}

/*时钟节拍，一个节拍服务*/
void
os_timtick(void)
{
	struct os_blocks_tk *p_tk;

	/*用于给用户需要在拍节中断一开始就做点什么的机会
	 * os_time_tick_hook()正是这样的函数
	 os_time_tick_hook();
	 * 因为现在不用，所以暂不用写*/

	p_tk = p_task_list;	/*指向任务列表 */

	/*OS_IDLE_PRIO为最低任务，初始化时第一个任务为空闲任务 */
	while (p_tk->osbt_prio != OS_IDLE_PRIO) {
		OS_OFF_INTERRUPT();
		if (p_tk->osbt_dly != 0) {
			if (--p_tk->osbt_dly == 0) {
				if (!(p_tk->osbt_stat & OS_STAT_SUSPEND)) { /*没被挂起，就加入就绪表 */
					vie_os_ready |= p_tk->osbt_bit_y;
					array_os_tbl[p_tk->osbt_cb_y] |=
					    p_tk->osbt_bit_x;
				} else {
					p_tk->osbt_dly = 1;
				}
			}
		}
		p_tk = p_tk->osbt_next;
		OS_ON_INTERRUPT();
	}
}

/*求栈顶函数*/
/*1参数是任务函数地址*/
/*2参数是任务参数数据*/
/*3参数是栈指针*/
/*4参数是cmd选项*/
OS_STK *
os_task_stk_init(void (*p_task) (void *), void *p_data,
		OS_STK * p_bos, INT8U cmd)
{
	INT16U tmp;
	OS_STK *p;

	OS_OFF_INTERRUPT();

	/*防Wall */
	cmd = cmd;

	p = p_bos;
	tmp = (INT16U) p_task;

	/*PC地址 */
	*p-- = tmp;
	*p-- = tmp >> 8;

	*p-- = 0x00;		/*r0 */
	*p-- = 0x00;		/*r1 */
	*p-- = 0x00;		/*r2 */
	*p-- = 0x00;		/*r3 */
	*p-- = 0x00;		/*r4 */
	*p-- = 0x00;		/*r5 */
	*p-- = 0x00;		/*r6 */
	*p-- = 0x00;		/*r7 */
	*p-- = 0x00;		/*r8 */
	*p-- = 0x00;		/*r9 */
	*p-- = 0x00;		/*r10 */
	*p-- = 0x00;		/*r11 */
	*p-- = 0x00;		/*r12 */
	*p-- = 0x00;		/*r13 */
	*p-- = 0x00;		/*r14 */
	*p-- = 0x00;		/*r15 */
	*p-- = 0x00;		/*r16 */
	*p-- = 0x00;		/*r17 */
	*p-- = 0x00;		/*r18 */
	*p-- = 0x00;		/*r19 */
	*p-- = 0x00;		/*r20 */
	*p-- = 0x00;		/*r21 */
	*p-- = 0x00;		/*r22 */
	*p-- = 0x00;		/*r23 */

	tmp = (INT16U) p_data;
	*p-- = tmp;		/*r24 */
	*p-- = tmp >> 8;	/*r25 */

	*p-- = 0x00;		/*r26 */
	*p-- = 0x00;		/*r27 */
	*p-- = 0x00;		/*r28 */
	*p-- = 0x00;		/*r29 */
	*p-- = 0x00;		/*r30 */
	*p-- = 0x00;		/*r31 */

	*p-- = 0x80;	        /*SREG*/
	
	
	OS_ON_INTERRUPT();

	return p;
}

/*os_isr_exit调用来退出中断*/
/*任务转换时的实际转换函数*/
/*用于把寄器存压入当前任务堆*/
/*把最优先任务的值弹出来到寄存器*/
void
os_isr_sw(void)
{
	/*返回任务自己的堆栈 */
	OS_SP_L = (int) p_task_cur->osbt_stkptr;
	OS_SP_H = (int) p_task_cur->osbt_stkptr >> 8;

	/*弹出寄存器的值,马上返回，POP_REGISTER里包含了回返代码 */
	asm(POP_REGISTER);
}

/*用于sched任务调度时，摸拟中断*/
void
os_sched_sw(void)
{
	OS_OFF_INTERRUPT();
	/*avrgcc调用函数会把r28,r29压入栈，所以先弹出r28,r29*/
	asm("pop r28\n" "pop r29\n" PUSH_REGISTER);
	p_task_cur->osbt_stkptr = OS_SP;
	p_task_cur = p_task_high;

	OS_SP_L = (int) p_task_cur->osbt_stkptr;
	OS_SP_H = (int) p_task_cur->osbt_stkptr >> 8;

	asm(POP_REGISTER);
}

/*延时value个节拍*/
void
os_time_dly(INT16U value)
{

	OS_OFF_INTERRUPT();

	/*把当前任务从就绪表去掉，好延时 */
	if (!(array_os_tbl[p_task_cur->osbt_cb_y]
	      &= ~p_task_cur->osbt_bit_x)) {
		vie_os_ready &= ~p_task_cur->osbt_bit_y;
	}
	p_task_cur->osbt_dly = value;

	OS_ON_INTERRUPT();

	os_task_sched();	/*调度其它未延时的任务 */
}

/*空函数，用于空操作*/
void
os_nop_function(void) {}

/*获得版本号*100的值*/
INT16U
os_version(void)
{
	return OS_VERSION;
}
