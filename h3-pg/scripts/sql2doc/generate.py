"""
The concept:

We don't want to manually keep API docs in sync with SQL files.

So instead we should generate the docs from the sql/install/*.sql files.

We have the following things to document:
    * Type
    * Cast (put in same section as the type)
    * Operator
    * Function
    * Aggregate function

Some functions do not need docs, we can signal this by not applying a comment.
This is internal operator functions for example.

We should parse the sql files, extract the above, and then document all things that
have comments applied.

We should probably hardcode a list of things that should not be documented, so we can
fail if new function are undocumented.

"""

import argparse
import glob
import re
from pathlib import Path
from lark import Lark, Transformer, v_args, visitors
import sys #TEST

# see PGXN::API::Indexer::_clean_html_body
def to_anchor(text):
    text = re.sub(r"[^\w\s\(\)-]", "", text) # approximating processing done in Text::Markdown
    text = re.sub(r"^([^a-zA-Z])", r"L\1", text)
    text = re.sub(r"[^a-zA-Z0-9_:.-]+", ".", text)
    return "#" + text

def md_heading(text, level=0):
    return "#" * (level + 1) + " " + text

class Context:
    level = 0

    def __init__(self, statements_by_refid={}):
        self.statements_by_refid = statements_by_refid
        self.level = 0

class StmtBase:
    level = 0
    newline_before = True
    newline_after = True
    decorators = {}

    def __init__(self, level=-1):
        self.level = level

    def set_decorators(self, decorators):
        self.decorators = decorators

    def get_decorator(self, key):
        return self.decorators[key] if self.has_decorator(key) else None

    def has_decorator(self, key):
        return key in self.decorators

    def is_internal(self):
        return self.has_decorator("internal")

    def is_deprecated(self):
        return self.has_decorator("deprecated")

    def is_visible(self):
        return not self.is_internal() and not self.is_deprecated()

    def get_refid(self):
        return self.get_decorator("refid")

    def get_ref_text(self):
        return str(self)

    def get_anchor(self):
        if not self.is_visible() or self.level < 0:
            return None
        return to_anchor(str(self))

    def get_refs(self, statements_by_refid):
        ids = self.get_decorator("ref") or []
        return [statements_by_refid[refid] for refid in ids if refid in statements_by_refid]

    def to_ref(self):
        return '<a href="{}">{}</a>'.format(self.get_anchor(), self.get_ref_text())

    def to_md(self, context):
        if not self.is_visible():
            return ""

        md = str(self)
        if self.level > -1:
            md = md_heading(md, self.level)

        if self.newline_before:
            md = "\n" + md

        # availability
        availability = self.get_decorator("availability")
        if availability:
            md += f"\n*Since v{availability}*"

        # references
        refs = self.get_refs(context.statements_by_refid)
        if len(refs) > 0:
            md += "\n\nSee also: "
            md += ", ".join([ref.to_ref() for ref in refs])

        if self.newline_after:
            md += "\n"

        return md

    def __str__(self):
        raise Execepion("Not implemented")


class Argument:
    def __init__(self, argmode, name, argtype, default):
        self.argmode = argmode
        self.name = name
        self.argtype = argtype
        self.default = default

    def get_typestr(self):
        s = "`{}`".format(self.argtype)
        if self.default:
            s = "[{} = {}]".format(s, self.default)
        return s

    def __str__(self):
        s = ""
        if self.argmode:
            s += self.argmode + " "
        if self.name:
            s += self.name + " "
        s += "`{}`".format(self.argtype)
        if self.default:
            s = "[{} = {}]".format(s, self.default)
        return s


class Column:
    def __init__(self, name, coltype):
        self.name = name;
        self.coltype = coltype;

    def __str__(self):
        return "{} `{}`".format(self.name, self.coltype)

class FunctionReturnsBase:
    pass


class FunctionReturns(FunctionReturnsBase):
    def __init__(self, rettype):
        self.rettype = rettype

    def __str__(self):
        return "`{}`".format(self.rettype)


