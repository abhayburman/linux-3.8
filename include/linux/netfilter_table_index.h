/*
 * This is a module which is used for L3 Firewall acceleration
 *
 * This module adds a table for index. When a new IP packet comes,
 * it will go along the original filter path, then a message about
 * the IP packet can be sent to the table so that the table can
 * remember the IP packet as an acquaintance, and the action in
 * the original filter path will be recorded either.
 * When another IP packet comes, it will be searched in the table
 * and then it will be judged whether it is an acquainted IP packet
 * or not. If it is found, in another word, if it is an acquainted
 * IP packet, it will take the last action recorded in the table,
 * but it does not need to go along the old long distance, this is
 * the shortest path.
 * In this way, we shortened the IP packet filter path to increase
 * the Firewall performance.
 *
 * Copyright (C) 2011 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Authors: Jianhua Xie(Adam) <b29408@freescale.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef	__H_NETFILTER_TABLE_INDEX_H__
#define	__H_NETFILTER_TABLE_INDEX_H__

#include <linux/spinlock.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/module.h>
#include <linux/init.h>

#ifdef CONFIG_NETFILTER_TABLE_INDEX

/* The length of the table */
#define MAXLINES_OF_TABLE 8

struct TABLE_INDEX{
	/* Income interface index */
	int	ifindex;
	/* Destination IP address */
	__be32 dst;
	/* Response from hook functions. */
	int response;
};

struct NF_TABLE_INDEX{
	rwlock_t    lock;
	struct TABLE_INDEX table_index[MAXLINES_OF_TABLE] ;
};

extern struct NF_TABLE_INDEX *p_netfilter_table_index;

/* The number of the rules in firewall */
extern bool firewall_rules;

int init_netfilter_table_index(void);

/* Record a new ip packet and the netfilter's response at the
 * top line of the acceleration table, including the destination
 * ip of the packet, the net device which the packet comes from,
 * and the netfilter's response.
 * The function returns 0 forever.
 */
int update_netfilter_table_index(
		struct NF_TABLE_INDEX *p_netfilter_table_index,
		struct net_device *dev,
		__be32  dst,
		int     response);

/* Search a new ip packet from the acceleration table, if it is found,
 * the function returns the response in the same line of the table,
 * else it returns -1.
 */
int match_netfilter_table_index(
		struct NF_TABLE_INDEX *p_netfilter_table_index,
		struct net_device *dev,
		__be32  dst);

#endif	/*endif	CONFIG_NETFILTER_TABLE_INDEX*/
#endif	/*endif __H_NETFILTER_TABLE_INDEX_H__*/
