#include <stdlib.h>
#define SOCKET int 

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

void append_element(table_t *table, table_elem *appending_elem) {
    if(table->table_size <= table->num_of_elems) {
        table = (table_t *) realloc(table, table->table_size*2);
        table->table_size = table->table_size*2;
    }
    for(int i = 0; i < table->table_size; i++) {
        if((table->table[i].key == NULL) && (table->table[i].username == NULL)) {
            table->table[i].key = appending_elem->key;
            table->table[i].username = appending_elem->username;
            return;
        }
    }
}

void remove_element(table_t *table, table_elem *removing_elem) {
    for (int i = 0; i < table->num_of_elems; i++) {
        if(removing_elem->key == table->table[i].key) {
            table->table[i].key = NULL;
            table->table[i].username = NULL;
            return;
        }
    }
}