class FunctionReturnsSet(FunctionReturns):
    def __str__(self):
        return "SETOF " + super().__str__()


class FunctionReturnsTable(FunctionReturnsBase):
    def __init__(self, columns):
        self.columns = columns

    def __str__(self):
        return "TABLE (" + ", ".join([str(col) for col in self.columns]) + ")";


class CustomMd(StmtBase):
    def __init__(self, line):
        super().__init__()
        self.newline_after = False
        match = re.match(r"(#+)\s*(.*)", line)
        if match:
            self.newline_before = True
            self.level = max(0, len(match[1]) - 1)
            self.line = match[2]
        else:
            self.newline_before = False
            self.level = -1
            self.line = line

    def update_level(self, context):
        if self.level > -1:
            context.level = max(0, self.level)

    def __str__(self):
        return self.line


class CreateFunctionStmt(StmtBase):
    def __init__(self, name: str, arguments, returns: FunctionReturnsBase):
        super().__init__(2)
        self.name = name
        self.arguments = arguments or []
        self.returns = returns

    def get_ref_text(self):
        return "{}({})".format(
            self.name,
            ", ".join([arg.get_typestr() for arg in self.arguments]))

    def __str__(self):
        return "{}({}) ⇒ {}".format(
            self.name,
            ", ".join([str(arg) for arg in self.arguments]),
            self.returns)


class CreateAggregateStmt(StmtBase):
    def __init__(self, name: str, arguments):
        super().__init__(2)
        self.name = name
        self.arguments = arguments or []

    def get_ref_text(self):
        return "{}(setof {})".format(
            self.name,
            ", ".join([arg.get_typestr() for arg in self.arguments]))

    def __str__(self):
        return "{}(setof {})".format(
            self.name,
            ", ".join([str(arg) for arg in self.arguments]))


class CreateTypeStmt(StmtBase):
    def __str__(self):
        return ""

class CreateCastStmt(StmtBase):
    def __init__(self, source, target):
        super().__init__(2)
        self.source = source
        self.target = target

    def __str__(self):
        return "`{}` :: `{}`".format(self.source, self.target)


class CreateOperatorStmt(StmtBase):
    def __init__(self, name, left, right):
        super().__init__(2)
        self.name = name
        self.left = left
        self.right = right

    def __str__(self):
        return "Operator: `{}` {} `{}`".format(self.left, self.name, self.right)


class CreateCommentStmt(StmtBase):
    def __init__(self, text):
        super().__init__()
        self.text = text

    def __str__(self):
        return self.text


