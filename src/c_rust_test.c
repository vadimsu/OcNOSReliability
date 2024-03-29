#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "c_rust_test.h"

static void build_ip_addr(char *ip_addr_str, int increment, unsigned int *prefix)
{
	struct in_addr in;
	inet_aton(ip_addr_str, &in);
	*prefix = htonl(ntohl(in.s_addr) + increment);
}

static void setup_ip_addr(IpAddrC *addr, unsigned int *prefix)
{
	addr->family = 1;
	addr->addr = (uint8_t *)prefix;
}

static void setup_peer_entry(PeerEntry *peer_entry, unsigned int *prefix, unsigned int out_ifindex)
{
	peer_entry->prefix.family = 1;
	peer_entry->prefix.addr = (uint8_t *)prefix;
	peer_entry->out_ifindex = out_ifindex;
}

static void setup_forwarding_entry(ForwardingEntry *forwarding_entry, unsigned int *prefix, unsigned int out_ifindex)
{
	forwarding_entry->next_hop.family = 1;
	forwarding_entry->next_hop.addr = (uint8_t *)prefix;
	forwarding_entry->out_ifindex = out_ifindex;
}

static void setup_route_entry(RouteEntry *route_entry, unsigned int *prefix, unsigned int *mask, unsigned int *next_hop, unsigned int out_ifindex)
{
	route_entry->prefix.family = 1;
	route_entry->prefix.addr = (uint8_t *)prefix;
	route_entry->mask.family = 1;
	route_entry->mask.addr = (uint8_t *)mask;
	route_entry->next_hop.family = 1;
	route_entry->next_hop.addr = (uint8_t *)next_hop;
	route_entry->out_ifindex = out_ifindex;
}

#define PEER_ENTRY_TEST_INITIAL_PEER "1.1.1.1"
#define PEER_ENTRY_TEST_PEER_NUMBER 1000
#define PEER_ENTRY_TEST_INITIAL_IFINDEX 3

int c_rust_peer_entry_test()
{
	int rc = 0;
	PeerEntry entry;
	IpAddrC ip_addr;
	unsigned int current_peer_prefix;
	unsigned int dummy = 0;

	init_logger();

	for (int i = 0; i < PEER_ENTRY_TEST_PEER_NUMBER; i++) {
		build_ip_addr(PEER_ENTRY_TEST_INITIAL_PEER,i, &current_peer_prefix);
		setup_peer_entry(&entry, &current_peer_prefix, PEER_ENTRY_TEST_INITIAL_IFINDEX + 1);
		rc = peer_add_modify(&entry.prefix, &entry);
		if (rc != 0)
			return rc;
		setup_peer_entry(&entry, &dummy, 0);
		setup_ip_addr(&ip_addr, &current_peer_prefix);
		rc = peer_lookup(&ip_addr,&entry);
		if (rc != 0)
			return rc;
		rc = peer_delete(&ip_addr);
		if (rc != 0)
			return rc;
	}
	for (int i = 0; i < PEER_ENTRY_TEST_PEER_NUMBER; i++) {
		build_ip_addr(PEER_ENTRY_TEST_INITIAL_PEER,i, &current_peer_prefix);
		setup_ip_addr(&ip_addr, &current_peer_prefix);
		setup_peer_entry(&entry, &current_peer_prefix, PEER_ENTRY_TEST_INITIAL_IFINDEX + 1);
		rc = peer_add_modify(&ip_addr, &entry);
		if (rc != 0)
			return rc;
		setup_peer_entry(&entry, &dummy, 0);
		rc = peer_lookup(&ip_addr,&entry);
		if (rc != 0)
			return rc;
	}
	for (int i = 0; i < PEER_ENTRY_TEST_PEER_NUMBER; i++) {
		build_ip_addr(PEER_ENTRY_TEST_INITIAL_PEER,i, &current_peer_prefix);
		setup_ip_addr(&ip_addr, &current_peer_prefix);
		setup_peer_entry(&entry, &dummy, 0);
		rc = peer_lookup(&ip_addr,&entry);
		if (rc != 0)
			return rc;
		rc = peer_delete(&ip_addr);
		if (rc != 0)
			return rc;
	}
	return rc;
}

