#ifndef DATA_TYPE
	#include "datatype.h"
#endif

#ifndef OS_TASK_CREATE_EXT_EN
	#include "osmacro.h"
#endif

/*建立任务,正确返回0,错误返回-1*/
/* 4个参数分别是*/
/*任务函数地址*/
/*任务函数的参数（是个void指针）*/
/*栈指针*/
/*任务的优先级*/
INT8S
os_task_create(void (*p_task) (void *p_dt), void *p_data, OS_STK * p_bos,
		INT8U prio)
{
	void *p_tmp_pop;	/*用于指向栈顶 */
	INT8S err;

	if (prio > OS_LOWEST_PRIO_A)
		return OS_ERROR;

	OS_OFF_INTERRUPT();
	if (p_task_array[prio] == NULL) {
		p_task_array[prio] = (void *) 1;
		OS_ON_INTERRUPT();
		/*求堆顶地址 */
		p_tmp_pop = os_task_stk_init(p_task, p_data, p_bos, 0);
		/*用os_task_caeinit初始创建化任务，让p_task_array指向任务 */
		err = os_task_caeinit(prio, p_tmp_pop, NULL, 0, 0, NULL, 0);
		if (err == 0) {
			if (vie_os_runing)
				os_task_sched();
		} else {
			OS_OFF_INTERRUPT();
			p_task_array[prio] = NULL;
			OS_ON_INTERRUPT();
			return OS_ERROR;
		}
	} else {
		OS_ON_INTERRUPT();
		errno = OS_ERRNO_1;
		return OS_ERROR;
	}
	return OS_RIGHT;
}

/*初始创建化任务，让p_task_array指向任务*/
/*正确返回0，错误返回-1*/
/*prio    : 任务优先级号*/
/*p_tos   : 堆栈顶指针*/
/*p_bos   : 堆栈底顶指针(扩展）*/
/*id      : 任务ID(扩展）*/
/*stk_size: 栈的大小(扩展）*/
/*p_ext   : 用于跟踪值行时间,或调用次数(扩展）*/
/*opt     : 选项，用于一些函数检查(扩展）*/
INT8S
os_task_caeinit(INT8U prio, OS_STK * p_tos, OS_STK * p_bos,
		INT16U id, INT16U stk_size, void *p_ext, INT16U opt)
{
	struct os_blocks_tk *ptk;

	OS_OFF_INTERRUPT();
	ptk = p_task_free_list;
	if (ptk != NULL) {
		p_task_free_list = p_task_free_list->osbt_next;
		OS_ON_INTERRUPT();

		/*赋值部份 */
		ptk->osbt_stkptr = p_tos;
		ptk->osbt_dly = 0;
		ptk->osbt_stat = OS_STAT_READY;
		ptk->osbt_prio = prio;
		ptk->osbt_cb_y = prio >> 3;
		ptk->osbt_cb_x = prio & 0x07;
		ptk->osbt_bit_y = array_cn_taskmap[ptk->osbt_cb_y];
		ptk->osbt_bit_x = array_cn_taskmap[ptk->osbt_cb_x];

		/*扩展部份 */
#if OS_TASK_CREATE_EXT_EN
		ptk->osbt_ext_ptr = p_ext;
		ptk->osbt_stk_bottom = p_bos;
		ptk->osbt_stk_size = stk_size;
		ptk->osbt_opt = opt;
		ptk->osbt_id = id;
#else
		p_ext = p_ext;
		p_bos = p_bos;
		stk_size = stk_size;
		opt = opt;
		id = id;
#endif

		/*消息、事件同步 */
#if (OS_Q_EN && (OS_MAX_QS >= 2)) || OS_MBOX_EN || OS_SEM_EN
		ptk->osbt_event_ptr = NULL;
#endif
#if (OS_Q_EN && (OS_MAX_QS >= 2)) || OS_MBOX_EN
		ptk->osbt_msg = NULL;
#endif

		/*任务是否删除 */
		ptk->osbt_del_req = OS_DEL_TASK_NO;

		OS_OFF_INTERRUPT();

		/*任务号数组链接与链接非空任务表 */
		p_task_array[prio] = ptk;
		ptk->osbt_prev  = NULL;
		ptk->osbt_next = p_task_list;
		if (p_task_list != NULL)
			p_task_list->osbt_prev = ptk;
		p_task_list = ptk;

		/*把任务放入就绪表 */
		vie_os_ready |= ptk->osbt_bit_y;
		array_os_tbl[ptk->osbt_cb_y] |= ptk->osbt_bit_x;

		OS_ON_INTERRUPT();
		return OS_RIGHT;
	} else {
		OS_ON_INTERRUPT();
		errno = 4;
		return OS_ERROR;
	}
}

/*空闲任务*/
void
os_task_idle(void *p_data)
{
	p_data = p_data;
	for (;;) {
		OS_OFF_INTERRUPT();
		++vie_os_idle;	/*空闲任务统计 */
		OS_ON_INTERRUPT();
	}
}


/*
 * 删除任务函数
 * 用于删除任调用该函数的任务
 * 成功返回0
 * 出错返回-1
 * errno的值被修改
 * 查看error手册，可查看错误信息
 */