class SQLTransformer(Transformer):
    def start(self, statements):
        # flatten
        flat = [item for sublist in statements for item in sublist]
        return flat

    @v_args(inline=True)
    def custom_decorated_statement(self, decorators, statement = None):
        if not statement:
            raise visitors.Discard()
        if decorators:
            statement.set_decorators(decorators)
        return statement

    def custom_decorators(self, lines):
        decorators = {}
        for line in lines:
            try:
                key, value = line.split(":")
                key = str(key).lower()
                if key in ["ref"]:
                    decorators[key] = [v.strip() for v in value.split(",")]
                else:
                    decorators[key] = value.strip()
            except:
                decorators[str(line).lower()] = True
        return decorators

    def custom_md_statement(self, children):
        return children

    # -- MARKDOWN --------------------------------------------------------------
    @v_args(inline=True)
    def custom_markdown(self, line=""):
        line = re.sub(r"^--\|[ \t]?", '', line)
        return CustomMd(line)

    # -- CREATE TYPE -----------------------------------------------------------
    def create_type_stmt(self, children):
        return CreateTypeStmt()

    # -- CREATE CAST -----------------------------------------------------------
    @v_args(inline=True)
    def create_cast_stmt(self, source, target, *children):
        return CreateCastStmt(source, target)

    # -- CREATE OPERATOR -------------------------------------------------------
    @v_args(inline=True)
    def create_oper_stmt(self, name, options):
        return CreateOperatorStmt(name, options['LEFTARG'], options['RIGHTARG'])

    def create_oper_opts(self, opts):
        return {key: value for [key, value] in opts}

    @v_args(inline=True)
    def create_oper_opt(self, option, value=None):
        return [str(option), value]

    # -- CREATE FUNCTION -------------------------------------------------------
    @v_args(inline=True)
    def create_fun_ret_table_columns(self, columns):
        return FunctionReturnsTable(columns)

    def create_fun_rettype(self, children):
        rettype = children[1]
        if children[0] is not None:
            return FunctionReturnsSet(rettype)
        return FunctionReturns(rettype)

    def column_list(self, children):
        return children

    @v_args(inline=True)
    def column(self, name: str, coltype: str):
        return Column(name, coltype)

    @v_args(inline=True)
    def create_func_stmt(self, name: str, arguments, returns, *opts):
        # skip internal functions
        if name.startswith("__"):
            raise visitors.Discard()
        return CreateFunctionStmt(name, arguments, returns)

    # -- CREATE AGGREGATE ------------------------------------------------------
    @v_args(inline=True)
    def create_agg_stmt(self, name: str, arguments, *params):
        return CreateAggregateStmt(name, arguments)

    # -- CREATE COMMENT --------------------------------------------------------
    @v_args(inline=True)
    def comment_on_stmt(self, child, text):
        return CreateCommentStmt(text)

    # -- FUNCTION OR AGGREGATE ARGUMENTS ---------------------------------------
    def argument_list(self, children):
        return children

    @v_args(inline=True)
    def argument(self, argmode, name, argtype, default=None):
        return Argument(argmode, name, argtype, default)

    # -- CREATE OPERATOR CLASS -------------------------------------------------
    def create_opcl_stmt(self, children):
        raise visitors.Discard()

    # -- SIMPLE RULES ----------------------------------------------------------

    true = lambda self, _: "`true`"
    false = lambda self, _: "`false`"
    number = v_args(inline=True)(int)

    def fun_name(self, children):
        return children[1]

    @v_args(inline=True)
    def string(self, s):
        return s[1:-1].replace('\\"', '"')

    # -- TERMINALS -------------------------------------------------------------

    def SIGNED_NUMBER(self, children):
        return int(children)

    def CNAME(self, cname):
        return str(cname)

    def OPERATOR(self, name):
        return str(name)

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--group', '-g',
        nargs=2, metavar=('header', 'glob'),
        action='append',
        help='Globs for each group of SQL install files')
    return parser.parse_args()

def create_parser():
    here = Path(__file__).parent
    parser = Lark.open(
        here / "sql.lark",
        parser="lalr",
        maybe_placeholders=True)
    return parser

def process_group(parser, group, statements_by_refid):
    [header, files_glob] = group
    statements = []
    paths = glob.glob(files_glob)
    for path in sorted(paths):
        group_statements = parse_file(parser, path)
        # Add referenced statements to map
        for statement in group_statements:
            refid = statement.get_refid()
            if refid:
                statements_by_refid[refid] = statement
        statements += group_statements
    return (header, statements)

# return flat list of statements in file
def parse_file(parser, path):
    with open(path) as fd:
        sql = fd.read()
        tree = parser.parse(sql)
        statements = SQLTransformer(visit_tokens=True).transform(tree)
        return statements

def statements_to_md(statements, context):
    items = [stmt.to_md(context) for stmt in statements]
    return [item for item in items if len(item) > 0]

def main():
    args = parse_args()
    parser = create_parser()

    groups = []
    statements_by_refid = {}

    # Process groups
    for item in args.group:
        group = process_group(parser, item, statements_by_refid)
        groups.append(group)

    # Output
    md = ""
    for group in groups:
        [header, statements] = group
        md += md_heading(header) + "\n"
        md += "\n".join(statements_to_md(statements, Context(statements_by_refid))) + "\n"

    print(md)

if __name__ == "__main__":
    main()