#define PEER_ROUTE_ENTRY_TEST1_INITIAL_PEER "10.0.0.1"
#define PEER_ROUTE_ENTRY_TEST1_PEER_NUMBER 1000
#define PEER_ROUTE_ENTRY_TEST1_INITIAL_IFINDEX 3000
#define PEER_ROUTE_ENTRY_TEST1_INITIAL_ROUTE "10.10.0.1"
#define PEER_ROUTE_ENTRY_TEST1_ROUTE_NUMBER 1000

int c_rust_peer_route_entry_test1()
{
	int rc = 0;
	PeerEntry peer_entry;
	IpAddrC ip_addr;
	unsigned int current_peer_prefix;
	unsigned int current_route_prefix;
	unsigned int dummy = 0;

	init_logger();

	for (int i = 0; i < PEER_ROUTE_ENTRY_TEST1_PEER_NUMBER; i++ ) {
		build_ip_addr(PEER_ROUTE_ENTRY_TEST1_INITIAL_PEER,i, &current_peer_prefix);
		setup_ip_addr(&ip_addr, &current_peer_prefix);
		setup_peer_entry(&peer_entry, &current_peer_prefix, PEER_ROUTE_ENTRY_TEST1_INITIAL_IFINDEX + i);
		rc = peer_add_modify(&ip_addr, &peer_entry);
		if (rc != 0)
			return rc;
		setup_peer_entry(&peer_entry, &dummy, 0);
		rc = peer_lookup(&ip_addr,&peer_entry);
		if (rc != 0)
			return rc;
		RouteEntry route_entry;
		for (int j = 0; j < PEER_ROUTE_ENTRY_TEST1_ROUTE_NUMBER; j++) {
			unsigned int mask = 0xFFFFFF00;
			unsigned int next_hop = 10;
			build_ip_addr(PEER_ROUTE_ENTRY_TEST1_INITIAL_ROUTE,j, &current_route_prefix);
			setup_route_entry(&route_entry, &current_route_prefix, &mask, &next_hop, PEER_ROUTE_ENTRY_TEST1_INITIAL_IFINDEX + i);
			rc = peer_route_add_modify(&peer_entry.prefix,&route_entry);
			if (rc != 0)
				return rc;
			next_hop = 0;
			setup_route_entry(&route_entry, &current_route_prefix, &mask, &next_hop, 0);
			rc = peer_route_lookup(&peer_entry.prefix,&route_entry.prefix, &route_entry);
			if (rc != 0)
				return rc;
		}
	}
	for (int i = 0; i < PEER_ROUTE_ENTRY_TEST1_PEER_NUMBER; i++ ) {
		build_ip_addr(PEER_ROUTE_ENTRY_TEST1_INITIAL_PEER,i, &current_peer_prefix);
		setup_ip_addr(&ip_addr, &current_peer_prefix);
		for (int j = 0; j < PEER_ROUTE_ENTRY_TEST1_ROUTE_NUMBER; j++) {
			unsigned int mask = 0xFFFFFF00;
			unsigned int next_hop = 10;
			RouteEntry route_entry;
			build_ip_addr(PEER_ROUTE_ENTRY_TEST1_INITIAL_ROUTE,j, &current_route_prefix);
			setup_route_entry(&route_entry, &current_route_prefix, &mask, &next_hop, 0);
			rc = peer_route_delete(&ip_addr,&route_entry.prefix);
			if (rc != 0)
				return rc;
		}
		rc = peer_delete(&ip_addr);
		if (rc != 0)
			return rc;
	}
	return rc;
}

#define PEER_ROUTE_ENTRY_TEST2_INITIAL_PEER "2.0.0.1"
#define PEER_ROUTE_ENTRY_TEST2_PEER_NUMBER 2
#define PEER_ROUTE_ENTRY_TEST2_INITIAL_IFINDEX 3000
#define PEER_ROUTE_ENTRY_TEST2_INITIAL_ROUTE "20.0.0.1"
#define PEER_ROUTE_ENTRY_TEST2_ROUTE_NUMBER 1

