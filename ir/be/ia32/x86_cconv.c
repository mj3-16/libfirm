/*
 * This file is part of libFirm.
 * Copyright (C) 2014 University of Karlsruhe.
 */

/**
 * @file
 * @brief   calling convention helpers
 * @author  Matthias Braun
 */
#include <stdlib.h>

#include "irnode_t.h"
#include "iredges_t.h"
#include "ircons.h"
#include "irgmod.h"
#include "x86_cconv.h"

void x86_free_calling_convention(x86_cconv_t *cconv)
{
	free(cconv->parameters);
	free(cconv->results);
	free(cconv->caller_saves);
	free(cconv->callee_saves);
	free(cconv);
}

void x86_create_parameter_loads(ir_graph *irg, const x86_cconv_t *cconv)
{
	ir_node *start       = get_irg_start(irg);
	ir_node *start_block = get_irg_start_block(irg);
	ir_node *nomem       = get_irg_no_mem(irg);
	ir_node *frame       = get_irg_frame(irg);
	ir_node *proj_args   = get_Proj_for_pn(start, pn_Start_T_args);
	foreach_out_edge_safe(proj_args, edge) {
		ir_node *proj = get_edge_src_irn(edge);
		if (!is_Proj(proj))
			continue;
		unsigned pn   = get_Proj_num(proj);
		const reg_or_stackslot_t *param = &cconv->parameters[pn];
		ir_entity *entity = param->entity;
		if (entity == NULL)
			continue;
		ir_type  *const type   = get_entity_type(entity);
		ir_mode  *const mode   = get_type_mode(type);
		dbg_info *const dbgi   = get_irn_dbg_info(proj);
		unsigned        offset = param->offset + 4;
		ir_node  *const c      = new_r_Const_long(irg, mode_Is, offset);
		ir_node  *const add    = new_rd_Add(dbgi, start_block, frame, c);
		ir_node  *const load   = new_rd_Load(dbgi, start_block, nomem, add, mode, type, cons_none);
		ir_node  *const res    = new_r_Proj(load, mode, pn_Load_res);
		exchange(proj, res);
	}
}

void x86_layout_param_entities(ir_graph *const irg, x86_cconv_t *const cconv,
                               int const parameters_offset)
{
	/* construct argument type */
	ir_type    *const frame_type = get_irg_frame_type(irg);
	size_t      const n_params   = cconv->n_parameters;
	ir_entity **const param_map  = ALLOCANZ(ir_entity*, n_params);
	/** Return address is on the stack after that comes the parameters */

	for (size_t f = get_compound_n_members(frame_type); f-- > 0; ) {
		ir_entity *member = get_compound_member(frame_type, f);
		if (!is_parameter_entity(member))
			continue;

		size_t num = get_entity_parameter_number(member);
		assert(num < n_params);
		if (param_map[num] != NULL)
			panic("multiple entities for parameter %u in %+F found", f, irg);
		param_map[num] = member;
	}

	/* Create missing entities and set offsets */
	for (size_t p = 0; p < n_params; ++p) {
		reg_or_stackslot_t *param = &cconv->parameters[p];
		if (param->type == NULL)
			continue;

		ir_entity *entity = param_map[p];
		if (entity == NULL)
			entity = new_parameter_entity(frame_type, p, param->type);
		param->entity = entity;
		/* Adjust for return address on stack */
		int const offset = param->offset + parameters_offset;
		set_entity_offset(param->entity, offset);
	}

	ir_entity *const entity        = get_irg_entity(irg);
	ir_type   *const function_type = get_entity_type(entity);
	if (is_method_variadic(function_type)) {
		ir_type   *unknown       = get_unknown_type();
		ident     *id            = new_id_from_str("$va_start");
		ir_entity *va_start_addr = new_entity(frame_type, id, unknown);
		int const offset = cconv->param_stacksize + parameters_offset;
		set_entity_offset(va_start_addr, offset);
		cconv->va_start_addr = va_start_addr;
	}
}
