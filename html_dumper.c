#include "adt/array.h"
#include "assert.h"

#include "pbqp_edge_t.h"
#include "pbqp_node_t.h"
#include "html_dumper.h"
#include "kaps.h"
#include "pbqp_t.h"

/* print vector */
static void dump_vector(FILE *f, vector *vec)
{
	unsigned index;
	assert(vec);

	fprintf(f, "<span class=\"vector\">( ");
	unsigned len = vec->len;
	assert(len> 0);
	for (index = 0; index < len; ++index) {
		fprintf(f, "%6d", vec->entries[index].data);
	}
	fprintf(f, " )</span>\n");
}

static void dump_matrix(FILE *f, pbqp_matrix *mat)
{
	unsigned row, col;
	assert(mat);
	num *p = mat->entries;

	assert(mat->cols > 0);
	assert(mat->rows > 0);
	fprintf(f, "\t\\begin{pmatrix}\n");
	for (row = 0; row < mat->rows; ++row) {
		fprintf(f, "\t %6d", *p++);
		for (col = 1; col < mat->cols; ++col) {
			fprintf(f, "& %6d", *p++);
		}
		fprintf(f, "\\\\\n");
	}
	fprintf(f, "\t\\end{pmatrix}\n");
}

static void dump_edge_costs(pbqp *pbqp)
{
	unsigned src_index;

	assert(pbqp);
	assert(pbqp->dump_file);

	fputs("<p>", pbqp->dump_file);
	for (src_index = 0; src_index < pbqp->num_nodes; ++src_index) {
		pbqp_node *src_node = get_node(pbqp, src_index);
		unsigned edge_index;
		unsigned len = ARR_LEN(src_node->edges);
		for (edge_index = 0; edge_index < len; ++edge_index) {
			pbqp_edge *edge = src_node->edges[edge_index];
			unsigned tgt_index = edge->tgt;
			if (src_index < tgt_index) {
				fputs("<tex>\n", pbqp->dump_file);
				fprintf(pbqp->dump_file, "\t\\overline\n{C}_{%d,%d}=\n",
						src_index, tgt_index);
				dump_matrix(pbqp->dump_file, edge->costs);
				fputs("</tex><br>", pbqp->dump_file);
			}
		}
	}
	fputs("</p>", pbqp->dump_file);
}

static void dump_node_costs(pbqp *pbqp)
{
	unsigned index;

	assert(pbqp);
	assert(pbqp->dump_file);

	/* dump node costs */
	fputs("<p>", pbqp->dump_file);
	for (index = 0; index < pbqp->num_nodes; ++index) {
		pbqp_node *node = get_node(pbqp, index);
		fprintf(pbqp->dump_file, "\tc<sub>%d</sub> = ", index);
		dump_vector(pbqp->dump_file, node->costs);
		fputs("<br>\n", pbqp->dump_file);
	}
	fputs("</p>", pbqp->dump_file);
}

static void dump_section(FILE *f, int level, char *txt)
{
	assert(f);

	fprintf(f, "<h%d>%s</h%d>\n", level, txt, level);
}

void dump_graph(pbqp *pbqp)
{
	unsigned src_index;

	assert(pbqp != NULL);
	assert(pbqp->dump_file != NULL);

	fputs("<p>\n<graph>\n\tgraph input {\n", pbqp->dump_file);
	for (src_index = 0; src_index < pbqp->num_nodes; ++src_index) {
		fprintf(pbqp->dump_file, "\t n%d;\n", src_index);
	}

	for (src_index = 0; src_index < pbqp->num_nodes; ++src_index) {
		pbqp_node *node = get_node(pbqp, src_index);
		unsigned len = ARR_LEN(node->edges);
		unsigned edge_index;
		for (edge_index = 0; edge_index < len; ++edge_index) {
			unsigned tgt_index = node->edges[edge_index]->tgt;

			if (src_index < tgt_index) {
				fprintf(pbqp->dump_file, "\t n%d -- n%d;\n", src_index,
						tgt_index);
			}
		}
	}
	fputs("\t}\n</graph>\n</p>\n", pbqp->dump_file);
}

void dump_input(pbqp *pbqp)
{
	assert(pbqp);
	assert(pbqp->dump_file);

	dump_section(pbqp->dump_file, 1, "1. PBQP Problem");
	dump_section(pbqp->dump_file, 2, "1.1 Topology");
	dump_graph(pbqp);
	dump_section(pbqp->dump_file, 2, "1.2 Cost Vectors");
	dump_node_costs(pbqp);
	dump_section(pbqp->dump_file, 2, "1.3 Cost Matrices");
	dump_edge_costs(pbqp);
}