int c_rust_peer_route_entry_test2()
{
	int rc = 0;
	PeerEntry peer_entry;
	IpAddrC ip_addr;
	unsigned int dummy = 0;
	unsigned int current_peer_prefix;

	init_logger();

	build_ip_addr(PEER_ROUTE_ENTRY_TEST2_INITIAL_PEER,0, &current_peer_prefix);
	setup_ip_addr(&ip_addr, &current_peer_prefix);
	setup_peer_entry(&peer_entry, &current_peer_prefix, 3);
	rc = peer_add_modify(&ip_addr, &peer_entry);
	if (rc != 0)
		return rc;
	setup_peer_entry(&peer_entry, &dummy, 0);

	rc = peer_lookup(&ip_addr,&peer_entry);
	if (rc != 0)
		return rc;
	RouteEntry route_entry;
	unsigned int current_route_prefix;
	unsigned int mask = 0xFFFFFF00;
	unsigned int next_hop = 10;
	build_ip_addr(PEER_ROUTE_ENTRY_TEST2_INITIAL_ROUTE,0, &current_route_prefix);
	setup_route_entry(&route_entry, &current_route_prefix, &mask, &next_hop, PEER_ROUTE_ENTRY_TEST2_INITIAL_IFINDEX);
	
	rc = peer_route_add_modify(&ip_addr,&route_entry);
	if (rc != 0)
		return rc;
	rc = peer_route_lookup(&ip_addr,&route_entry.prefix, &route_entry);
	if (rc != 0)
		return rc;
	// adding one more peer
	build_ip_addr(PEER_ROUTE_ENTRY_TEST2_INITIAL_PEER,1, &current_peer_prefix);
	setup_ip_addr(&ip_addr, &current_peer_prefix);
	setup_peer_entry(&peer_entry, &current_peer_prefix, PEER_ROUTE_ENTRY_TEST2_INITIAL_IFINDEX);
	rc = peer_add_modify(&ip_addr, &peer_entry);
	if (rc != 0)
		return rc;
	rc = peer_route_add_modify(&ip_addr,&route_entry);
	if (rc != 0)
		return rc;
	build_ip_addr(PEER_ROUTE_ENTRY_TEST2_INITIAL_PEER,1, &current_peer_prefix);
	build_ip_addr(PEER_ROUTE_ENTRY_TEST2_INITIAL_ROUTE,0, &current_route_prefix);
	rc = peer_route_lookup(&peer_entry.prefix,&route_entry.prefix, &route_entry);
	if (rc != 0)
		return rc;
	build_ip_addr(PEER_ROUTE_ENTRY_TEST2_INITIAL_PEER,0, &current_peer_prefix);
	build_ip_addr(PEER_ROUTE_ENTRY_TEST2_INITIAL_ROUTE,0, &current_route_prefix);
	rc = peer_route_delete(&ip_addr,&route_entry.prefix);
	if (rc != 0)
		return rc;
	build_ip_addr(PEER_ROUTE_ENTRY_TEST2_INITIAL_PEER,0, &current_peer_prefix);
	rc = peer_delete(&ip_addr);
	if (rc != 0)
		return rc;
	build_ip_addr(PEER_ROUTE_ENTRY_TEST2_INITIAL_ROUTE,0, &current_route_prefix);
	rc = route_lookup(&route_entry.prefix, &route_entry);
	if (rc != 0)
		return rc;
	build_ip_addr(PEER_ROUTE_ENTRY_TEST2_INITIAL_PEER,1, &current_peer_prefix);
	rc = peer_delete(&ip_addr);
	if (rc != 0)
		return rc;
	build_ip_addr(PEER_ROUTE_ENTRY_TEST2_INITIAL_ROUTE,0, &current_route_prefix);
	rc = route_lookup(&route_entry.prefix, &route_entry);
	if (rc == 0)
		return -1;
	return 0;
}

#define PEER_ENTRY_TEST2_INITIAL_PEER "30.0.0.1"
#define PEER_ENTRY_TEST2_PEER_NUMBER 1
#define PEER_ENTRY_TEST2_INITIAL_IFINDEX1 3
#define PEER_ENTRY_TEST2_INITIAL_IFINDEX2 5

