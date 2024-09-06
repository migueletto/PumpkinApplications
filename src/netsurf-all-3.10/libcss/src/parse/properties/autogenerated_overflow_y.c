/*
 * This file was generated by LibCSS gen_parser 
 * 
 * Generated from:
 *
 * overflow_y:CSS_PROP_OVERFLOW_Y IDENT:( INHERIT: VISIBLE:0,OVERFLOW_VISIBLE HIDDEN:0,OVERFLOW_HIDDEN SCROLL:0,OVERFLOW_SCROLL AUTO:0,OVERFLOW_AUTO  IDENT:)
 * 
 * Licensed under the MIT License,
 *		  http://www.opensource.org/licenses/mit-license.php
 * Copyright 2010 The NetSurf Browser Project.
 */

#include <assert.h>
#include <string.h>

#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "parse/properties/properties.h"
#include "parse/properties/utils.h"

/**
 * Parse overflow_y
 *
 * \param c	  Parsing context
 * \param vector  Vector of tokens to process
 * \param ctx	  Pointer to vector iteration context
 * \param result  resulting style
 * \return CSS_OK on success,
 *	   CSS_NOMEM on memory exhaustion,
 *	   CSS_INVALID if the input is not valid
 *
 * Post condition: \a *ctx is updated with the next token to process
 *		   If the input is invalid, then \a *ctx remains unchanged.
 */
css_error css__parse_overflow_y(css_language *c,
		const parserutils_vector *vector, int *ctx,
		css_style *result)
{
	int orig_ctx = *ctx;
	css_error error;
	const css_token *token;
	bool match;

	token = parserutils_vector_iterate(vector, ctx);
	if ((token == NULL) || ((token->type != CSS_TOKEN_IDENT))) {
		*ctx = orig_ctx;
		return CSS_INVALID;
	}

	if ((lwc_string_caseless_isequal(token->idata, c->strings[INHERIT], &match) == lwc_error_ok && match)) {
			error = css_stylesheet_style_inherit(result, CSS_PROP_OVERFLOW_Y);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[VISIBLE], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_OVERFLOW_Y, 0,OVERFLOW_VISIBLE);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[HIDDEN], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_OVERFLOW_Y, 0,OVERFLOW_HIDDEN);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[SCROLL], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_OVERFLOW_Y, 0,OVERFLOW_SCROLL);
	} else if ((lwc_string_caseless_isequal(token->idata, c->strings[AUTO], &match) == lwc_error_ok && match)) {
			error = css__stylesheet_style_appendOPV(result, CSS_PROP_OVERFLOW_Y, 0,OVERFLOW_AUTO);
	} else {
		error = CSS_INVALID;
	}

	if (error != CSS_OK)
		*ctx = orig_ctx;
	
	return error;
}

