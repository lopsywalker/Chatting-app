#include <stdio.h>
#include <stdlib.h>

typedef struct table_element {
    SOCKET* key;
    char* username;
} table_elem;

typedef struct h_table {
    size_t table_size;
    size_t num_of_elems;
    table_elem *table;
}  table_t;

char* table_search(table_t *table, size_t table_size, SOCKET* key) {
    for (size_t i = 0; i < table_size; i++)
    {
    if(table->table[i].key == key)
        {
            return table->table[i].username;
        }            
    }
    return NULL;
}

// TODO: just iterate through it and add elements like that
void append_element(table_t *table, table_elem appending_elem) {
    if(table->table_size <= table->num_of_elems) {
        table = realloc(table, (table->table_size) * 2);
        table->table_size = table->table_size * 2; 
        table->table[(table->num_of_elems) + 1] = appending_elem;
        table->num_of_elems = table->num_of_elems + 1;  
        return;
    }

    else if (table->num_of_elems != 0)
    {
        table->table[(table->num_of_elems) + 1] = appending_elem;
        table->num_of_elems = table->num_of_elems + 1;  
        return;
    }

    table->table[0] = appending_elem;
    table->num_of_elems = table->num_of_elems + 1;
}

// TODO: remove element