int c_rust_peer_entry_test2()
{
	int rc = 0;
	PeerEntry peer_entry;
	IpAddrC ip_addr;
	unsigned int current_peer_prefix;

	init_logger();

	for (int i = 0; i < PEER_ENTRY_TEST2_PEER_NUMBER; i++) {
		build_ip_addr(PEER_ENTRY_TEST2_INITIAL_PEER,i, &current_peer_prefix);
		setup_ip_addr(&ip_addr, &current_peer_prefix);
		setup_peer_entry(&peer_entry, &current_peer_prefix, PEER_ENTRY_TEST2_INITIAL_IFINDEX1);
		rc = peer_add_modify(&ip_addr, &peer_entry);
		if (rc != 0)
			return rc;

		rc = peer_lookup(&ip_addr,&peer_entry);
		if (rc != 0)
			return rc;
		if (peer_entry.out_ifindex != PEER_ENTRY_TEST2_INITIAL_IFINDEX1)
			return -1;
	}
	for (int i = 0; i < PEER_ENTRY_TEST2_PEER_NUMBER; i++) {
		build_ip_addr(PEER_ENTRY_TEST2_INITIAL_PEER,i, &current_peer_prefix);
		setup_ip_addr(&ip_addr, &current_peer_prefix);
		setup_peer_entry(&peer_entry, &current_peer_prefix, PEER_ENTRY_TEST2_INITIAL_IFINDEX2);
		rc = peer_add_modify(&ip_addr, &peer_entry);
		if (rc != 1)
			return -1;

		rc = peer_lookup(&ip_addr,&peer_entry);
		if (rc != 0)
			return rc;
		if (peer_entry.out_ifindex != PEER_ENTRY_TEST2_INITIAL_IFINDEX2)
			return -1;
	}
	for (int i = 0; i < PEER_ENTRY_TEST2_PEER_NUMBER; i++) {
		build_ip_addr(PEER_ENTRY_TEST2_INITIAL_PEER,i, &current_peer_prefix);
		setup_ip_addr(&ip_addr, &current_peer_prefix);
		rc = peer_lookup(&ip_addr,&peer_entry);
		if (rc != 0)
			return rc;
		rc = peer_delete(&ip_addr);
		if (rc != 0)
			return rc;
	}
	return rc;
}

#define PEER_ROUTE_ENTRY_TEST3_INITIAL_PEER "40.0.0.1"
#define PEER_ROUTE_ENTRY_TEST3_PEER_NUMBER 2
#define PEER_ROUTE_ENTRY_TEST3_INITIAL_IFINDEX 3000
#define PEER_ROUTE_ENTRY_TEST3_MODIFIED_IFINDEX 5000
#define PEER_ROUTE_ENTRY_TEST3_INITIAL_NEXT_HOP 10
#define PEER_ROUTE_ENTRY_TEST3_MODIFIED_NEXT_HOP 30
#define PEER_ROUTE_ENTRY_TEST3_INITIAL_ROUTE "200.0.0.1"
#define PEER_ROUTE_ENTRY_TEST3_ROUTE_NUMBER 1

