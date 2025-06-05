#include "b+treetemplate.h"
#include "functionpointer.h"

// Generic comparison functions
int compareInt(const void* a, const void* b) {
    int* ia = (int*)a;
    int* ib = (int*)b;
    return (*ia) - (*ib);
}

int compareStr(const void* a, const void* b) {
    char* sa = (char*)a;
    char* sb = (char*)b;
    return strcmp(sa, sb);
}

// Generic print functions
void printInt(const void* data) {
    printf("%d", *(int*)data);  // Fixed: Dereference pointer properly
}

void printStr(const void* data) {
    printf("%s", (char*)data);
}

// Generic clone functions
void* cloneInt(const void* data) {
    int* new_data = (int*)malloc(sizeof(int));
    if (new_data) {
        *new_data = *(int*)data;  // Fixed: Copy the value, not the pointer
    }
    return new_data;
}

void* cloneStr(const void* data) {
    char* str = (char*)data;
    char* new_str = (char*)malloc(strlen(str) + 1);
    if (new_str) {
        strcpy(new_str, str);
    }
    return new_str;
}

// Generic free functions
void freeInt(void* data) {
    free(data);
}

void freeStr(void* data) {
    free(data);
}

// Car related functions
int compareVIN(const void* a, const void* b) {
    Car* car_a = (Car*)a;
    Car* car_b = (Car*)b;
    return strcmp(car_a->VIN, car_b->VIN);
}

void printCar(const void* data) {
    Car* car = (Car*)data;
    printf("VIN: %s, Model: %s, Color: %s, Price: %.2f lakhs, Fuel: %s, Type: %s", 
           car->VIN, car->name, car->color, car->price, car->fuel_type, car->car_type);
}

void* cloneCar(const void* data) {
    Car* original = (Car*)data;
    Car* clone = (Car*)malloc(sizeof(Car));
    if (clone) {
        memcpy(clone, original, sizeof(Car));
    }
    return clone;
}

void freeCar(void* data) {
    free(data);
}

// SoldCar related functions
void printSoldCar(const void* data) {
    SoldCar* sold_car = (SoldCar*)data;
    printf("VIN: %s, Payment: %s", sold_car->VIN, sold_car->payment_type);
    if (strcmp(sold_car->payment_type, "Loan") == 0) {
        printf(", Down: %.2f, Period: %d months, EMI: %.2f", 
               sold_car->down_payment, sold_car->loan_period_months, sold_car->monthly_emi);
    }
}

void* cloneSoldCar(const void* data) {
    SoldCar* original = (SoldCar*)data;
    SoldCar* clone = (SoldCar*)malloc(sizeof(SoldCar));
    if (clone) {
        memcpy(clone, original, sizeof(SoldCar));
    }
    return clone;
}

void freeSoldCar(void* data) {
    free(data);
}

// Customer related functions
int compareCustomerByEMI(const void* a, const void* b) {
    Customer* cust_a = (Customer*)a;
    Customer* cust_b = (Customer*)b;
    return cust_a->loan_months-cust_b->loan_months;
}

void printCustomer(const void* data) {
    Customer* customer = (Customer*)data;
    printf("Name: %s, Mobile: %s, Address: %s, VIN: %s, Reg: %s", 
           customer->name, customer->mobile, customer->address, 
           customer->car_VIN, customer->reg_number);
}

void* cloneCustomer(const void* data) {
    Customer* original = (Customer*)data;
    Customer* clone = (Customer*)malloc(sizeof(Customer));
    if (clone) {
        memcpy(clone, original, sizeof(Customer));
    }
    return clone;
}

void freeCustomer(void* data) {
    free(data);
}

// SalesPerson related functions
int compareSalesPersonID(const void* a, const void* b) {
    SalesPerson* sp_a = (SalesPerson*)a;
    SalesPerson* sp_b = (SalesPerson*)b;
    return sp_a->id - sp_b->id;
}

void printSalesPerson(const void* data) {
    SalesPerson* sales_person = (SalesPerson*)data;
    printf("ID: %d, Name: %s, Target: %.2f lakhs, Achieved: %.2f lakhs, Commission: %.2f lakhs", 
           sales_person->id, sales_person->name, sales_person->target_sales, 
           sales_person->achieved_sales, sales_person->commission);
}

void* cloneSalesPerson(const void* data) {
    SalesPerson* original = (SalesPerson*)data;
    SalesPerson* clone = (SalesPerson*)malloc(sizeof(SalesPerson));
    if (!clone) return NULL;
    
    // Copy basic data
    clone->id = original->id;
    strcpy(clone->name, original->name);
    clone->target_sales = original->target_sales;
    clone->achieved_sales = original->achieved_sales;
    clone->commission = original->commission;
    
    // Initialize trees to NULL first
    clone->customer_tree = NULL;
    clone->sold_car_tree = NULL;
    
    // Create new trees
    if (original->customer_tree) {
        clone->customer_tree = createBPlusTree(compareCustomerByEMI, printCustomer, cloneCustomer, freeCustomer);
        
        // Now copy all entries from original tree to clone tree
        if (clone->customer_tree && original->customer_tree->root) {
            BTreeNode* node = original->customer_tree->root;
            while (!node->is_leaf) {
                node = node->children[0];
            }
            
            // Add all customers from original tree to clone tree
            while (node) {
                for (int i = 0; i < node->num_keys; i++) {
                    Customer* customer = (Customer*)node->keys[i].key;
                    Customer* customer_copy = (Customer*)cloneCustomer(customer);
                    if (customer_copy) {
                        bplusInsert(clone->customer_tree, customer_copy);
                        free(customer_copy); // bplusInsert makes its own copy
                    }
                }
                node = node->leaf_link.next;
            }
        }
    }
    
    if (original->sold_car_tree) {
        clone->sold_car_tree = createBPlusTree(compareVIN, printSoldCar, cloneSoldCar, freeSoldCar);
        
        // Now copy all entries from original tree to clone tree
        if (clone->sold_car_tree && original->sold_car_tree->root) {
            BTreeNode* node = original->sold_car_tree->root;
            while (!node->is_leaf) {
                node = node->children[0];
            }
            
            // Add all sold cars from original tree to clone tree
            while (node) {
                for (int i = 0; i < node->num_keys; i++) {
                    SoldCar* sold_car = (SoldCar*)node->keys[i].key;
                    SoldCar* sold_car_copy = (SoldCar*)cloneSoldCar(sold_car);
                    if (sold_car_copy) {
                        bplusInsert(clone->sold_car_tree, sold_car_copy);
                        free(sold_car_copy); // bplusInsert makes its own copy
                    }
                }
                node = node->leaf_link.next;
            }
        }
    }
    
    return clone;
}

