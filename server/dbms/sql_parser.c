#include "sql_parser.h"
#include "common.h"

#include <ctype.h>

// Removes spaces at start/end
static void trim(char *s) {
    char *start = s;
    int len = strlen(start);

    // trim end
    while (len > 0 && isspace((unsigned char)start[len - 1]))
        start[--len] = '\0';

    // trim start
    while (*start && isspace((unsigned char)*start))
        ++start, --len;

    memmove(s, start, len + 1);
}

// Splits by spaces, commas, parentheses
static int tokenize(char *s, char out_tokens[][MAX_STR]) {
    int token_count = 0;
    char *token_ptr = strtok(s, " (),\n\t\r");

    while (token_ptr && token_count < MAX_TOKENS) {
        strncpy(out_tokens[token_count++], token_ptr, MAX_STR);
        token_ptr = strtok(NULL, " (),\n\t\r");
    }
    return token_count;
}

// Main parser
int parse_sql(const char *input, struct sql_query *out) {
    char input_copy[512];
    char token_list[MAX_TOKENS][MAX_STR];
    int token_count, idx;

    memset(out, 0, sizeof(*out));

    strncpy(input_copy, input, sizeof(input_copy));
    input_copy[sizeof(input_copy) - 1] = '\0';

    trim(input_copy);

    token_count = tokenize(input_copy, token_list);
    if (token_count == 0)
        return ERROR;

    // SELECT * FROM table
    if (strcmp(token_list[0], "SELECT") == 0) {
        if (token_count == 4 && strcmp(token_list[1], "*") == 0
            && strcmp(token_list[2], "FROM") == 0) {

            out->type = SQL_SELECT;
            strncpy(out->data.select_q.table, token_list[3], MAX_STR);
            return SUCCESS;
        }
        return ERROR;
    }

    // INSERT INTO table VALUES (...)
    if (strcmp(token_list[0], "INSERT") == 0
        && strcmp(token_list[1], "INTO") == 0) {

        out->type = SQL_INSERT;
        strncpy(out->data.insert_q.table, token_list[2], MAX_STR);

        // Find VALUES token
        int pos = -1;
        for (idx = 3; idx < token_count; ++idx) {
            if (strcmp(token_list[idx], "VALUES") == 0) {
                pos = idx + 1;
                break;
            }
        }
        if (pos == -1)
            return ERROR;

        // Remaining tokens are values
        out->data.insert_q.value_count = 0;
        for (idx = pos; idx < token_count; ++idx) {
            strncpy(out->data.insert_q.values[out->data.insert_q.value_count++], token_list[idx], MAX_STR);
        }

        return SUCCESS;
    }

    // UPDATE table SET col=val WHERE col=val
    if (strcmp(token_list[0], "UPDATE") == 0) {
        if (token_count < 6)
            return ERROR;

        out->type = SQL_UPDATE;
        strncpy(out->data.update_q.table, token_list[1], MAX_STR);

        if (strcmp(token_list[2], "SET") != 0)
            return ERROR;

        // SET col=val
        sscanf(token_list[3], "%[^=]=%s", out->data.update_q.set_col, out->data.update_q.set_val);

        // WHERE col=val
        if (strcmp(token_list[4], "WHERE") != 0)
            return ERROR;

        sscanf(token_list[5], "%[^=]=%s", out->data.update_q.where_col, out->data.update_q.where_val);

        return SUCCESS;
    }

    // DELETE FROM table WHERE col=val
    if (strcmp(token_list[0], "DELETE") == 0) {

        if (token_count < 5 || strcmp(token_list[1], "FROM") != 0)
            return ERROR;

        out->type = SQL_DELETE;
        strncpy(out->data.delete_q.table, token_list[2], MAX_STR);

        if (strcmp(token_list[3], "WHERE") != 0)
            return ERROR;

        sscanf(token_list[4], "%[^=]=%s", out->data.delete_q.where_col, out->data.delete_q.where_val);

        return SUCCESS;
    }

    return ERROR;
}