int c_rust_peer_route_entry_test3()
{
	int rc = 0;
	PeerEntry peer_entry;
	IpAddrC ip_addr;
	unsigned int current_peer_prefix;

	init_logger();

	build_ip_addr(PEER_ROUTE_ENTRY_TEST3_INITIAL_PEER,0, &current_peer_prefix);
	setup_ip_addr(&ip_addr, &current_peer_prefix);
	setup_peer_entry(&peer_entry, &current_peer_prefix, 3);
	printf("adding first peer\n");
	rc = peer_add_modify(&ip_addr, &peer_entry);
	if (rc != 0)
		return rc;
	printf("done. lookup...\n");
	
	rc = peer_lookup(&ip_addr,&peer_entry);
	if (rc != 0)
		return rc;
	printf("done\n");
	RouteEntry route_entry;
	unsigned int current_route_prefix;
	unsigned int mask = 0xFFFFFF00;
	unsigned int next_hop = PEER_ROUTE_ENTRY_TEST3_INITIAL_NEXT_HOP;
	build_ip_addr(PEER_ROUTE_ENTRY_TEST3_INITIAL_ROUTE,0, &current_route_prefix);
	setup_route_entry(&route_entry, &current_route_prefix, &mask, &next_hop, PEER_ROUTE_ENTRY_TEST3_INITIAL_IFINDEX);
	printf("adding route to first peer\n");
	rc = peer_route_add_modify(&ip_addr,&route_entry);
	if (rc != 0)
		return rc;

	rc = peer_route_lookup(&ip_addr,&route_entry.prefix, &route_entry);
	if (rc != 0)
		return rc;
	// adding one more peer
	build_ip_addr(PEER_ROUTE_ENTRY_TEST3_INITIAL_PEER,1, &current_peer_prefix);
	printf("adding second peer\n");
	rc = peer_add_modify(&ip_addr, &peer_entry);
	if (rc != 0)
		return rc;
	printf("adding route to second peer\n");
	rc = peer_route_add_modify(&ip_addr,&route_entry);
	if (rc != 0)
		return rc;

	rc = peer_route_lookup(&ip_addr,&route_entry.prefix, &route_entry);
	if (rc != 0)
		return rc;
	//now modify
	next_hop = PEER_ROUTE_ENTRY_TEST3_MODIFIED_NEXT_HOP;
	setup_route_entry(&route_entry, &current_route_prefix, &mask, &next_hop, PEER_ROUTE_ENTRY_TEST3_MODIFIED_IFINDEX);
	printf("modifying route\n");
	build_ip_addr(PEER_ROUTE_ENTRY_TEST3_INITIAL_PEER,0, &current_peer_prefix);
	rc = peer_route_add_modify(&ip_addr,&route_entry);
	if (rc != 1)
		return rc;
	build_ip_addr(PEER_ROUTE_ENTRY_TEST3_INITIAL_PEER,1, &current_peer_prefix);
	rc = peer_route_lookup(&ip_addr,&route_entry.prefix, &route_entry);
	if (rc != 0)
		return rc;
	next_hop = *((unsigned int *)route_entry.next_hop.addr);
	if ((route_entry.out_ifindex != PEER_ROUTE_ENTRY_TEST3_MODIFIED_IFINDEX) ||
	    (next_hop != PEER_ROUTE_ENTRY_TEST3_MODIFIED_NEXT_HOP)) {
		printf("either next_hop %d or out_ifindex %d is not reflected\n",next_hop,route_entry.out_ifindex);
		return -1;
	}
	printf("deleting peer1 route\n");
	build_ip_addr(PEER_ROUTE_ENTRY_TEST3_INITIAL_PEER,0, &current_peer_prefix);
	rc = peer_route_delete(&ip_addr,&route_entry.prefix);
	if (rc != 0)
		return rc;
	printf("deleting peer1\n");
	rc = peer_delete(&ip_addr);
	if (rc != 0)
		return rc;

	printf("looking up for route\n");
	rc = route_lookup(&route_entry.prefix, &route_entry);
	if (rc != 0)
		return 0;
	printf("deleting peer2\n");
	build_ip_addr(PEER_ROUTE_ENTRY_TEST3_INITIAL_PEER,1, &current_peer_prefix);
	rc = peer_delete(&ip_addr);
	if (rc != 0)
		return rc;

	printf("looking up route\n");
	rc = route_lookup(&route_entry.prefix, &route_entry);
	if (rc == 0)
		return -1;
	return 0;
}
int callbacks_count = 0;
int on_peer(const IpAddrC * ip_addr)
{
	callbacks_count++;
	printf("on_peer %s %d %d\n",__FILE__,__LINE__,callbacks_count);
	return 0;
}

#define PEER_ROUTE_ENTRY_ITERATION_TEST1_INITIAL_PEER "80.0.0.1"
#define PEER_ROUTE_ENTRY_ITERATION_TEST1_PEER_NUMBER 2
#define PEER_ROUTE_ENTRY_ITERATION_TEST1_INITIAL_IFINDEX 3000
#define PEER_ROUTE_ENTRY_ITERATION_TEST1_MODIFIED_IFINDEX 5000
#define PEER_ROUTE_ENTRY_ITERATION_TEST1_INITIAL_NEXT_HOP 10
#define PEER_ROUTE_ENTRY_ITERATION_TEST1_MODIFIED_NEXT_HOP 30
#define PEER_ROUTE_ENTRY_ITERATION_TEST1_INITIAL_ROUTE "200.200.0.1"
#define PEER_ROUTE_ENTRY_ITERATION_TEST1_ROUTE_NUMBER 1

