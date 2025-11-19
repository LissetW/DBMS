#include "common.h"
#include "sql_parser.h"

#define DB_EXTENSION ".txt"

static void get_table_filename(
    const char *table_name, char *out_filename, size_t out_size) {
    if (table_name == NULL || out_filename == NULL || out_size == 0)
        return;

    snprintf(out_filename, out_size, "data/%s%s", table_name, DB_EXTENSION);
}

// SELECT * FROM table
int execute_select_all(
    struct sql_query *q, char *result_buffer, size_t buffer_size) {
    char table_filename[256];
    FILE *file_ptr;
    int c;
    size_t idx = 0;

    if (q == NULL || result_buffer == NULL || buffer_size == 0)
        return ERROR;

    get_table_filename(q->data.select_q.table, table_filename, sizeof(table_filename));

    // Try to open the file
    file_ptr = fopen(table_filename, "r");
    if (file_ptr == NULL) {
        return ERROR;
    }

    // Read file content char-by-char and write into result_buffer
    while ((c = fgetc(file_ptr)) != EOF) {
        if (idx < buffer_size - 1) {
            result_buffer[idx++] = (char)c;
        } else {
            fclose(file_ptr);
            return ERROR;
        }
    }

    result_buffer[idx] = '\0';
    fclose(file_ptr);
    return SUCCESS;
}

// INSERT INTO table VALUES (v1,v2,...)
int execute_insert(struct sql_insert *q, char *result_buffer, size_t buffer_size) {
    FILE *fp;
    char line[256];
    int last_id = 0;
    char table_filename[256];

    if (q == NULL || result_buffer == NULL || buffer_size == 0)
        return ERROR;

    get_table_filename(q->table, table_filename, sizeof(table_filename));

    // Expect 4 values
    if (q->value_count != 4) {
        snprintf(result_buffer, buffer_size, "ERROR: invalid number of values\n");
        return ERROR;
    }

    // Step 1: open for reading to find last ID
    fp = fopen(table_filename, "r");
    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp) != NULL) {
            int id;
            // parse first column (id)
            if (sscanf(line, "%d", &id) == 1) {
                if (id > last_id)
                    last_id = id;
            }
        }
        fclose(fp);
    }
    last_id++;

    // Step 2: open for appending new record
    fp = fopen(table_filename, "a");
    if (fp == NULL) {
        snprintf(result_buffer, buffer_size, "ERROR: could not open table '%s' for appending\n", table_filename);
        return ERROR;
    }

    // Write new record
    // Format: id,Nombre,Apellido,Semestre,Carrera
    fprintf(fp, "\n%d,%s,%s,%s,%s", last_id, q->values[0], q->values[1],
        q->values[2], q->values[3]);

    fclose(fp);

    snprintf(result_buffer, buffer_size, "OK: INSERT\n");
    return SUCCESS;
}

