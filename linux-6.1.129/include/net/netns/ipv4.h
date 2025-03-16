/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ipv4 in net namespaces
 */

#ifndef __NETNS_IPV4_H__
#define __NETNS_IPV4_H__

#include <linux/uidgid.h>
#include <net/inet_frag.h>
#include <linux/rcupdate.h>
#include <linux/seqlock.h>
#include <linux/siphash.h>

struct ctl_table_header;
struct ipv4_devconf;
struct fib_rules_ops;
struct hlist_head;
struct fib_table;
struct sock;
struct local_ports {
	seqlock_t	lock;
	int		range[2];
	bool		warned;
};

struct ping_group_range {
	seqlock_t	lock;
	kgid_t		range[2];
};

struct inet_hashinfo;

struct inet_timewait_death_row {
	refcount_t		tw_refcount;

	/* Padding to avoid false sharing, tw_refcount can be often written */
	struct inet_hashinfo 	*hashinfo ____cacheline_aligned_in_smp;
	int			sysctl_max_tw_buckets;
};

struct tcp_fastopen_context;

struct netns_ipv4 {
    struct inet_timewait_death_row tcp_death_row; // TCP TIME_WAIT 连接的管理

#ifdef CONFIG_SYSCTL
    struct ctl_table_header *forw_hdr;
    struct ctl_table_header *frags_hdr;
    struct ctl_table_header *ipv4_hdr;
    struct ctl_table_header *route_hdr;
    struct ctl_table_header *xfrm4_hdr;
#endif

    struct ipv4_devconf *devconf_all;   // 所有设备的 IPv4 配置
    struct ipv4_devconf *devconf_dflt;  // 默认设备的 IPv4 配置

    struct ip_ra_chain __rcu *ra_chain; // 路由通告链表
    struct mutex ra_mutex;              // 路由通告的互斥锁

#ifdef CONFIG_IP_MULTIPLE_TABLES
    struct fib_rules_ops *rules_ops;    // FIB 规则操作
    struct fib_table __rcu *fib_main;   // 主 FIB（Forwarding Information Base）路由表
    struct fib_table __rcu *fib_default;// 默认 FIB 路由表
    unsigned int fib_rules_require_fldissect;
    bool fib_has_custom_rules;          // 是否有自定义路由规则
#endif

    bool fib_has_custom_local_routes;   // 是否有本地自定义路由
    bool fib_offload_disabled;          // 是否禁用路由卸载

    u8 sysctl_tcp_shrink_window;        // TCP 窗口收缩策略

#ifdef CONFIG_IP_ROUTE_CLASSID
    atomic_t fib_num_tclassid_users;
#endif

    struct hlist_head *fib_table_hash;  // FIB 路由表哈希
    struct sock *fibnl;                 // FIB 通知 socket（Netlink）

    struct sock *mc_autojoin_sk;        // 组播自动加入的 socket

    struct inet_peer_base *peers;       // TCP 连接跟踪
    struct fqdir *fqdir;                // Flow Queue 目录

    /* ICMP 相关参数 */
    u8 sysctl_icmp_echo_ignore_all;
    u8 sysctl_icmp_echo_enable_probe;
    u8 sysctl_icmp_echo_ignore_broadcasts;
    u8 sysctl_icmp_ignore_bogus_error_responses;
    u8 sysctl_icmp_errors_use_inbound_ifaddr;
    int sysctl_icmp_ratelimit;
    int sysctl_icmp_ratemask;

    /* 路由相关参数 */
    u32 ip_rt_min_pmtu;
    int ip_rt_mtu_expires;
    int ip_rt_min_advmss;

    struct local_ports ip_local_ports;  // 本地端口管理

    /* TCP 相关参数 */
    u8 sysctl_tcp_ecn;
    u8 sysctl_tcp_ecn_fallback;
    u8 sysctl_tcp_mtu_probing;
    int sysctl_tcp_mtu_probe_floor;
    int sysctl_tcp_base_mss;
    int sysctl_tcp_min_snd_mss;
    int sysctl_tcp_probe_threshold;
    u32 sysctl_tcp_probe_interval;

    int sysctl_tcp_keepalive_time;
    int sysctl_tcp_keepalive_intvl;
    u8 sysctl_tcp_keepalive_probes;

    u8 sysctl_tcp_syn_retries;
    u8 sysctl_tcp_synack_retries;
    u8 sysctl_tcp_syncookies;
    u8 sysctl_tcp_migrate_req;

    int sysctl_tcp_fin_timeout;
    unsigned int sysctl_tcp_notsent_lowat;

    int sysctl_tcp_wmem[3];
    int sysctl_tcp_rmem[3];

    /* UDP 相关参数 */
    int sysctl_udp_wmem_min;
    int sysctl_udp_rmem_min;

    /* FIB 路由多路径 */
#ifdef CONFIG_IP_ROUTE_MULTIPATH
    u32 sysctl_fib_multipath_hash_fields;
    u8 sysctl_fib_multipath_use_neigh;
    u8 sysctl_fib_multipath_hash_policy;
#endif

    atomic_t rt_genid;  // 路由表版本控制
    siphash_key_t ip_id_key;  // IP ID 生成的密钥
};

#endif