void freeSalesPerson(void* data) {
    SalesPerson* sales_person = (SalesPerson*)data;
    
    // Free B+ trees
    if (sales_person->customer_tree) {
        freeBPlusTree(sales_person->customer_tree);
    }
    
    if (sales_person->sold_car_tree) {
        freeBPlusTree(sales_person->sold_car_tree);
    }
    
    free(sales_person);
}

// Showroom related functions
int compareShowroomID(const void* a, const void* b) {
    Showroom* shr_a = (Showroom*)a;
    Showroom* shr_b = (Showroom*)b;
    return shr_a->id - shr_b->id;
}

void printShowroom(const void* data) {
    Showroom* showroom = (Showroom*)data;
    printf("ID: %d, Name: %s, Location: %s, Contact: %s, Available Cars: %d, Sold Cars: %d", 
           showroom->id, showroom->name, showroom->location, showroom->contact, 
           showroom->total_available_cars, showroom->total_sold_cars);
}

void* cloneShowroom(const void* data) {
    Showroom* original = (Showroom*)data;
    Showroom* clone = (Showroom*)malloc(sizeof(Showroom));
    if (!clone) return NULL;
    
    // Copy basic data
    clone->id = original->id;
    strcpy(clone->name, original->name);
    strcpy(clone->location, original->location);
    strcpy(clone->contact, original->contact);
    clone->total_available_cars = original->total_available_cars;
    clone->total_sold_cars = original->total_sold_cars;
    
    // Initialize trees to NULL first
    clone->available_cars = NULL;
    clone->sold_cars = NULL;
    clone->sales_persons = NULL;
    
    // Create new trees
    if (original->available_cars) {
        clone->available_cars = createBPlusTree(compareVIN, printCar, cloneCar, freeCar);
        
        // Now copy all entries from original tree to clone tree
        if (clone->available_cars && original->available_cars->root) {
            BTreeNode* node = original->available_cars->root;
            while (!node->is_leaf) {
                node = node->children[0];
            }
            
            // Add all cars from original tree to clone tree
            while (node) {
                for (int i = 0; i < node->num_keys; i++) {
                    Car* car = (Car*)node->keys[i].key;
                    Car* car_copy = (Car*)cloneCar(car);
                    if (car_copy) {
                        bplusInsert(clone->available_cars, car_copy);
                        free(car_copy); // bplusInsert makes its own copy
                    }
                }
                node = node->leaf_link.next;
            }
        }
    }
    
    if (original->sold_cars) {
        clone->sold_cars = createBPlusTree(compareVIN, printCar, cloneCar, freeCar);
        
        // Now copy all entries from original tree to clone tree
        if (clone->sold_cars && original->sold_cars->root) {
            BTreeNode* node = original->sold_cars->root;
            while (!node->is_leaf) {
                node = node->children[0];
            }
            
            // Add all cars from original tree to clone tree
            while (node) {
                for (int i = 0; i < node->num_keys; i++) {
                    Car* car = (Car*)node->keys[i].key;
                    Car* car_copy = (Car*)cloneCar(car);
                    if (car_copy) {
                        bplusInsert(clone->sold_cars, car_copy);
                        free(car_copy); // bplusInsert makes its own copy
                    }
                }
                node = node->leaf_link.next;
            }
        }
    }
    
    if (original->sales_persons) {
        clone->sales_persons = createBPlusTree(compareSalesPersonID, printSalesPerson, cloneSalesPerson, freeSalesPerson);
        
        // Now copy all entries from original tree to clone tree
        if (clone->sales_persons && original->sales_persons->root) {
            BTreeNode* node = original->sales_persons->root;
            while (!node->is_leaf) {
                node = node->children[0];
            }
            
            // Add all salespeople from original tree to clone tree
            while (node) {
                for (int i = 0; i < node->num_keys; i++) {
                    SalesPerson* sp = (SalesPerson*)node->keys[i].key;
                    SalesPerson* sp_copy = (SalesPerson*)cloneSalesPerson(sp);
                    if (sp_copy) {
                        bplusInsert(clone->sales_persons, sp_copy);
                        free(sp_copy); // bplusInsert makes its own copy
                    }
                }
                node = node->leaf_link.next;
            }
        }
    }
    
    return clone;
}

void freeShowroom(void* data) {
    Showroom* showroom = (Showroom*)data;
    
    // Free B+ trees
    if (showroom->available_cars) {
        freeBPlusTree(showroom->available_cars);
    }
    
    if (showroom->sold_cars) {
        freeBPlusTree(showroom->sold_cars);
    }
    
    if (showroom->sales_persons) {
        freeBPlusTree(showroom->sales_persons);
    }
    
    free(showroom);
}