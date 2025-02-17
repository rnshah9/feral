/*
	MIT License

	Copyright (c) 2020 Feral Language repositories

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so.
*/

#include "Compiler/CodeGen/Internal.hpp"

Errors parse_var_decl(phelper_t &ph, stmt_base_t *&loc)
{
	std::vector<const stmt_var_decl_base_t *> decls;
	stmt_base_t *decl = nullptr;

	size_t idx = ph.peak()->pos;
	if(!ph.accept(TOK_LET)) {
		err::set(E_PARSE_FAIL, ph.peak()->pos,
			 "expected keyword 'let' here, but found: '%s'", TokStrs[ph.peakt()]);
		goto fail;
	}
	ph.next();

begin:
	if(parse_var_decl_base(ph, decl) != E_OK) {
		goto fail;
	}
	decls.push_back((stmt_var_decl_base_t *)decl);
	decl = nullptr;

	if(ph.accept(TOK_COMMA)) {
		ph.next();
		goto begin;
	}

	if(!ph.accept(TOK_COLS)) {
		err::set(E_PARSE_FAIL, ph.peak(-1)->pos,
			 "expected semicolon after variable declaration, found: '%s'",
			 TokStrs[ph.peakt()]);
		goto fail;
	}
	ph.next();
	loc = new stmt_var_decl_t(decls, idx);
	return E_OK;
fail:
	for(auto &decl : decls) delete decl;
	return E_PARSE_FAIL;
}

Errors parse_var_decl_base(phelper_t &ph, stmt_base_t *&loc)
{
	const lex::tok_t *lhs = nullptr;
	stmt_base_t *in = nullptr, *rhs = nullptr;

	// iden index
	size_t idx = ph.peak()->pos;

	// the variable in which data is to be stored, must be a const str
	ph.sett(TOK_STR);
	lhs = ph.peak();
	ph.next();

	if(ph.peakt() == TOK_IN) {
		ph.next();
		// 01 = parenthesized expression, func call, subscript, dot
		if(parse_expr_01(ph, in) != E_OK) {
			goto fail;
		}
	}

	if(!ph.accept(TOK_ASSN)) {
		err::set(E_PARSE_FAIL, ph.peak()->pos,
			 "expected assignment operator here for var decl, but found: '%s'",
			 TokStrs[ph.peakt()]);
		goto fail;
	}
	ph.next();

	// 15 = everything excluding comma
	if(parse_expr_15(ph, rhs) != E_OK) {
		goto fail;
	}

	loc = new stmt_var_decl_base_t(new stmt_simple_t(lhs), in, rhs);
	return E_OK;
fail:
	if(in) delete in;
	if(rhs) delete rhs;
	return E_PARSE_FAIL;
}