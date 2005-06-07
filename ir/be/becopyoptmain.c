/**
 * Author:      Daniel Grund
 * Date:		11.04.2005
 * Copyright:   (c) Universitaet Karlsruhe
 * Licence:     This file protected by GPL -  GNU GENERAL PUBLIC LICENSE.

 * Main file for the optimization reducing the copies needed for:
 * - phi coalescing
 * - register-constrained nodes
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "becopyopt.h"
#include "becopystat.h"
#include "becopyoptmain.h"

#define DO_HEUR
#define DO_ILP

#define DEBUG_LVL SET_LEVEL_1
static firm_dbg_module_t *dbg = NULL;

void be_copy_opt_init(void) {
}

void be_copy_opt(be_chordal_env_t *chordal_env) {
	copy_opt_t *co;
	int lb, copies;

	dbg = firm_dbg_register("ir.be.copyopt");
	firm_dbg_set_mask(dbg, DEBUG_LVL);

	co = new_copy_opt(chordal_env);
	DBG((dbg, LEVEL_1, "\n\n    ===>  %s  <===\n\n", co->name));
	co_check_allocation(co);

#ifdef DO_STAT
	copies = co_get_copy_count(co);
	curr_vals[I_COPIES_INIT] += copies;
	DBG((dbg, 1, "Init copies: %3d\n", copies));
#endif

#ifdef DO_HEUR
	co_heur_opt(co);
	co_check_allocation(co);
#ifdef DO_STAT
	copies = co_get_copy_count(co);
	curr_vals[I_COPIES_HEUR] += copies;
	DBG((dbg, 1, "Heur copies: %3d\n", copies));
#endif
	DBG((dbg, 1, "Heur copies: %3d\n", co_get_copy_count(co)));
#endif

#ifdef DO_ILP
	lb = co_get_lower_bound(co);
	copies = co_get_copy_count(co);
//TODO remove checks and enable lb
	if (copies<lb)
		DBG((dbg, 0, "\n\nAt least one computation of these two is boooogy %d < %d\n\n", lb, copies));

//	if (copies > lb) {
		co_ilp_opt(co);
		co_check_allocation(co);
//	}
	copies = co_get_copy_count(co);
	assert(copies>=lb && "At least one computation of these two is boooogy");

#ifdef DO_STAT
	copies = co_get_copy_count(co);
	curr_vals[I_COPIES_HEUR] += copies;
	DBG((dbg, 1, "Opt  copies: %3d\n", copies));
#endif
	DBG((dbg, 1, "Opt  copies: %3d\n", co_get_copy_count(co)));
#endif

	free_copy_opt(co);
}
