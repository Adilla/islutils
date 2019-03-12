/// These are automatically generated conversions between
/// the default and the checked C++ bindings for isl.
///
/// isl is a library for computing with integer sets and maps described by
/// Presburger formulas. On top of this, isl provides various tools for
/// polyhedral compilation, ranging from dependence analysis over scheduling
/// to AST generation.

#ifndef ISL_CPP_CHECKED_CONVERSION
#define ISL_CPP_CHECKED_CONVERSION

#include <isl/cpp.h>
#include <isl/cpp-checked.h>

namespace isl {

checked::aff check(aff obj) {
	return checked::manage(obj.copy());
}

aff uncheck(checked::aff obj) {
	return manage(obj.copy());
}

checked::aff_list check(aff_list obj) {
	return checked::manage(obj.copy());
}

aff_list uncheck(checked::aff_list obj) {
	return manage(obj.copy());
}

checked::ast_build check(ast_build obj) {
	return checked::manage(obj.copy());
}

ast_build uncheck(checked::ast_build obj) {
	return manage(obj.copy());
}

checked::ast_expr check(ast_expr obj) {
	return checked::manage(obj.copy());
}

ast_expr uncheck(checked::ast_expr obj) {
	return manage(obj.copy());
}

checked::ast_expr_list check(ast_expr_list obj) {
	return checked::manage(obj.copy());
}

ast_expr_list uncheck(checked::ast_expr_list obj) {
	return manage(obj.copy());
}

checked::ast_node check(ast_node obj) {
	return checked::manage(obj.copy());
}

ast_node uncheck(checked::ast_node obj) {
	return manage(obj.copy());
}

checked::ast_node_list check(ast_node_list obj) {
	return checked::manage(obj.copy());
}

ast_node_list uncheck(checked::ast_node_list obj) {
	return manage(obj.copy());
}

checked::basic_map check(basic_map obj) {
	return checked::manage(obj.copy());
}

basic_map uncheck(checked::basic_map obj) {
	return manage(obj.copy());
}

checked::basic_map_list check(basic_map_list obj) {
	return checked::manage(obj.copy());
}

basic_map_list uncheck(checked::basic_map_list obj) {
	return manage(obj.copy());
}

checked::basic_set check(basic_set obj) {
	return checked::manage(obj.copy());
}

basic_set uncheck(checked::basic_set obj) {
	return manage(obj.copy());
}

checked::basic_set_list check(basic_set_list obj) {
	return checked::manage(obj.copy());
}

basic_set_list uncheck(checked::basic_set_list obj) {
	return manage(obj.copy());
}

checked::constraint check(constraint obj) {
	return checked::manage(obj.copy());
}

constraint uncheck(checked::constraint obj) {
	return manage(obj.copy());
}

checked::constraint_list check(constraint_list obj) {
	return checked::manage(obj.copy());
}

constraint_list uncheck(checked::constraint_list obj) {
	return manage(obj.copy());
}

checked::fixed_box check(fixed_box obj) {
	return checked::manage(obj.copy());
}

fixed_box uncheck(checked::fixed_box obj) {
	return manage(obj.copy());
}

checked::id check(id obj) {
	return checked::manage(obj.copy());
}

id uncheck(checked::id obj) {
	return manage(obj.copy());
}

checked::id_list check(id_list obj) {
	return checked::manage(obj.copy());
}

id_list uncheck(checked::id_list obj) {
	return manage(obj.copy());
}

checked::id_to_ast_expr check(id_to_ast_expr obj) {
	return checked::manage(obj.copy());
}

id_to_ast_expr uncheck(checked::id_to_ast_expr obj) {
	return manage(obj.copy());
}

checked::local_space check(local_space obj) {
	return checked::manage(obj.copy());
}

local_space uncheck(checked::local_space obj) {
	return manage(obj.copy());
}

checked::map check(map obj) {
	return checked::manage(obj.copy());
}

map uncheck(checked::map obj) {
	return manage(obj.copy());
}

checked::map_list check(map_list obj) {
	return checked::manage(obj.copy());
}

map_list uncheck(checked::map_list obj) {
	return manage(obj.copy());
}

checked::mat check(mat obj) {
	return checked::manage(obj.copy());
}

mat uncheck(checked::mat obj) {
	return manage(obj.copy());
}

checked::multi_aff check(multi_aff obj) {
	return checked::manage(obj.copy());
}

multi_aff uncheck(checked::multi_aff obj) {
	return manage(obj.copy());
}

checked::multi_pw_aff check(multi_pw_aff obj) {
	return checked::manage(obj.copy());
}

multi_pw_aff uncheck(checked::multi_pw_aff obj) {
	return manage(obj.copy());
}

checked::multi_union_pw_aff check(multi_union_pw_aff obj) {
	return checked::manage(obj.copy());
}

multi_union_pw_aff uncheck(checked::multi_union_pw_aff obj) {
	return manage(obj.copy());
}

checked::multi_val check(multi_val obj) {
	return checked::manage(obj.copy());
}

multi_val uncheck(checked::multi_val obj) {
	return manage(obj.copy());
}

checked::point check(point obj) {
	return checked::manage(obj.copy());
}

point uncheck(checked::point obj) {
	return manage(obj.copy());
}

checked::pw_aff check(pw_aff obj) {
	return checked::manage(obj.copy());
}

pw_aff uncheck(checked::pw_aff obj) {
	return manage(obj.copy());
}

checked::pw_aff_list check(pw_aff_list obj) {
	return checked::manage(obj.copy());
}

pw_aff_list uncheck(checked::pw_aff_list obj) {
	return manage(obj.copy());
}

checked::pw_multi_aff check(pw_multi_aff obj) {
	return checked::manage(obj.copy());
}

pw_multi_aff uncheck(checked::pw_multi_aff obj) {
	return manage(obj.copy());
}

checked::pw_multi_aff_list check(pw_multi_aff_list obj) {
	return checked::manage(obj.copy());
}

pw_multi_aff_list uncheck(checked::pw_multi_aff_list obj) {
	return manage(obj.copy());
}

checked::pw_qpolynomial check(pw_qpolynomial obj) {
	return checked::manage(obj.copy());
}

pw_qpolynomial uncheck(checked::pw_qpolynomial obj) {
	return manage(obj.copy());
}

checked::pw_qpolynomial_fold_list check(pw_qpolynomial_fold_list obj) {
	return checked::manage(obj.copy());
}

pw_qpolynomial_fold_list uncheck(checked::pw_qpolynomial_fold_list obj) {
	return manage(obj.copy());
}

checked::pw_qpolynomial_list check(pw_qpolynomial_list obj) {
	return checked::manage(obj.copy());
}

pw_qpolynomial_list uncheck(checked::pw_qpolynomial_list obj) {
	return manage(obj.copy());
}

checked::qpolynomial check(qpolynomial obj) {
	return checked::manage(obj.copy());
}

qpolynomial uncheck(checked::qpolynomial obj) {
	return manage(obj.copy());
}

checked::schedule check(schedule obj) {
	return checked::manage(obj.copy());
}

schedule uncheck(checked::schedule obj) {
	return manage(obj.copy());
}

checked::schedule_constraints check(schedule_constraints obj) {
	return checked::manage(obj.copy());
}

schedule_constraints uncheck(checked::schedule_constraints obj) {
	return manage(obj.copy());
}

checked::schedule_node check(schedule_node obj) {
	return checked::manage(obj.copy());
}

schedule_node uncheck(checked::schedule_node obj) {
	return manage(obj.copy());
}

checked::set check(set obj) {
	return checked::manage(obj.copy());
}

set uncheck(checked::set obj) {
	return manage(obj.copy());
}

checked::set_list check(set_list obj) {
	return checked::manage(obj.copy());
}

set_list uncheck(checked::set_list obj) {
	return manage(obj.copy());
}

checked::space check(space obj) {
	return checked::manage(obj.copy());
}

space uncheck(checked::space obj) {
	return manage(obj.copy());
}

checked::term check(term obj) {
	return checked::manage(obj.copy());
}

term uncheck(checked::term obj) {
	return manage(obj.copy());
}

checked::union_access_info check(union_access_info obj) {
	return checked::manage(obj.copy());
}

union_access_info uncheck(checked::union_access_info obj) {
	return manage(obj.copy());
}

checked::union_flow check(union_flow obj) {
	return checked::manage(obj.copy());
}

union_flow uncheck(checked::union_flow obj) {
	return manage(obj.copy());
}

checked::union_map check(union_map obj) {
	return checked::manage(obj.copy());
}

union_map uncheck(checked::union_map obj) {
	return manage(obj.copy());
}

checked::union_map_list check(union_map_list obj) {
	return checked::manage(obj.copy());
}

union_map_list uncheck(checked::union_map_list obj) {
	return manage(obj.copy());
}

checked::union_pw_aff check(union_pw_aff obj) {
	return checked::manage(obj.copy());
}

union_pw_aff uncheck(checked::union_pw_aff obj) {
	return manage(obj.copy());
}

checked::union_pw_aff_list check(union_pw_aff_list obj) {
	return checked::manage(obj.copy());
}

union_pw_aff_list uncheck(checked::union_pw_aff_list obj) {
	return manage(obj.copy());
}

checked::union_pw_multi_aff check(union_pw_multi_aff obj) {
	return checked::manage(obj.copy());
}

union_pw_multi_aff uncheck(checked::union_pw_multi_aff obj) {
	return manage(obj.copy());
}

checked::union_pw_multi_aff_list check(union_pw_multi_aff_list obj) {
	return checked::manage(obj.copy());
}

union_pw_multi_aff_list uncheck(checked::union_pw_multi_aff_list obj) {
	return manage(obj.copy());
}

checked::union_pw_qpolynomial check(union_pw_qpolynomial obj) {
	return checked::manage(obj.copy());
}

union_pw_qpolynomial uncheck(checked::union_pw_qpolynomial obj) {
	return manage(obj.copy());
}

checked::union_set check(union_set obj) {
	return checked::manage(obj.copy());
}

union_set uncheck(checked::union_set obj) {
	return manage(obj.copy());
}

checked::union_set_list check(union_set_list obj) {
	return checked::manage(obj.copy());
}

union_set_list uncheck(checked::union_set_list obj) {
	return manage(obj.copy());
}

checked::val check(val obj) {
	return checked::manage(obj.copy());
}

val uncheck(checked::val obj) {
	return manage(obj.copy());
}

checked::val_list check(val_list obj) {
	return checked::manage(obj.copy());
}

val_list uncheck(checked::val_list obj) {
	return manage(obj.copy());
}

checked::vec check(vec obj) {
	return checked::manage(obj.copy());
}

vec uncheck(checked::vec obj) {
	return manage(obj.copy());
}

} // namespace isl

#endif /* ISL_CPP_CHECKED_CONVERSION */
