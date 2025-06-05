#ifndef ESS_FUN_H
#define ESS_FUN_H

#include "functionpointer.h"

extern BPlusTree* showroom_tree;
extern CarPopularityEntry* car_popularity_table[HASH_SIZE];

typedef struct {
    int month;
    int year;
    int sales_count;
    double sales_value;
} MonthlySales;


double calculate_interest_rate(int months);
double calculate_emi(double loan_amount, double interest_rate, int months);
void init_system();
void add_showroom();
void add_new_stock();
void recruit_salesperson();   //markB
void car_purchase();          //markE
 

void merge_showrooms();
void display_showroom_inventory();
void find_most_successful_SP();
void predict_next_month_sales();
void find_car_by_VIN();
void search_salespersons_by_sales_range();
void increment_car_popularity(const char* model_name);
char* find_most_popular_car(int* max_count);
void display_car_popularity();
void free_car_popularity_table();
void list_customers_with_emi_in_range();

//helper
int count_nodes_in_tree(BTreeNode* node);
unsigned int hash_model(const char* str);




#endif