int c_rust_peer_route_entry_iteration_test1()
{
	int rc = 0;
	PeerEntry peer_entry;
	IpAddrC ip_addr;
	unsigned int current_peer_prefix;

	init_logger();

	build_ip_addr(PEER_ROUTE_ENTRY_ITERATION_TEST1_INITIAL_PEER,0, &current_peer_prefix);
	setup_ip_addr(&ip_addr, &current_peer_prefix);
	setup_peer_entry(&peer_entry, &current_peer_prefix, 3);
	printf("adding first peer\n");
	rc = peer_add_modify(&ip_addr, &peer_entry);
	if (rc != 0)
		return rc;
	printf("done. lookup...\n");
	
	rc = peer_lookup(&ip_addr,&peer_entry);
	if (rc != 0)
		return rc;
	printf("done\n");
	// adding one more peer
	build_ip_addr(PEER_ROUTE_ENTRY_ITERATION_TEST1_INITIAL_PEER,1, &current_peer_prefix);
	printf("adding second peer\n");
	rc = peer_add_modify(&ip_addr, &peer_entry);
	if (rc != 0)
		return rc;
	peer_iterate(1);
	build_ip_addr(PEER_ROUTE_ENTRY_ITERATION_TEST1_INITIAL_PEER,0, &current_peer_prefix);
	rc = peer_delete(&ip_addr);
	if (rc != 0)
		return rc;
	build_ip_addr(PEER_ROUTE_ENTRY_ITERATION_TEST1_INITIAL_PEER,1, &current_peer_prefix);
	rc = peer_delete(&ip_addr);
	if (rc != 0)
		return rc;
	return callbacks_count == 2 ? 0 : -1;
}

#define PREFIX_TREE_TEST1_INITIAL_PREFIX "180.0.0.1"
#define PREFIX_TREE_TEST1_INITIAL_IFINDEX 3000
#define PREFIX_TREE_TEST1_INITIAL_NEXT_HOP "1.1.1.1"
#define PREFIX_TREE_TEST1_ROUTE_NUMBER 10

int c_rust_prefix_tree_test1()
{
	int rc = 0;
	ForwardingEntry forwarding_entry;
	IpAddrC ip_addr;
	unsigned int current_prefix;
	unsigned int current_next_hop;

	init_logger();

	for (int i = 0; i < PREFIX_TREE_TEST1_ROUTE_NUMBER; i++) {
		build_ip_addr(PREFIX_TREE_TEST1_INITIAL_PREFIX,i, &current_prefix);
		setup_ip_addr(&ip_addr, &current_prefix);
		build_ip_addr(PREFIX_TREE_TEST1_INITIAL_NEXT_HOP,0, &current_next_hop);
		setup_forwarding_entry(&forwarding_entry, &current_next_hop, PREFIX_TREE_TEST1_INITIAL_IFINDEX);
		if (longest_match_add(&ip_addr, &forwarding_entry) != 0) {
			printf("failed here %s %d\n",__FILE__,__LINE__);
			return -1;
		}
	}
	for (int i = 0; i < PREFIX_TREE_TEST1_ROUTE_NUMBER; i++) {
		build_ip_addr(PREFIX_TREE_TEST1_INITIAL_PREFIX,i, &current_prefix);
		setup_ip_addr(&ip_addr, &current_prefix);
		build_ip_addr(PREFIX_TREE_TEST1_INITIAL_NEXT_HOP,0, &current_next_hop);
		setup_forwarding_entry(&forwarding_entry, &current_next_hop, 0);
		if (longest_match_lookup(&ip_addr, &forwarding_entry) != 0) {
			printf("failed here %s %d\n",__FILE__,__LINE__);
			return -1;
		}
		if (forwarding_entry.out_ifindex != PREFIX_TREE_TEST1_INITIAL_IFINDEX) {
			printf("failed here %s %d\n",__FILE__,__LINE__);
			return -1;
		}
	}
	for (int i = 0; i < PREFIX_TREE_TEST1_ROUTE_NUMBER; i++) {
		build_ip_addr(PREFIX_TREE_TEST1_INITIAL_PREFIX,i, &current_prefix);
		setup_ip_addr(&ip_addr, &current_prefix);
		if (longest_match_delete(&ip_addr) != 0) {
			printf("failed here %s %d\n",__FILE__,__LINE__);
			return -1;
		}
	}
	return 0;
}