// UPDATE Alumno SET col=value WHERE col=value
int execute_update(struct sql_update *q, char *result_buffer, size_t buffer_size) {
    FILE *fp_in, *fp_out;
    char line[256];
    int updated = 0;
    char table_filename[256];

    if (q == NULL || result_buffer == NULL || buffer_size == 0)
        return ERROR;

    // Prevent updating the Id field
    if (strcmp(q->set_col, "Id") == 0) {
        snprintf(result_buffer, buffer_size, "ERROR: Cannot update the Id field.\n");
        return ERROR;
    }

    get_table_filename(q->table, table_filename, sizeof(table_filename));

    fp_in = fopen(table_filename, "r");
    if (fp_in == NULL) {
        snprintf(result_buffer, buffer_size, "ERROR: could not open table '%s'\n", table_filename);
        return ERROR;
    }

    // temp file where we write new DB
    fp_out = fopen("Alumno.tmp", "w");
    if (fp_out == NULL) {
        fclose(fp_in);
        snprintf(result_buffer, buffer_size, "ERROR: could not create temporary file\n");
        return ERROR;
    }

    // Read original DB line by line
    while (fgets(line, sizeof(line), fp_in) != NULL) {
        char id[32], nombre[64], apellido[64], calif[64], carrera[64];
        char where_val[64];

        // parse data line
        if (sscanf(line, "%[^,],%[^,],%[^,],%[^,],%s", id, nombre, apellido, calif, carrera) != 5) {
            fputs(line, fp_out);
            continue;
        }

        // evaluate WHERE condition
        if (strcmp(q->where_col, "Id") == 0) {
            snprintf(where_val, sizeof(where_val), "%s", id);

            if (strcmp(where_val, q->where_val) == 0) {
                if (strcmp(q->set_col, "Nombre") == 0)
                    snprintf(nombre, sizeof(nombre), "%s", q->set_val);
                else if (strcmp(q->set_col, "Apellido") == 0)
                    snprintf(apellido, sizeof(apellido), "%s", q->set_val);
                else if (strcmp(q->set_col, "Calificacion") == 0)
                    snprintf(calif, sizeof(calif), "%s", q->set_val);
                else if (strcmp(q->set_col, "Carrera") == 0)
                    snprintf(carrera, sizeof(carrera), "%s", q->set_val);

                updated = 1;
            }
        }

        // Write the (maybe modified) record
        fprintf(fp_out, "%s,%s,%s,%s,%s\n", id, nombre, apellido, calif, carrera);
    }

    fclose(fp_in);
    fclose(fp_out);

    // Replace old DB with updated temp file
    remove(table_filename);
    rename("Alumno.tmp", table_filename);

    if (!updated) {
        snprintf(result_buffer, buffer_size, "OK: UPDATE (0 rows modified)\n");
    } else {
        snprintf(result_buffer, buffer_size, "OK: UPDATE\n");
    }

    return SUCCESS;
}

// DELETE FROM table WHERE col=val
int execute_delete(
    struct sql_delete *q, char *result_buffer, size_t buffer_size) {
    FILE *fp_in, *fp_out;
    char line[256];
    int deleted = 0;
    char table_filename[256];

    if (q == NULL || result_buffer == NULL || buffer_size == 0)
        return ERROR;

    get_table_filename(q->table, table_filename, sizeof(table_filename));

    fp_in = fopen(table_filename, "r");
    if (fp_in == NULL) {
        snprintf(result_buffer, buffer_size, "ERROR: could not open table '%s'\n", table_filename);
        return ERROR;
    }

    fp_out = fopen("Alumno.tmp", "w");
    if (fp_out == NULL) {
        fclose(fp_in);
        snprintf(result_buffer, buffer_size, "ERROR: could not create temporary file\n");
        return ERROR;
    }

    while (fgets(line, sizeof(line), fp_in) != NULL) {
        char id[32], nombre[64], apellido[64], calif[64], carrera[64];
        char where_val[64];

        if (sscanf(line, "%[^,],%[^,],%[^,],%[^,],%s", id, nombre, apellido, calif, carrera) != 5) {
            fputs(line, fp_out);
            continue;
        }

        if (strcmp(q->where_col, "Id") == 0) {
            snprintf(where_val, sizeof(where_val), "%s", id);
            if (strcmp(where_val, q->where_val) == 0) {
                deleted++;
                continue;
            }
        }

        // keep record
        fprintf(fp_out, "%s,%s,%s,%s,%s\n", id, nombre, apellido, calif, carrera);
    }

    fclose(fp_in);
    fclose(fp_out);

    remove(table_filename);
    rename("Alumno.tmp", table_filename);

    if (deleted == 0) 
        snprintf(result_buffer, buffer_size, "OK: DELETE (0 rows removed)\n");
    else
        snprintf(result_buffer, buffer_size, "OK: DELETE (%d rows removed)\n", deleted);

    return SUCCESS;
}