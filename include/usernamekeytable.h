#ifndef USERNAMETABLE_H
#define USERNAMETABLE_H

// struct needed before the defined functions
typedef struct table_element {
    int *key;
    char *username;
} table_elem;

typedef struct h_table {
    size_t table_size;
    size_t num_of_elems;
    table_elem *table;
}  table_t;

table_elem* table_search(table_t *table, size_t table_size, int* key);

void append_element(table_t *table, table_elem *appending_elem);

void remove_element(table_t *table, table_elem *removing_elem);


#endif