INT8S
os_task_del(void)
{
#if OS_Q_EN
	OSEVENT             *pointer_event;
#endif
	if (p_task_cur == NULL) {
		errno = OS_ERRNO_7;
		return OS_ERROR;
	}
	if (vie_os_prio_cur == OS_IDLE_PRIO) {
		errno = OS_ERRNO_5;
		return OS_ERROR;
	}
	OS_OFF_INTERRUPT();
	if (vie_os_isrnumber > 0) {
		OS_ON_INTERRUPT();
		errno = OS_ERRNO_6;
		return OS_ERROR;
	}
	if ((array_os_tbl[p_task_cur->osbt_cb_y]
			&= ~p_task_cur->osbt_bit_x) == 0) {
		vie_os_ready &= ~p_task_cur->osbt_bit_y;
	}
	/*
	 * 这里还应该有个删除邮箱、消信列队和信号量等的代码，未写
	 * if (....) {}
	 */

	p_task_cur->osbt_dly  = 0;                /*节拍延为0，防止任务重启*/
	p_task_cur->osbt_stat = OS_STAT_DORMANT;  /*挂起状态*/

	/*
	 * 锁上任务调度，开启中断
	 * 用一个空操作，防止开启中断必须执行一条指令
	 * 然后马上关闭中断
	 */
	++vie_os_locksched;
	OS_ON_INTERRUPT();
	OS_NOP();
	OS_OFF_INTERRUPT();
	--vie_os_locksched;
	
	p_task_array[vie_os_prio_cur] = NULL;
	p_task_cur->osbt_next->osbt_prev =
	    p_task_cur->osbt_prev;
	if (p_task_cur->osbt_prev != NULL) {
		p_task_cur->osbt_prev->osbt_next =
		    p_task_cur->osbt_next;
	} else {
		p_task_list = p_task_cur->osbt_next;
	}
	p_task_cur->osbt_next = p_task_free_list;
	p_task_free_list     = p_task_cur;
	
	OS_ON_INTERRUPT();
	os_task_sched();
	return 0;

}

/*
 * 请求删除函数
 * 返回值说明：
 * OS_ERROR  函数出错返回
 * OS_DEL_TASK_NOT_EXIST 任务不存或已经被删除
 * OS_DEL_TASK_YES 任务已请求被删除
 * OS_DEL_TASK_NO 任务未请求被删除
 * 返回0  任务正常返回，表明请求已被接受但任务还没被删除
 */
INT8S
os_task_del_request(INT8U prio)
{
	if (prio == OS_IDLE_PRIO) {
		errno = OS_ERRNO_5;
		return OS_ERROR;
	}
	if (prio > OS_IDLE_PRIO) {
		errno = OS_ERRNO_1;
		return OS_ERROR;
	}

	OS_OFF_INTERRUPT();
	if (prio == vie_os_prio_cur) {
		OS_ON_INTERRUPT();
		return p_task_cur->osbt_del_req;
	} else {
		if (p_task_array[prio] == NULL)
			return OS_DEL_TASK_NOT_EXIST;
		p_task_array[prio]->osbt_del_req = OS_DEL_TASK_YES;
		OS_ON_INTERRUPT();
		return 0;
	}
}

/*
 * 任务优先级更改
 * 成功返回0
 * 错误返回OS_ERROR，并修改errno的值
 */
INT8S
os_task_change_prio(INT8U old_value, INT8U new_value)
{
	struct os_blocks_tk *pointer_task;
	INT8U  cb_y;
	INT8U  cb_x;
	INT8U  bit_y;
	INT8U  bit_x;

	if (old_value == OS_IDLE_PRIO && new_value == OS_IDLE_PRIO) {
		errno = OS_ERRNO_5;
		return OS_ERROR;
	}
	if (old_value > OS_LOWEST_PRIO_A && new_value > OS_LOWEST_PRIO_A) {
		errno = OS_ERRNO_1;
		return OS_ERROR;
	}

	OS_OFF_INTERRUPT();
	if (p_task_array[new_value] == NULL) {
		p_task_array[new_value] = (struct os_blocks_tk *) 1;
		OS_ON_INTERRUPT();
		cb_y = new_value >> 3;
		cb_x = new_value & 0x07;
		bit_y = array_cn_taskmap[cb_y];
		bit_x = array_cn_taskmap[cb_x];
	} else {
		OS_ON_INTERRUPT();
		errno = OS_ERRNO_3;
		return OS_ERROR;
	}
	
	OS_OFF_INTERRUPT();
	if ((pointer_task = p_task_array[old_value]) != NULL) {
		p_task_array[old_value] = NULL;
		if (array_os_tbl[pointer_task->osbt_cb_y]
			& pointer_task->osbt_bit_x) {
			if ((array_os_tbl[pointer_task->osbt_cb_y]
				&= ~pointer_task->osbt_bit_x) == 0) {
				vie_os_ready &= ~pointer_task->osbt_cb_y;
			}
			vie_os_ready       |= bit_y;
			array_os_tbl[cb_y] |= bit_x;
		} 
		#if OS_Q_EN
		/*如果没有就绪，查看事件列表，未写*/
		else {
			if ((pinter_event = pointer_task->osbt_event) != NULL)
				/*未写……*/
		}
		#endif
		p_task_array[new_value] = pointer_task;
		pointer_task->osbt_cb_x  = cb_x;
		pointer_task->osbt_cb_y  = cb_y;
		pointer_task->osbt_bit_x = bit_x;
		pointer_task->osbt_bit_y = bit_y;
		OS_ON_INTERRUPT();
		os_task_sched();
		return 0;
	} else {
		p_task_array[new_value] = NULL;
		OS_ON_INTERRUPT();
		errno = OS_ERRNO_8;
		return OS_ERROR;
	}
}
