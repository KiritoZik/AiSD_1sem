#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct Table{
    double* keys;
    char** values;
    int count;
    int capacity;
} Table;


Table* init_table(){
    Table* myTable = (Table*)malloc(sizeof(Table));
    myTable->capacity = 0;
    myTable->count = 0;
    myTable->keys = NULL;
    myTable-> values = NULL;
    return myTable;
}

void add_to_table(Table* myTable, double myKey, char* myValue){
    if (myTable->count >= myTable->capacity) {
        int new_capacity = (myTable->capacity == 0) ? 8 : myTable->capacity * 2;
        double* new_keys = (double*)realloc(myTable->keys, new_capacity * sizeof(double));
        char** new_values = (char**)realloc(myTable->values, new_capacity * sizeof(char*));
        
        if (!new_keys || !new_values) {
            printf("Memory allocation failed!\n");
            exit(1); 
        }

        myTable->keys = new_keys;
        myTable->values = new_values;
        myTable->capacity = new_capacity;
    }

    myTable->keys[myTable->count] = myKey;
    myTable->values[myTable->count] = strdup(myValue);
    myTable->count++;
}

void free_table(Table* myTable) {
    for (int i = 0; i < myTable->count; i++) free(myTable->values[i]);
    free(myTable->keys);
    free(myTable->values);
    free(myTable);
}

void swap(Table* myTable, int i, int j){
    const double kTemp = myTable->keys[i];
    myTable->keys[i] = myTable->keys[j];
    myTable->keys[j] = kTemp;

    char* vTemp = myTable->values[i];
    myTable->values[i] = myTable->values[j];
    myTable->values[j] = vTemp;
}

void sift_down(Table* t, const int n, int i) {
    int maxKey = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && t->keys[left] > t->keys[maxKey])
        maxKey = left;

    if (right < n && t->keys[right] > t->keys[maxKey])
        maxKey = right;

    if (maxKey != i) {
        swap(t, i, maxKey);
        sift_down(t, n, maxKey);
    }
}

void tree_sort(Table* t) {
    for (int i = t->count / 2 - 1; i >= 0; i--) {
        sift_down(t, t->count, i);
    }
    for (int i = t->count - 1; i > 0; i--) {
        swap(t, 0, i);  
        sift_down(t, i, 0); 
    }
}

int binary_search(Table* t, double target) {
    int low = 0, high = t->count - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (fabs(t->keys[mid] - target) < 1e-9) return mid;
        if (t->keys[mid] < target) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}


int main(void) {
    FILE* f = fopen("reverse.txt", "r");
    // FILE* f = fopen("sorted.txt", "r");
    if (f == NULL) {
        printf("Error, file not open\n");
        return 1;
    }
    Table* myTable = init_table();
    double temp_k;
    char temp_v[256];
    while (fscanf(f, "%lf %255s", &temp_k, temp_v) == 2) {
        add_to_table(myTable, temp_k, temp_v);
    }


    printf("Table:\n");
    for (int i = 0; i < myTable->count; i++) {
        printf("%02d) %.4f -> %s\n", i + 1, myTable->keys[i], myTable->values[i]);
    }
    printf("\n");

    tree_sort(myTable);

    printf("\n Sort table\n");
    for (int i = 0; i < myTable->count; i++) {
        printf("%02d) %.4f -> %s\n", i + 1, myTable->keys[i], myTable->values[i]);
    }

    fclose(f);

    double search_key;
    printf("\nInput key for search\n");
    if (scanf("%lf", &search_key) == 1) {
        int idx = binary_search(myTable, search_key);
        if (idx != -1) {
            printf("Key %d: %s\n", idx + 1, myTable->values[idx]);
        } else {
            printf("Key %.4f not found in table.\n", search_key);
        }
    }

    free_table(myTable);
    return 0;
}
