#pragma once

#include "common.h"
#include "sql_parser.h"

/**
 * @brief Execute a SELECT * FROM <table> query and store the result in a
 * buffer.
 * @param q Pointer to the sql_query structure containing the table name.
 * @param result_buffer Buffer where the query result (full table contents)
 * will be stored.
 * @param buffer_size Size in bytes of result_buffer.
 * @return SUCCESS if executed correctly, ERROR on failure (file not found or
 * buffer too small).
 */
int execute_select_all(struct sql_query *q, char *result_buffer, size_t buffer_size);


/**
 * @brief Insert a new record into the specified table.
 * @param q Pointer to the sql_insert structure with values to insert.
 * @param result_buffer Buffer where the result message will be written.
 * @param buffer_size Size of result_buffer in bytes.
 * @return SUCCESS if executed correctly, ERROR on failure.
 */
int execute_insert(struct sql_insert *q, char *result_buffer, size_t buffer_size);

/**
 * @brief Update a record field in the table according to a WHERE condition.
 * @param q Pointer to the sql_update structure with update data.
 * @param result_buffer Buffer where the result message will be written.
 * @param buffer_size Size of result_buffer in bytes.
 * @return SUCCESS if executed correctly, ERROR on failure.
 */
int execute_update(struct sql_update *q, char *result_buffer, size_t buffer_size);

/**
 * @brief Delete records from the table according to a WHERE condition.
 * @param q Pointer to the sql_delete structure with delete data.
 * @param result_buffer Buffer where the result message will be written.
 * @param buffer_size Size of result_buffer in bytes.
 * @return SUCCESS if executed correctly, ERROR on failure.
 */
int execute_delete(struct sql_delete *q, char *result_buffer, size_t buffer_size);
