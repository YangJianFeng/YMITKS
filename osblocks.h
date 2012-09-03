#ifndef DATA_TYPE
	#include "datatype.h"
#endif

#ifndef OS_TASK_CREATE_EXT_EN
	#include "osmacro.h"
#endif

#define OS_BLOCKS		/*加载该模块用 */

struct os_blocks_tk {
	OS_STK *osbt_stkptr;	/*栈顶指针 */

/*扩展部份*/
#if OS_TASK_CREATE_EXT_EN
	void   *osbt_ext_ptr;	  /*可用于跟踪任务执行时间等 */
	OS_STK *osbt_stk_bottom;  /*堆底指针 */
	INT32U osbt_stk_size;	  /*堆中地址大小 */
	INT16U osbt_opt;	  /*用于一些函数参数，检查栈等 */
	INT16U osbt_id;		  /*任务ID */
#endif

	struct os_blocks_tk *osbt_prev;	/*指向上一个链表 */
	struct os_blocks_tk *osbt_next;	/*指向下一个链表 */

#if (OS_Q_EN && (OS_MAX_QS >= 2)) || OS_MBOX_EN || OS_SEM_EN
	OS_EVENT *osbt_event;	        /*指向事件控制块的指针，暂不懂 */
	                                /*OS_EVENT类型未写 */
#endif

#if (OS_Q_EN && (OS_MAX_QS >= 2)) || OS_MBOX_EN
	void *osbt_msg;		/*指向任务消息的指针 */
#endif

	INT16U osbt_dly;	/*任务延时若干节拍 */
	INT8U osbt_stat;	/*任务状态 */
	INT8U osbt_prio;	/*任务优先级 */

	/*
	 * 用于加任务进入就绪状态的过程
	 * 或进入等待事件发生状态的过程
	 *（避免在运行中去计算这些值）
	 */
	INT8U osbt_cb_x;	/*就绪x值 */
	INT8U osbt_cb_y;	/*就绪y值 */
	INT8U osbt_bit_x;	/*就绪x位置 */
	INT8U osbt_bit_y;	/*就绪y位置 */

	BOOLEAN osbt_del_req;	/*用于该任务是否被请求删除 */
};
