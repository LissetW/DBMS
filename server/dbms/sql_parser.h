#pragma once

#define MAX_TOKENS 32
#define MAX_STR 128

enum sql_type {
    SQL_UNKNOWN = 0,
    SQL_SELECT,
    SQL_INSERT,
    SQL_UPDATE,
    SQL_DELETE,
    SQL_MAX
};

/* Simple structures to hold parsed data */

struct sql_select {
    char table[MAX_STR];
};

struct sql_insert {
    char table[MAX_STR];
    int value_count;
    char values[MAX_TOKENS][MAX_STR];
};

struct sql_update {
    char table[MAX_STR];
    char set_col[MAX_STR];
    char set_val[MAX_STR];
    char where_col[MAX_STR];
    char where_val[MAX_STR];
};

struct sql_delete {
    char table[MAX_STR];
    char where_col[MAX_STR];
    char where_val[MAX_STR];
};

struct sql_query {
    enum sql_type type;

    union {
        struct sql_select select_q;
        struct sql_insert insert_q;
        struct sql_update update_q;
        struct sql_delete delete_q;
    } data;
};

/**
 * @brief Parse a simple SQL statement into a structured representation.
 * @param input SQL statement string to parse. Allowed forms
 *        - SELECT * FROM <table>
 *        - INSERT INTO <table> VALUES (v1,v2,...)
 *        - UPDATE <table> SET col=val WHERE col=val
 *        - DELETE FROM <table> WHERE col=val
 * @param out Pointer to an allocated struct sql_query which will be populated on success.
 *
 * @return int Returns SUCCESS on successful parse and ERROR on parse error.
 */
int parse_sql(const char *input, struct sql_query *out);