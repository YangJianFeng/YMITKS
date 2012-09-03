#define OS_NO_EXTERN		/*防止加入extern */

#ifndef DATA_TYPE
	#include "datatype.h"
#endif

#ifndef OS_TASK_CREATE_EXT_EN
	#include "osmacro.h"
#endif

#ifndef OS_BLOCKS
	#include "osblocks.h"
#endif

/***************************************************************
                         就绪表
 ***************************************************************/
INT8U vie_os_ready;
INT8U array_os_tbl[8];
const INT8U array_cn_taskmap[8] =
    { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

const INT8U array_cn_un_taskmap[] = {
	0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0x00 to 0x0F */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0x10 to 0x1F */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0x20 to 0x2F */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0x30 to 0x3F */
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0x40 to 0x4F */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0x50 to 0x5F */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0x60 to 0x6F */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0x70 to 0x7F */
	7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0x80 to 0x8F */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0x90 to 0x9F */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0xA0 to 0xAF */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0xB0 to 0xBF */
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0xC0 to 0xCF */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0xD0 to 0xDF */
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* 0xE0 to 0xEF */
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0	/* 0xF0 to 0xFF */
};

INT8U  vie_os_isr_number;	/*中断嵌套层数 */
INT8U  vie_os_locksched;	/*任务调度上锁 */
INT8U  vie_os_stat_rdy;		/*统计任务就绪标志 */
INT32U vie_os_idle;		/*空闲任务时统计任务 */
INT32U vie_os_idle_last;	/*记录最后一次空闲任务的统计值 */
INT32U vie_os_idle_max;		/*1常用节拍的空闲时间统计值 */

INT8U  vie_os_runing;		/*是否有可运行的任务，TRUE or FALSE */
INT8U  vie_os_prio_high;	/*优先级最高的任务号 */
INT8U  vie_os_prio_cur;		/*当前任务号 */

INT8U  vie_os_cpu_usage;	/*任务的cpu利用率 */

INT8U  vie_os_isrnumber;	/*中断嵌套层数 */
INT8U  vie_os_isrexit;		/*用于os_isr_exit函数计算最高任务的调度 */

INT8U  stack_os_leisure[OS_STACK_MAX];	/*空闲任务堆栈 */
INT8U  stack_an_porta[OS_STACK_MAX];	/*任务1，porta灯亮任务的堆栈 */
INT8U  stack_an_portb[OS_STACK_MAX];	/*任务2，portb灯亮任务的堆栈 */

INT16U errno;			        /*用于记录出错编号，可在errno手册中查看 */

struct os_blocks_tk    array_os_task_tbl[OS_MAX_TASKS];	/*任务数组 */
struct os_blocks_tk    *p_task_array[64];	        /*用于任务号对应任务 */
struct os_blocks_tk    *p_task_high;	                /*最高任务指针 */
struct os_blocks_tk    *p_task_cur;	                /*当前任务指针 */
struct os_blocks_tk    *p_task_list;	                /*非空任务列表 */
struct os_blocks_tk    *p_task_free_list;	        /*空任务列表 */
