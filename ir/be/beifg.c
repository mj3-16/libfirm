/**
 * @file   beifg.c
 * @date   18.11.2005
 * @author Sebastian Hack
 *
 * Copyright (C) 2005 Universitaet Karlsruhe
 * Released under the GPL
 */

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include "bitset.h"

#include "irnode_t.h"
#include "irprintf.h"
#include "irtools.h"
#include "beifg_t.h"

size_t (be_ifg_nodes_iter_size)(const void *self)
{
	const be_ifg_t *ifg = self;
	return ifg->impl->nodes_iter_size;
}

size_t (be_ifg_neighbours_iter_size)(const void *self)
{
	const be_ifg_t *ifg = self;
	return ifg->impl->neighbours_iter_size;
}

size_t (be_ifg_cliques_iter_size)(const void *self)
{
	const be_ifg_t *ifg = self;
	return ifg->impl->cliques_iter_size;
}

void (be_ifg_free)(void *self)
{
	be_ifg_t *ifg = self;
	ifg->impl->free(self);
}

int (be_ifg_connected)(const void *self, const ir_node *a, const ir_node *b)
{
	const be_ifg_t *ifg = self;
	return ifg->impl->connected(self, a, b);
}

ir_node *(be_ifg_neighbours_begin)(const void *self, void *iter, const ir_node *irn)
{
	const be_ifg_t *ifg = self;
	return ifg->impl->neighbours_begin(self, iter, irn);
}

ir_node *(be_ifg_neighbours_next)(const void *self, void *iter)
{
	const be_ifg_t *ifg = self;
	return ifg->impl->neighbours_next(self, iter);
}

void (be_ifg_neighbours_break)(const void *self, void *iter)
{
	const be_ifg_t *ifg = self;
	ifg->impl->neighbours_break(self, iter);
}

ir_node *(be_ifg_nodes_begin)(const void *self, void *iter)
{
	const be_ifg_t *ifg = self;
	return ifg->impl->nodes_begin(self, iter);
}

ir_node *(be_ifg_nodes_next)(const void *self, void *iter)
{
	const be_ifg_t *ifg = self;
	return ifg->impl->nodes_next(self, iter);
}

void (be_ifg_nodes_break)(const void *self, void *iter)
{
	const be_ifg_t *ifg = self;
	ifg->impl->nodes_break(self, iter);
}

int (be_ifg_cliques_begin)(const void *self, void *iter, ir_node **buf)
{
	const be_ifg_t *ifg = self;
	return ifg->impl->cliques_begin(self, iter, buf);
}

int (be_ifg_cliques_next)(const void *self, void *iter)
{
	const be_ifg_t *ifg = self;
	return ifg->impl->cliques_next(self, iter);
}

void (be_ifg_cliques_break)(const void *self, void *iter)
{
	const be_ifg_t *ifg = self;
	ifg->impl->cliques_break(self, iter);
}

int (be_ifg_degree)(const void *self, const ir_node *irn)
{
	const be_ifg_t *ifg = self;
	return ifg->impl->degree(self, irn);
}


int be_ifg_is_simplicial(const be_ifg_t *ifg, const ir_node *irn)
{
	int degree = be_ifg_degree(ifg, irn);
	void *iter = be_ifg_neighbours_iter_alloca(ifg);

	ir_node **neighbours = xmalloc(degree * sizeof(neighbours[0]));

	ir_node *curr;
	int i, j;

	i = 0;
	be_ifg_foreach_neighbour(ifg, iter, irn, curr)
		neighbours[i++] = curr;

	for(i = 0; i < degree; ++i) {
		for(j = 0; j < i; ++j)
			if(!be_ifg_connected(ifg, neighbours[i], neighbours[j])) {
				free(neighbours);
				return 0;
			}
	}


	free(neighbours);
	return 1;
}

void be_ifg_check(const be_ifg_t *ifg)
{
	void *iter1 = be_ifg_nodes_iter_alloca(ifg);
	void *iter2 = be_ifg_neighbours_iter_alloca(ifg);

	ir_node *n, *m;
	int node_count = 0;
	int neighbours_count = 0;
	int degree = 0;

	/* count all nodes */
	ir_printf("\n\nFound the following nodes in the graph %+F:\n\n", current_ir_graph);
	be_ifg_foreach_node(ifg,iter1,n)
	{
		node_count++;
		degree = be_ifg_degree(ifg, n);
		ir_printf("%d. %+F with degree: %d\n", node_count, n, degree);
	}

	ir_printf("\n\nNumber of nodes: %d\n\n", node_count);

	/* Check, if all neighbours are indeed connected to the node. */
	be_ifg_foreach_node(ifg, iter1, n)
	{
		ir_printf("\n%+F; ", n);
		be_ifg_foreach_neighbour(ifg, iter2, n, m)
		{
			ir_printf("%+F; ", m);
			neighbours_count++;
			if(!be_ifg_connected(ifg, n, m))
				ir_fprintf(stderr, "%+F is a neighbour of %+F but they are not connected!\n", n, m);
		}
	}
	ir_printf("\n\nFound %d nodes in the 'check neighbour section'\n", neighbours_count);
}