#define FTN_TEST1_ENTRIES_NUMBER 1
#define FTN_TEST1_INITIAL_PREFIX "1.1.1.1"
#define FTN_TEST1_INITIAL_NEXT_HOP "2.2.2.2"
#define FTN_TEST1_INITIAL_IFINDEX 1
#define FTN_TEST1_INITIAL_FTN_IX 1

void setup_ftn_entry_add(FtnAddData *ftn_add_data, unsigned int *current_label, unsigned int ifindex, unsigned int ftn_ix)
{
	ftn_add_data->out_ifindex = ifindex;
	ftn_add_data->out_label_number = 1;
	ftn_add_data->out_label = current_label;
	ftn_add_data->ftn_ix = ftn_ix;
}

void setup_ftn_entry_del(FtnDelData *ftn_del_data, unsigned int ftn_ix)
{
	ftn_del_data->ftn_ix = ftn_ix;
}

int c_rust_ftn_test1()
{
	int rc = 0;
	FtnAddData ftn_add_data;
	FtnDelData ftn_del_data;
	unsigned int current_prefix;
	unsigned int current_next_hop;
	unsigned int current_label;

	init_logger();

	for (int i = 0; i < FTN_TEST1_ENTRIES_NUMBER; i++) {
		build_ip_addr(FTN_TEST1_INITIAL_PREFIX,i, &current_prefix);
		setup_ip_addr(&ftn_add_data.fec, &current_prefix);
		build_ip_addr(FTN_TEST1_INITIAL_NEXT_HOP,0, &current_next_hop);
		setup_ip_addr(&ftn_add_data.next_hop, &current_next_hop);
		setup_ftn_entry_add(&ftn_add_data, &current_label, FTN_TEST1_INITIAL_IFINDEX, FTN_TEST1_INITIAL_FTN_IX + i);
		if (ftn_add(&ftn_add_data) != 0) {
			printf("failed here %s %d\n",__FILE__,__LINE__);
			return -1;
		}
	}
	for (int i = 0; i < FTN_TEST1_ENTRIES_NUMBER; i++) {
		build_ip_addr(FTN_TEST1_INITIAL_PREFIX,i, &current_prefix);
		setup_ip_addr(&ftn_del_data.fec, &current_prefix);
		setup_ftn_entry_del(&ftn_del_data, FTN_TEST1_INITIAL_FTN_IX + i);
		if (ftn_del(&ftn_del_data) != 0) {
			printf("failed here %s %d\n",__FILE__,__LINE__);
			return -1;
		}
	}
	for (int i = 0; i < FTN_TEST1_ENTRIES_NUMBER; i++) {
		build_ip_addr(FTN_TEST1_INITIAL_PREFIX,i, &current_prefix);
		setup_ip_addr(&ftn_add_data.fec, &current_prefix);
		build_ip_addr(FTN_TEST1_INITIAL_NEXT_HOP,0, &current_next_hop);
		setup_ip_addr(&ftn_add_data.next_hop, &current_next_hop);
		setup_ftn_entry_add(&ftn_add_data, &current_label, FTN_TEST1_INITIAL_IFINDEX, FTN_TEST1_INITIAL_FTN_IX + i);
		if (ftn_add(&ftn_add_data) != 0) {
			printf("failed here %s %d\n",__FILE__,__LINE__);
			return -1;
		}
		build_ip_addr(FTN_TEST1_INITIAL_PREFIX,i, &current_prefix);
		setup_ip_addr(&ftn_del_data.fec, &current_prefix);
		setup_ftn_entry_del(&ftn_del_data, FTN_TEST1_INITIAL_FTN_IX + i);
		if (ftn_del(&ftn_del_data) != 0) {
			printf("failed here %s %d\n",__FILE__,__LINE__);
			return -1;
		}
	}
	return 0;
}