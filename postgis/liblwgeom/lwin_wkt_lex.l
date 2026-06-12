%{

/* The lexer */

#include <stdio.h>
#include <string.h>
#include "lwin_wkt.h"
#include "lwin_wkt_parse.h"
#include "lwgeom_log.h"

/*
* Handle errors due to unexpected junk in WKT strings.
* MEOS: reentrant scanner — the location comes from the scanner's yylloc and
* the error is recorded into the calling thread's global_parser_result
* (which is MEOS_TLS, i.e. per-thread).
*/
static void wkt_lexer_unknown(YYLTYPE *llocp)
{
	/* Set the (per-thread) error state */
	global_parser_result.errcode = PARSER_ERROR_OTHER;
	global_parser_result.message = parser_error_messages[PARSER_ERROR_OTHER];
	global_parser_result.errlocation = llocp->last_column;
}

/*
* This macro is magically run after a rule is found but before the main
* action is run. We use it to update the parse location information
* so we can report on where things fail. Also optionally to dump
* debugging info.
* MEOS: reentrant scanner — location/text/length come through the scanner.
*/
#define YY_USER_ACTION do { \
	yylloc->first_line = yylloc->last_line = yyget_lineno(yyscanner); \
	yylloc->first_column = yylloc->last_column; \
	yylloc->last_column += yyget_leng(yyscanner); \
	LWDEBUGF(5,"lex: %s", yyget_text(yyscanner)); \
	} while (0);

/*
* Ensure we have a definition of NAN to use when encountering
* NAN tokens.
*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <math.h>
#ifndef NAN
#define NAN 0.0/0.0
#endif

%}

%option prefix="wkt_yy"
/* MEOS: reentrant scanner so concurrent WKT parses do not share lexer state.
 * bison-bridge/bison-locations pass yylval/yylloc through the scanner. */
%option reentrant bison-bridge bison-locations
/* MEOS: the keyword rules are written in uppercase but WKT is
 * case-insensitive (POINT/Point/point all parse); the committed lexer was
 * generated with this option, so it must stay in the source or a regenerate
 * silently produces a case-sensitive lexer that rejects mixed-case input. */
%option case-insensitive
%option nounput
%option noinput
%option never-interactive
%option outfile="lwin_wkt_lex.c"
%option noyywrap
/* Suppress the default implementations. */
%option noyyalloc noyyrealloc noyyfree

%%

-?(([0-9]+\.?)|([0-9]*\.?[0-9]+)([eE][-+]?[0-9]+)?)[ \,\)\t\n\r] {
	LWDEBUG(5,"DOUBLE");
	yylval->doublevalue = atof(yytext);
	yyless(yyleng-1);
	return DOUBLE_TOK;
	}

([Nn][Aa][Nn])[ \,\)\t\n\r] {
	LWDEBUG(5,"DOUBLE NAN");
	yylval->doublevalue = NAN;
	yyless(yyleng-1);
	return DOUBLE_TOK;
}

SRID=-?[0-9]+ {
	LWDEBUG(5,"SRID");
	yylval->integervalue = wkt_lexer_read_srid(yytext);
	return SRID_TOK;
	}

GEOMETRYCOLLECTION { return COLLECTION_TOK; }
MULTISURFACE { return MSURFACE_TOK; }
MULTIPOLYGON { return MPOLYGON_TOK; }
MULTICURVE { return MCURVE_TOK; }
MULTILINESTRING { return MLINESTRING_TOK; }
MULTIPOINT { return MPOINT_TOK; }
CURVEPOLYGON { return CURVEPOLYGON_TOK; }
POLYGON { return POLYGON_TOK; }
COMPOUNDCURVE { return COMPOUNDCURVE_TOK; }
CIRCULARSTRING { return CIRCULARSTRING_TOK; }
LINESTRING { return LINESTRING_TOK; }
POLYHEDRALSURFACE { return POLYHEDRALSURFACE_TOK; }
TRIANGLE { return TRIANGLE_TOK; }
TIN { return TIN_TOK; }
POINT { return POINT_TOK; }
EMPTY { return EMPTY_TOK; }

Z|M|ZM {
	LWDEBUG(5,"DIMENSIONALITY");
	yylval->stringvalue = yytext;
	return DIMENSIONALITY_TOK;
	}

\( { LWDEBUG(5,"LBRACKET"); return LBRACKET_TOK; }
\) { LWDEBUG(5,"RBRACKET"); return RBRACKET_TOK; }
,  { LWDEBUG(5,"COMMA"); return COMMA_TOK; }
\; { LWDEBUG(5,"SEMICOLON"); return SEMICOLON_TOK; }

[ \t\n\r]+ { /* ignore whitespace */ LWDEBUG(5,"WHITESPACE"); }

. { /* Error out and stop parsing on unknown/unexpected characters */
	LWDEBUG(5,"UNKNOWN");
	wkt_lexer_unknown(yylloc);
	yyterminate();
	}

%%

/* MEOS: reentrant allocators receive the scanner handle. */
void *wkt_yyalloc (yy_size_t  size , void *yyscanner)
{
	(void) yyscanner;
	return (void *) lwalloc( size );
}

void *wkt_yyrealloc  (void * ptr, yy_size_t  size , void *yyscanner)
{
	(void) yyscanner;
	return (void *) lwrealloc( (char *) ptr, size );
}

void wkt_yyfree (void * ptr , void *yyscanner)
{
	(void) yyscanner;
	/* MEOS: the reentrant yylex_destroy() frees the (possibly NULL) buffer
	 * stack through this hook. In the PostgreSQL-extension build lwfree() is
	 * pfree(), and pfree(NULL) dereferences NULL and crashes the backend
	 * (unlike free(NULL), which is a no-op — why the MEOS-only tests passed).
	 * Guard against NULL so freeing an unused buffer is safe in both builds. */
	if ( ptr )
		lwfree( (char *) ptr );	/* see wkt_yyrealloc() for (char *) cast */
}
