#include <stdlib.h>
#include <stdio.h>
#include "usernamekeytable.h"
#define SOCKET int 

table_elem *table_search(table_t *table, size_t table_size, SOCKET* key) {
    for (size_t i = 0; i < table_size; i++)
    {
		if(*(table->table[i]->key) == (*key))
        {
            return table->table[i];
        }           
    }
    return NULL;
}

void append_element(table_t *table, table_elem *appending_elem) {  
    if(appending_elem == NULL) {
		perror("Element not included.");
		return;
	}

    if(table->table_size <= table->num_of_elems) {
        table = realloc(table, table->table_size*2);
        table->table_size = table->table_size*2;
    }
    for(int i = 0; i < table->table_size; i++) {
        if((table->table[i]->key == NULL) && (table->table[i]->username == NULL)) {
            table->table[i]->key = appending_elem->key;
            table->table[i]->username = appending_elem->username;
	        table->num_of_elems++;
	        return;
        }
    }
}

void remove_element(table_t *table, table_elem *removing_elem) {
		if(removing_elem == NULL) {
		    perror("No element");
			return;
		}
		for (int i = 0; i < table->num_of_elems; i++) {
        if(removing_elem->key == table->table[i]->key) {
            table->table[i]->key = NULL;
            table->table[i]->username = NULL;
			table->num_of_elems--;
			return;
        } 
	}
}
