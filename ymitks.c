#include <avr/interrupt.h>
#include <avr/io.h>

#ifndef DATA_TYPE
	#include "datatype.h"
#endif

#ifndef OS_TASK_CREATE_EXT_EN
	#include "osmacro.h"
#endif

void TIMER2_COMP_vect(void) __attribute__ ((signal, naked));

int
main(void)
{
#if BUGNO
	DDRB = 0xff;
	PORTB = 0xff;
#endif
	os_init();		/*初始化YMITKS系统 */

	/*安装YMITKS的任务切换向量 */
	/*创建用户起始任务:an_task_start() */
	if (os_task_create(&an_task_start, NULL,
			   &stack_an_porta[OS_STACK_MAX - 1], 10) < 0) {
		/*这里写出错代码 */
	}

	/*创建任务B, 优先级30 */
	if (os_task_create(&an_task_portb, NULL,
			   &stack_an_portb[OS_STACK_MAX - 1], 30) < 0) {
		/*这里写出错代码 */
	}

	os_start();		/*开始多任务调度，未写完 */
	return 0;
}

/*起始任务*/
/*该任务的优先级10*/
void
an_task_start(void *p_data)
{
#if OS_TASK_STAT_EN
	os_statinit();		/*初始化统计任务 */
#endif

	INT8U  i;
	INT16U x, y;
	p_data = p_data;
	DDRA = 0xff;
	i = 1;
	for (;;) {
		if (i == 0x00)
			i = 1;
		PORTA = ~i;
		i <<= 1;

		os_time_dly(1);

		for (x = 0; x < 200; ++x)
			for (y = 0; y < 800; ++y)
				;
	}
}

/*统计任务初始化*/
/*主要用于初始化最大空闲任务统计数和把统计标志设为TRUE*/
void
os_statinit(void)
{
	os_time_dly(2);		/*延迟2个时钟节拍 */
	OS_OFF_INTERRUPT();
	vie_os_idle = 0l;
	OS_ON_INTERRUPT();

	os_time_dly(OS_TICKS_PER_SEC);	/*延迟一个常用节拍，一般是1 */

	OS_OFF_INTERRUPT();
	vie_os_idle_max = vie_os_idle;
	vie_os_stat_rdy = TRUE;
	OS_ON_INTERRUPT();
}

/*统计任务*/
/*用于计算任务cpu利用率*/
void
os_task_stat(void *p_data)
{
	INT8S usage;
	INT32U run;
	while (vie_os_stat_rdy == FALSE)
		os_time_dly(2 * OS_TICKS_PER_SEC);
	for (;;) {
		OS_OFF_INTERRUPT();
		vie_os_idle_last = vie_os_idle;
		run = vie_os_idle_last;
		vie_os_idle = 0l;
		OS_ON_INTERRUPT();

		if (vie_os_idle_max > 0l) {
			usage = (INT8S) (100l - 100l * run / vie_os_idle_max);
			if (usage > 100)
				vie_os_cpu_usage = 100;
			else if (usage < 0)
				vie_os_cpu_usage = 0;
			else
				vie_os_cpu_usage = usage;
		} else {
			vie_os_cpu_usage = 0;
		}
		/*os_task_stat_hook(); *//*在这里可以用该函数作点什么，未写 */
		os_time_dly(OS_TICKS_PER_SEC);	/*延时一个常用节拍 */
	}
}

/*开始任务调度*/
/*初始化时钟节拍*/
/*也初始化软件中断*/
void
os_start(void)
{
	/*用avr计数器2来做为时钟节拍的计数器 */
	/*1秒发生40节拍 */
	TIMSK |= 0x80;
	TCCR2 |= 0x08;
	OCR2 = 201;

	/*从任务就绪表中找出那个用户建立的最高优先任级 */
	INT8U y;
	INT8U x;

	if (vie_os_runing == FALSE) {	/*未启动就启动任务 */
		vie_os_runing = TRUE;	/*任务运行中 */
		y = array_cn_un_taskmap[vie_os_ready];
		x = array_cn_un_taskmap[array_os_tbl[y]];
		vie_os_prio_high = (INT8U) (y << 3) + x;
		vie_os_prio_cur = vie_os_prio_high;
		p_task_high = p_task_array[vie_os_prio_high];
		p_task_cur = p_task_high;

		TCCR2 |= 0x07;	/*开启计时器 */

		/*os_strart_highrdy是调用最高优先任务函数 */
		/*该函数是将任务栈中保存的值弹回寄存器 */
		/*该函数永远不会返回到os_start */
		os_strart_highrdy();
	} else {
		/*启动了，就提示出错 */
	}
}

/*启动最高的任务*/
void
os_strart_highrdy(void)
{
	OS_SP_L = (INT16U) p_task_cur->osbt_stkptr;
	OS_SP_H = (INT16U) p_task_cur->osbt_stkptr >> 8;
	/*SP = p_task_cur->osbt_stkptr; */
	asm(POP_REGISTER);
}

/*OCR2中断处理服务，也就是时钟节拍的中断处理*/
ISR(TIMER2_COMP_vect)
{
	asm(PUSH_REGISTER);	/*寄存器(register)压入栈 */
	p_task_cur->osbt_stkptr = OS_SP;	/*保存栈 */
	os_isr_add();
	OS_I_INTERRUPT();	/*开启嵌套中断，开中断 */
	os_timtick();
	os_isr_exit();
	asm(POP_REGISTER);	/*寄存器(register)弹出栈 */
}

/*系统初始化*/
/*建一个空闲任务，先项建立任务统计任务*/
/*初始化任务和空任务链表*/
void
os_init()
{
	int ik;
	/*初始化空任务链表，用单链表连接 */
	p_task_free_list = array_os_task_tbl;
	ik = 0;
	while (ik < OS_LAST_TASKS) {
		array_os_task_tbl[ik].osbt_next = &array_os_task_tbl[ik+1];
		++ik;
	}
	array_os_task_tbl[OS_LAST_TASKS].osbt_next = NULL;

	/*初始化非空任务链表，用双向链表连接 */
	p_task_list = NULL;

	/*建立一个空闲任务 */
	os_task_create(&os_task_idle, NULL,
		       &stack_os_leisure[OS_STACK_MAX - 1], OS_LOWEST_PRIO_A);
	/*根据宏来是否建立一个统计任务的任务 */
#if OS_TASK_STAT_EN
	os_task_create(OS_LOWEST_PRIO_B);
#endif

	/*系统初始化还要初始化4个空数据到缓冲区 */
	/*其中空任务链表已写，还有 */
	/*指向空事件表的缓冲区 */
	/*指向空队列表的缓冲区 */
	/*指向空存储区的缓冲区 */
}
