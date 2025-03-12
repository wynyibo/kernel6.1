/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_NSPROXY_H
#define _LINUX_NSPROXY_H

#include <linux/spinlock.h>
#include <linux/sched.h>

struct mnt_namespace;
struct uts_namespace;
struct ipc_namespace;
struct pid_namespace;
struct cgroup_namespace;
struct fs_struct;

/*
 * A structure to contain pointers to all per-process
 * namespaces - fs (mount), uts, network, sysvipc, etc.
 *
 * The pid namespace is an exception -- it's accessed using
 * task_active_pid_ns.  The pid namespace here is the
 * namespace that children will use.
 *
 * 'count' is the number of tasks holding a reference.
 * The count for each namespace, then, will be the number
 * of nsproxies pointing to it, not the number of tasks.
 *
 * The nsproxy is shared by tasks which share all namespaces.
 * As soon as a single namespace is cloned or unshared, the
 * nsproxy is copied.
 */
//命名空间核心数据结构，每个进程会通过task_struct关联一个nsproxy结构体
struct nsproxy {
	atomic_t count;
	struct uts_namespace *uts_ns;//主机名
	struct ipc_namespace *ipc_ns;//ipc
	struct mnt_namespace *mnt_ns;//文件系统挂载点
	struct pid_namespace *pid_ns_for_children;//进程标号
	struct net 	     *net_ns;//网络协议栈
	struct time_namespace *time_ns;//时间命名空间，隔离时钟和时间
	struct time_namespace *time_ns_for_children;//子进程继承的时间命名空间
	struct cgroup_namespace *cgroup_ns;//cgroup命名空间（控制进程对资源访问）
};
extern struct nsproxy init_nsproxy;

/*
 * A structure to encompass all bits needed to install
 * a partial or complete new set of namespaces.
 *
 * If a new user namespace is requested cred will
 * point to a modifiable set of credentials. If a pointer
 * to a modifiable set is needed nsset_cred() must be
 * used and tested.
 */
struct nsset {
	unsigned flags;
	struct nsproxy *nsproxy;
	struct fs_struct *fs;
	const struct cred *cred;
};

static inline struct cred *nsset_cred(struct nsset *set)
{
	if (set->flags & CLONE_NEWUSER)
		return (struct cred *)set->cred;

	return NULL;
}

/*
 * the namespaces access rules are:
 *
 *  1. only current task is allowed to change tsk->nsproxy pointer or
 *     any pointer on the nsproxy itself.  Current must hold the task_lock
 *     when changing tsk->nsproxy.
 *
 *  2. when accessing (i.e. reading) current task's namespaces - no
 *     precautions should be taken - just dereference the pointers
 *
 *  3. the access to other task namespaces is performed like this
 *     task_lock(task);
 *     nsproxy = task->nsproxy;
 *     if (nsproxy != NULL) {
 *             / *
 *               * work with the namespaces here
 *               * e.g. get the reference on one of them
 *               * /
 *     } / *
 *         * NULL task->nsproxy means that this task is
 *         * almost dead (zombie)
 *         * /
 *     task_unlock(task);
 *
 */

int copy_namespaces(unsigned long flags, struct task_struct *tsk);
void exit_task_namespaces(struct task_struct *tsk);
void switch_task_namespaces(struct task_struct *tsk, struct nsproxy *new);
void free_nsproxy(struct nsproxy *ns);
int unshare_nsproxy_namespaces(unsigned long, struct nsproxy **,
	struct cred *, struct fs_struct *);
int __init nsproxy_cache_init(void);

static inline void put_nsproxy(struct nsproxy *ns)
{
	if (atomic_dec_and_test(&ns->count)) {
		free_nsproxy(ns);
	}
}

static inline void get_nsproxy(struct nsproxy *ns)
{
	atomic_inc(&ns->count);
}

#endif
