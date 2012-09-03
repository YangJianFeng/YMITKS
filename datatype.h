/*用于加载该头文件*/
#define DATA_TYPE

/*数据类型定义*/
typedef unsigned char INT8U;
typedef signed   char INT8S;
typedef unsigned int  INT16U;
typedef signed   int  INT16S;
typedef unsigned long INT32U;
typedef signed   long INT32S;

/*define区*/
#define BOOLEAN INT8U		/*BOOL类型 */
#define OS_STK  char		/*堆栈类型 */

/*加载errno常量*/
#include "oserrno.h"

/*加载数据模块*/
#ifndef OS_BLOCKS
	#include "osblocks.h"
#endif

/*全局变量声明*/
#ifndef OS_NO_EXTERN

extern INT8U  vie_os_ready;	        /*就绪优先级组,相当于y值 */
extern INT8U  array_os_tbl[8];	        /*就绪优先级号所在组位 */
extern INT8U  array_cn_taskmap[8];	/*用于计算就绪优先级组位 */
extern INT8U  array_cn_un_taskmap[];	/*反算就绪优先级号 */

extern INT8U  vie_os_isr_number;	/*中断嵌套层数 */
extern INT8U  vie_os_locksched;	        /*任务调度上锁 */
extern INT8U  vie_os_stat_rdy;	        /*统计任务就绪标志 */
extern INT32U vie_os_idle;	        /*空闲任务时统计任务 */
extern INT32U vie_os_idle_last;	        /*记录最后一次空闲任务的统计值 */
extern INT32U vie_os_idle_max;	        /*1个常用节拍的最大空闲统计值 */

extern INT8U  vie_os_runing;	        /*是否有任务在可运行 */
extern INT8U  vie_os_prio_high;	        /*优先级最高的号数 */
extern INT8U  vie_os_prio_cur;	        /*当前任务的号数 */

extern INT8U  vie_os_cpu_usage;	        /*任务的cpu利用率 */
extern INT8U  vie_os_isrnumber;	        /*中断嵌套层数 */
extern INT8U  vie_os_isrexit;	        /*用于os_isr_exit计算最高任务的调度 */

extern INT16U errno;		        /*用于记录出错编号，可在errno手册查看 */

extern INT8U  stack_os_leisure[OS_STACK_MAX];	/*空闲任务堆栈 */
extern INT8U  stack_an_porta[OS_STACK_MAX];	/*应用任务堆栈 */

extern struct os_blocks_tk array_os_task_tbl[];	/*任务列表数组 */
extern struct os_blocks_tk *p_task_array[64];	/*用于任务号对应任务 */
extern struct os_blocks_tk *p_task_high;	/*最高任务指针 */
extern struct os_blocks_tk *p_task_cur;	        /*当前任务指针 */
extern struct os_blocks_tk *p_task_list;	/*非空任务列表 */
extern struct os_blocks_tk *p_task_free_list;	/*空任务列表 */

#endif

/*函数声明*/
void os_task_sched(void);	/*任务调度器 */
void os_ched_lock(void);	/*任务调度上锁 */
void os_ched_unlock(void);	/*任务解锁 */
void os_sched_sw(void);		/*sched时的任务转换 */

void os_statinit(void);		/*初始化统计任务等 */
void os_task_idle(void *);	/*空闲任务 */
void os_task_stat(void *);	/*统计任务 */
void an_task_start(void *);	/*起始任务 */

void os_isr(void);		/*中断服务函数 */
void os_isr_add(void);		/*中断嵌套层数加1 */
void os_isr_exit(void);		/*脱离中断服务,减1，如是0，就检查最高的任务 */
void os_isr_sw(void);		/*任务转换的实际转换函数 */
void os_start(void);		/*开始多任务调度 */
void os_strart_highrdy(void);	/*任务开始，调用最高优先级的任务 */

void os_timtick(void);		/*时钟节拍，一个节拍服务 */
void os_time_dly(INT16U x);	/*延时x个节拍 */
void os_nop_function(void);     /*空函数，用于空操作*/

void os_init(void);		/*系统初始化 */

INT16U os_version(void);	/*获得YMITKS版本号*100的值 */

/*求任务的栈顶指针,有4个参数*/
OS_STK *os_task_stk_init(void (*)(), void *, OS_STK *, INT8U);

/*建立任务，正确返回0，错误返回-1*/
/*分别是:任务函数地址*/
/*任务函数的参数（是个void指针）*/
/*栈指针*/
/*任务的优先级*/
INT8S os_task_create(void (*)(), void *, OS_STK *, INT8U);

/*初始化创建任务,正确返回0，错误返回-1*/
/*初始创建化任务，让p_task_array指向任务*/
/*有7个参数*/
/*1 : 任务优先级号*/
/*2 : 栈顶指针*/
/*3 : 栈底指针(扩展）*/
/*4 : 任务ID(扩展）*/
/*5 : 栈的大小(扩展）*/
/*6 : 用于跟踪值行时间,或调用次数(扩展）*/
/*7 : 选项，用于一些函数检查(扩展）*/
INT8S os_task_caeinit(INT8U, OS_STK *, OS_STK *, INT16U,
		INT16U, void *, INT16U);

/*任务相关函数声明*/
INT8S os_task_del(void);                      /*删除调用该函数的任务*/
INT8S os_task_del_request(INT8U prio);        /*请求删除prio任务*/

/*把old优先级改变为new估先级*/
INT8S os_task_change_prio(INT8U old, INT8U _new);


/******************************
 * 任务函数声明
 ******************************/
void an_task_portb(void *);	                /*任务B */
extern INT8U stack_an_portb[OS_STACK_MAX];	/*应用任务B堆栈 */
