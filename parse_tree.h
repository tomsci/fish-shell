/**\file parse_tree.h

    Programmatic representation of fish code.
*/

#ifndef FISH_PARSE_TREE_H
#define FISH_PARSE_TREE_H

#include <wchar.h>

#include "config.h"
#include "util.h"
#include "common.h"
#include "tokenizer.h"
#include <vector>

#define PARSE_ASSERT(a) assert(a)
#define PARSER_DIE() exit_without_destructors(-1)

class parse_node_t;
class parse_node_tree_t;
typedef size_t node_offset_t;
#define NODE_OFFSET_INVALID (static_cast<node_offset_t>(-1))

struct parse_error_t
{
    /** Text of the error */
    wcstring text;

    /** Offset and length of the token in the source code that triggered this error */
    size_t source_start;
    size_t source_length;

    /** Return a string describing the error, suitable for presentation to the user */
    wcstring describe(const wcstring &src) const;
};
typedef std::vector<parse_error_t> parse_error_list_t;

class parse_ll_t;
class parse_t
{
    parse_ll_t * const parser;

public:
    parse_t();
    bool parse(const wcstring &str, parse_node_tree_t *output, parse_error_list_t *errors);
};

enum parse_token_type_t
{
    token_type_invalid,

    // Non-terminal tokens
    symbol_job_list,
    symbol_job,
    symbol_job_continuation,
    symbol_statement,
    symbol_block_statement,
    symbol_block_header,
    symbol_for_header,
    symbol_while_header,
    symbol_begin_header,
    symbol_function_header,

    symbol_if_statement,
    symbol_if_clause,
    symbol_else_clause,
    symbol_else_continuation,

    symbol_switch_statement,
    symbol_case_item_list,
    symbol_case_item,

    symbol_boolean_statement,
    symbol_decorated_statement,
    symbol_plain_statement,
    symbol_arguments_or_redirections_list,
    symbol_argument_or_redirection,

    symbol_argument_list_nonempty,
    symbol_argument_list,
    
    symbol_optional_background,

    // Terminal types
    parse_token_type_string,
    parse_token_type_pipe,
    parse_token_type_redirection,
    parse_token_type_background,
    parse_token_type_end,
    parse_token_type_terminate,

    FIRST_PARSE_TOKEN_TYPE = parse_token_type_string
};

enum parse_keyword_t
{
    parse_keyword_none,
    parse_keyword_if,
    parse_keyword_else,
    parse_keyword_for,
    parse_keyword_in,
    parse_keyword_while,
    parse_keyword_begin,
    parse_keyword_function,
    parse_keyword_switch,
    parse_keyword_case,
    parse_keyword_end,
    parse_keyword_and,
    parse_keyword_or,
    parse_keyword_not,
    parse_keyword_command,
    parse_keyword_builtin
};

wcstring token_type_description(parse_token_type_t type);
wcstring keyword_description(parse_keyword_t type);

/** Base class for nodes of a parse tree */
class parse_node_t
{
public:

    /* Type of the node */
    enum parse_token_type_t type;

    /* Start in the source code */
    size_t source_start;

    /* Length of our range in the source code */
    size_t source_length;

    /* Children */
    node_offset_t child_start;
    node_offset_t child_count;

    /* Type-dependent data */
    uint32_t tag;

    /* Description */
    wcstring describe(void) const;

    /* Constructor */
    explicit parse_node_t(parse_token_type_t ty) : type(ty), source_start(0), source_length(0), child_start(0), child_count(0), tag(0)
    {
    }

    node_offset_t child_offset(node_offset_t which) const
    {
        PARSE_ASSERT(which < child_count);
        return child_start + which;
    }
};

class parse_node_tree_t : public std::vector<parse_node_t>
{
};


/* Fish grammar:

# A job_list is a list of jobs, separated by semicolons or newlines

    job_list = <empty> |
                <TOK_END> job_list |
                job job_list

# A job is a non-empty list of statements, separated by pipes. (Non-empty is useful for cases like if statements, where we require a command). To represent "non-empty", we require a statement, followed by a possibly empty job_continuation

    job = statement job_continuation
    job_continuation = <empty> |
                       <TOK_PIPE> statement job_continuation

# A statement is a normal command, or an if / while / and etc

    statement = boolean_statement | block_statement | if_statement | switch_statement | decorated_statement

# A block is a conditional, loop, or begin/end

    if_statement = if_clause else_clause <END> arguments_or_redirections_list
    if_clause = <IF> job STATEMENT_TERMINATOR job_list
    else_clause = <empty> |
                 <ELSE> else_continuation
    else_continuation = if_clause else_clause |
                        STATEMENT_TERMINATOR job_list

    switch_statement = SWITCH <TOK_STRING> STATEMENT_TERMINATOR case_item_list <END>
    case_item_list = <empty> |
                    case_item case_item_list
    case_item = CASE argument_list STATEMENT_TERMINATOR job_list

    argument_list_nonempty = <TOK_STRING> argument_list
    argument_list = <empty> | argument_list_nonempty

    block_statement = block_header <TOK_END> job_list <END> arguments_or_redirections_list
    block_header = for_header | while_header | function_header | begin_header
    for_header = FOR var_name IN arguments_or_redirections_list
    while_header = WHILE statement
    begin_header = BEGIN
    function_header = FUNCTION function_name argument_list

# A boolean statement is AND or OR or NOT

    boolean_statement = AND statement | OR statement | NOT statement

# A decorated_statement is a command with a list of arguments_or_redirections, possibly with "builtin" or "command"

    decorated_statement = COMMAND plain_statement | BUILTIN plain_statement | plain_statement
    plain_statement = COMMAND arguments_or_redirections_list optional_background

    arguments_or_redirections_list = <empty> |
                                     argument_or_redirection arguments_or_redirections_list
    argument_or_redirection = redirection | <TOK_STRING>
    redirection = <TOK_REDIRECTION>
    
    terminator = <TOK_END> | <TOK_BACKGROUND>
 
    optional_background = <empty> | <TOK_BACKGROUND>

*/

#endif