int be_ifg_check_get_node_count(const be_ifg_t *ifg)
{
	void *iter = be_ifg_nodes_iter_alloca(ifg);
	int node_count = 0;
	ir_node *n;

	be_ifg_foreach_node(ifg, iter, n)
	{
		node_count++;
	}

	return node_count;
}

static int be_ifg_check_cmp_nodes(const void *a, const void *b)
{
	const ir_node *node_a = *(ir_node **)a;
	const ir_node *node_b = *(ir_node **)b;

	int nr_a = node_a->node_nr;
	int nr_b = node_b->node_nr;

	return QSORT_CMP(nr_a, nr_b);
}

void be_ifg_check_sorted(const be_ifg_t *ifg, FILE *f)
{
	void *iter1 = be_ifg_nodes_iter_alloca(ifg);
	void *iter2 = be_ifg_neighbours_iter_alloca(ifg);

	ir_node *n, *m;
	const int node_count = be_ifg_check_get_node_count(ifg);
	int neighbours_count = 0;
	int i = 0;

	ir_node **all_nodes = xmalloc(node_count * sizeof(all_nodes[0]));

	be_ifg_foreach_node(ifg, iter1, n)
	{
		all_nodes[i] = n;
		i++;
	}

	qsort(all_nodes, node_count, sizeof(all_nodes[0]), be_ifg_check_cmp_nodes);

	for (i = 0; i < node_count; i++)
	{
		ir_node **neighbours = xmalloc(node_count * sizeof(neighbours[0]));
		int j = 0;
		int k = 0;
		int degree = 0;

		degree = be_ifg_degree(ifg, all_nodes[i]);

		be_ifg_foreach_neighbour(ifg, iter2, all_nodes[i], m)
		{
			neighbours[j] = m;
			j++;
		}

		qsort(neighbours, j, sizeof(neighbours[0]), be_ifg_check_cmp_nodes);

		ir_fprintf(f, "%d. %+F's neighbours(%d): ", i+1, all_nodes[i], degree);

		for(k = 0; k < j; k++)
		{
			ir_fprintf(f, "%+F, ", neighbours[k]);
		}

		ir_fprintf(f, "\n");

		free(neighbours);
	}

	free(all_nodes);

}

void be_ifg_dump_dot(be_ifg_t *ifg, ir_graph *irg, FILE *file, const be_ifg_dump_dot_cb_t *cb, void *self)
{
	void *nodes_it  = be_ifg_nodes_iter_alloca(ifg);
	void *neigh_it  = be_ifg_neighbours_iter_alloca(ifg);
	bitset_t *nodes = bitset_malloc(get_irg_last_idx(irg));

	ir_node *n, *m;

	fprintf(file, "graph G {\n\tgraph [");
	if(cb->graph_attr)
		cb->graph_attr(file, self);
	fprintf(file, "];\n");

	if(cb->at_begin)
		cb->at_begin(file, self);

	be_ifg_foreach_node(ifg, nodes_it, n) {
		if(cb->is_dump_node && cb->is_dump_node(self, n)) {
			int idx = get_irn_idx(n);
			bitset_set(nodes, idx);
			fprintf(file, "\tnode [");
			if(cb->node_attr)
				cb->node_attr(file, self, n);
			fprintf(file, "]; n%d;\n", idx);
		}
	}

	/* Check, if all neighbours are indeed connected to the node. */
	be_ifg_foreach_node(ifg, nodes_it, n) {
		be_ifg_foreach_neighbour(ifg, neigh_it, n, m) {
			int n_idx = get_irn_idx(n);
			int m_idx = get_irn_idx(m);

			if(n_idx < m_idx && bitset_is_set(nodes, n_idx) && bitset_is_set(nodes, m_idx)) {
				fprintf(file, "\tn%d -- n%d [", n_idx, m_idx);
				if(cb->edge_attr)
					cb->edge_attr(file, self, n, m);
				fprintf(file, "];\n");
			}
		}
	}

	if(cb->at_end)
		cb->at_end(file, self);

	fprintf(file, "}\n");
	bitset_free(nodes);
}
