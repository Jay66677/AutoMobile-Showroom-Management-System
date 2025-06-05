#include "filehandling.h"
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h>
#endif

// Field separator for data files
#define FIELD_SEP "|"

// Helper function to ensure data directory exists
void ensure_data_directory() {
    #ifdef _WIN32
    mkdir("data");
    #else
    mkdir("data", 0777);
    #endif
}

// Helper function to save a showroom
void save_showroom(Showroom* showroom, FILE* file) {
    fprintf(file, "%d%s%s%s%s%s%s%s%d%s%d\n", 
            showroom->id, FIELD_SEP,
            showroom->name, FIELD_SEP,
            showroom->location, FIELD_SEP,
            showroom->contact, FIELD_SEP,
            showroom->total_available_cars, FIELD_SEP,
            showroom->total_sold_cars);
}

// Helper function to process and save each showroom
void process_showroom_save(void* key, void* user_data) {
    Showroom* showroom = (Showroom*)key;
    FILE* file = (FILE*)user_data;
    
    save_showroom(showroom, file);
    
    // Save all cars in this showroom
    if (showroom->available_cars && showroom->available_cars->root) {
        save_cars_to_file(showroom, file);
    }
    
    // Save all sold cars in this showroom
    if (showroom->sold_cars && showroom->sold_cars->root) {
        save_sold_cars_to_file(showroom, file);
    }
    
    // Save all salespersons in this showroom
    if (showroom->sales_persons && showroom->sales_persons->root) {
        save_salespersons_to_file(showroom, file);
    }
}

// Save all showrooms to file
void save_showrooms_to_file() {
    ensure_data_directory();
    FILE* file = fopen(SHOWROOMS_FILE, "w");
    if (!file) {
        printf("Error: Could not open file for writing: %s\n", SHOWROOMS_FILE);
        return;
    }
    
    if (showroom_tree && showroom_tree->root) {
        // Find the leftmost leaf node (first showroom)
        BTreeNode* node = showroom_tree->root;
        while (!node->is_leaf) {
            node = node->children[0];
        }
        
        // Traverse all leaf nodes to save all showrooms
        while (node) {
            for (int i = 0; i < node->num_keys; i++) {
                process_showroom_save(node->keys[i].key, file);
            }
            node = node->leaf_link.next;
        }
    }
    
    fclose(file);
}

// Save cars for a specific showroom
void save_cars_to_file(Showroom* showroom, FILE* file) {
    FILE* car_file = fopen(CARS_FILE, "a");
    if (!car_file) {
        printf("Error: Could not open file for writing: %s\n", CARS_FILE);
        return;
    }
    
    // Process all cars
    if (showroom->available_cars && showroom->available_cars->root) {
        BTreeNode* node = showroom->available_cars->root;
        while (!node->is_leaf) {
            node = node->children[0];
        }
        
        while (node) {
            for (int i = 0; i < node->num_keys; i++) {
                Car* car = (Car*)node->keys[i].key;
                fprintf(car_file, "%d%s%s%s%s%s%s%s%.2f%s%s%s%s\n", 
                        showroom->id, FIELD_SEP,
                        car->VIN, FIELD_SEP,
                        car->name, FIELD_SEP,
                        car->color, FIELD_SEP,
                        car->price, FIELD_SEP,
                        car->fuel_type, FIELD_SEP,
                        car->car_type);
            }
            node = node->leaf_link.next;
        }
    }
    
    fclose(car_file);
}

// Save sold cars for a specific showroom
void save_sold_cars_to_file(Showroom* showroom, FILE* file) {
    FILE* sold_car_file = fopen(SOLD_CARS_FILE, "a");
    if (!sold_car_file) {
        printf("Error: Could not open file for writing: %s\n", SOLD_CARS_FILE);
        return;
    }
    
    // Process all sold cars
    if (showroom->sold_cars && showroom->sold_cars->root) {
        BTreeNode* node = showroom->sold_cars->root;
        while (!node->is_leaf) {
            node = node->children[0];
        }
        
        while (node) {
            for (int i = 0; i < node->num_keys; i++) {
                SoldCar* sold_car = (SoldCar*)node->keys[i].key;
                fprintf(sold_car_file, "%d%s%s%s%s%s%.2f%s%d%s%.2f%s%.2f%s%.2f\n", 
                        showroom->id, FIELD_SEP,
                        sold_car->VIN, FIELD_SEP,
                        sold_car->payment_type, FIELD_SEP,
                        sold_car->down_payment, FIELD_SEP,
                        sold_car->loan_period_months, FIELD_SEP,
                        sold_car->loan_amount, FIELD_SEP,
                        sold_car->interest_rate, FIELD_SEP,
                        sold_car->monthly_emi);
            }
            node = node->leaf_link.next;
        }
    }
    
    fclose(sold_car_file);
}

// Save salespersons for a specific showroom
void save_salespersons_to_file(Showroom* showroom, FILE* file) {
    FILE* sp_file = fopen(SALESPERSONS_FILE, "a");
    if (!sp_file) {
        printf("Error: Could not open file for writing: %s\n", SALESPERSONS_FILE);
        return;
    }
    
    // Process all salespersons
    if (showroom->sales_persons && showroom->sales_persons->root) {
        BTreeNode* node = showroom->sales_persons->root;
        while (!node->is_leaf) {
            node = node->children[0];
        }
        
        while (node) {
            for (int i = 0; i < node->num_keys; i++) {
                SalesPerson* sp = (SalesPerson*)node->keys[i].key;
                fprintf(sp_file, "%d%s%d%s%s%s%.2f%s%.2f%s%.2f\n", 
                        showroom->id, FIELD_SEP,
                        sp->id, FIELD_SEP,
                        sp->name, FIELD_SEP,
                        sp->target_sales, FIELD_SEP,
                        sp->achieved_sales, FIELD_SEP,
                        sp->commission);
                
                // Save customers for this salesperson
                save_customers_to_file(sp, sp_file);
            }
            node = node->leaf_link.next;
        }
    }
    
    fclose(sp_file);
}

// Save customers for a salesperson
void save_customers_to_file(SalesPerson* salesperson, FILE* file) {
    FILE* customer_file = fopen(CUSTOMERS_FILE, "a");
    if (!customer_file) {
        printf("Error: Could not open file for writing: %s\n", CUSTOMERS_FILE);
        return;
    }
    
    // Process all customers
    if (salesperson->customer_tree && salesperson->customer_tree->root) {
        BTreeNode* node = salesperson->customer_tree->root;
        while (!node->is_leaf) {
            node = node->children[0];
        }
        
        while (node) {
            for (int i = 0; i < node->num_keys; i++) {
                Customer* customer = (Customer*)node->keys[i].key;
                fprintf(customer_file, "%d%s%s%s%s%s%s%s%s%s%s%s%.2f%s%d%s%d%s%d%s%d\n", 
                        salesperson->id, FIELD_SEP,
                        customer->name, FIELD_SEP,
                        customer->mobile, FIELD_SEP,
                        customer->address, FIELD_SEP,
                        customer->car_VIN, FIELD_SEP,
                        customer->reg_number, FIELD_SEP,
                        customer->actual_aoumnt_paid, FIELD_SEP,
                        customer->purchase_day, FIELD_SEP,
                        customer->purchase_month, FIELD_SEP,
                        customer->purchase_year, FIELD_SEP,
                        customer->loan_months);
            }
            node = node->leaf_link.next;
        }
    }
    
    fclose(customer_file);
}

// Helper function to process a line from showrooms file
void process_showroom_line(char* line) {
    Showroom* showroom = (Showroom*)malloc(sizeof(Showroom));
    if (!showroom) return;
    
    // Parse the line
    char* token = strtok(line, FIELD_SEP);
    showroom->id = atoi(token);
    
    token = strtok(NULL, FIELD_SEP);
    strncpy(showroom->name, token, MAX_STR_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    strncpy(showroom->location, token, MAX_STR_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    strncpy(showroom->contact, token, MAX_MOBILE_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    showroom->total_available_cars = atoi(token);
    
    token = strtok(NULL, FIELD_SEP);
    showroom->total_sold_cars = atoi(token);
    
    // Initialize trees
    showroom->available_cars = createBPlusTree(compareVIN, printCar, cloneCar, freeCar);
    showroom->sold_cars = createBPlusTree(compareVIN, printSoldCar, cloneSoldCar, freeSoldCar);
    showroom->sales_persons = createBPlusTree(compareSalesPersonID, printSalesPerson, cloneSalesPerson, freeSalesPerson);
    
    // Add to showroom tree
    bplusInsert(showroom_tree, showroom);
}

// Helper function to process a line from cars file
void process_car_line(char* line) {
    int showroom_id;
    Car* car = (Car*)malloc(sizeof(Car));
    if (!car) return;
    
    // Parse the line with null checks
    char* token = strtok(line, FIELD_SEP);
    if (!token) { free(car); return; }
    showroom_id = atoi(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(car); return; }
    strncpy(car->VIN, token, MAX_VIN_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(car); return; }
    strncpy(car->name, token, MAX_STR_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(car); return; }
    strncpy(car->color, token, MAX_STR_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(car); return; }
    car->price = atof(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(car); return; }
    strncpy(car->fuel_type, token, MAX_STR_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(car); return; }
    strncpy(car->car_type, token, MAX_STR_LEN);
    
    // Find the showroom
    Showroom temp_showroom;
    temp_showroom.id = showroom_id;
    Showroom* showroom = (Showroom*)bplusSearch(showroom_tree, &temp_showroom);
    
    if (showroom) {
        // Ensure available_cars tree exists
        if (!showroom->available_cars) {
            showroom->available_cars = createBPlusTree(
                compareVIN, printCar, cloneCar, freeCar
            );
        }
        
        // Add car to showroom
        bplusInsert(showroom->available_cars, car);
        showroom->total_available_cars++; // Increment car count
        printf("Car %s added to showroom %d\n", car->VIN, showroom_id);
    } else {
        // Showroom not found, free car
        printf("Error: Could not find showroom with ID %d for car %s\n", 
               showroom_id, car->VIN);
        free(car);
    }
}


// Helper function to process a line from sold cars file
void process_sold_car_line(char* line) {
    int showroom_id;
    SoldCar* sold_car = (SoldCar*)malloc(sizeof(SoldCar));
    if (!sold_car) return;
    
    // Parse the line with null checks
    char* token = strtok(line, FIELD_SEP);
    if (!token) { free(sold_car); return; }
    showroom_id = atoi(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(sold_car); return; }
    strncpy(sold_car->VIN, token, MAX_VIN_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(sold_car); return; }
    strncpy(sold_car->payment_type, token, MAX_STR_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(sold_car); return; }
    sold_car->down_payment = atof(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(sold_car); return; }
    sold_car->loan_period_months = atoi(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(sold_car); return; }
    sold_car->loan_amount = atof(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(sold_car); return; }
    sold_car->interest_rate = atof(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(sold_car); return; }
    sold_car->monthly_emi = atof(token);
    
    // Find the showroom
    Showroom temp_showroom;
    temp_showroom.id = showroom_id;
    Showroom* showroom = (Showroom*)bplusSearch(showroom_tree, &temp_showroom);
    
    if (showroom) {
        // Ensure sold_cars tree exists
        if (!showroom->sold_cars) {
            showroom->sold_cars = createBPlusTree(
                compareVIN, printSoldCar, cloneSoldCar, freeSoldCar
            );
        }
        
        // Add sold car to showroom
        bplusInsert(showroom->sold_cars, sold_car);
        showroom->total_sold_cars++; // Increment sold car count
        printf("Sold car %s added to showroom %d\n", sold_car->VIN, showroom_id);
    } else {
        // Showroom not found, free sold car
        printf("Error: Could not find showroom with ID %d for sold car %s\n", 
               showroom_id, sold_car->VIN);
        free(sold_car);
    }
}

// Helper function to process a line from salespersons file
void process_salesperson_line(char* line) {
    int showroom_id;
    SalesPerson* sp = (SalesPerson*)malloc(sizeof(SalesPerson));
    if (!sp) return;
    
    // Parse the line with null checks
    char* token = strtok(line, FIELD_SEP);
    if (!token) { free(sp); return; }
    showroom_id = atoi(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(sp); return; }
    sp->id = atoi(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(sp); return; }
    strncpy(sp->name, token, MAX_STR_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(sp); return; }
    sp->target_sales = atof(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(sp); return; }
    sp->achieved_sales = atof(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(sp); return; }
    sp->commission = atof(token);
    
    // Initialize trees
    sp->customer_tree = createBPlusTree(compareCustomerByEMI, printCustomer, cloneCustomer, freeCustomer);
    sp->sold_car_tree = createBPlusTree(compareVIN, printSoldCar, cloneSoldCar, freeSoldCar);
    
    // Find the showroom
    Showroom temp_showroom;
    temp_showroom.id = showroom_id;
    Showroom* showroom = (Showroom*)bplusSearch(showroom_tree, &temp_showroom);
    
    if (showroom) {
        // Ensure sales_persons tree exists
        if (!showroom->sales_persons) {
            showroom->sales_persons = createBPlusTree(
                compareSalesPersonID, printSalesPerson, cloneSalesPerson, freeSalesPerson
            );
        }
        
        // Add salesperson to showroom
        bplusInsert(showroom->sales_persons, sp);
        printf("Salesperson %d added to showroom %d\n", sp->id, showroom_id);
    } else {
        // Free trees first to prevent memory leaks
        freeBPlusTree(sp->customer_tree);
        freeBPlusTree(sp->sold_car_tree);
        
        // Showroom not found, free salesperson
        printf("Error: Could not find showroom with ID %d for salesperson %d\n", 
               showroom_id, sp->id);
        free(sp);
    }
}

// Helper function to process a line from customers file
void process_customer_line(char* line) {
    int salesperson_id;
    Customer* customer = (Customer*)malloc(sizeof(Customer));
    if (!customer) return;
    
    // Parse the line
    char* token = strtok(line, FIELD_SEP);
    salesperson_id = atoi(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(customer); return; } // Prevent NULL dereference
    strncpy(customer->name, token, MAX_STR_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(customer); return; }
    strncpy(customer->mobile, token, MAX_MOBILE_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(customer); return; }
    strncpy(customer->address, token, MAX_STR_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(customer); return; }
    strncpy(customer->car_VIN, token, MAX_VIN_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(customer); return; }
    strncpy(customer->reg_number, token, MAX_REG_NUM_LEN);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(customer); return; }
    customer->actual_aoumnt_paid = atof(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(customer); return; }
    customer->purchase_day = atoi(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(customer); return; }
    customer->purchase_month = atoi(token);
    
    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(customer); return; }
    customer->purchase_year = atoi(token);

    token = strtok(NULL, FIELD_SEP);
    if (!token) { free(customer); return; }
    customer->loan_months = atoi(token);
    
    // Find the salesperson in all showrooms - improved algorithm
    SalesPerson* found_sp = NULL;
    
    // Use leaf node traversal for efficient searching through showrooms
    if (showroom_tree && showroom_tree->root) {
        BTreeNode* node = showroom_tree->root;
        while (!node->is_leaf) {
            node = node->children[0]; // Get to leftmost leaf
        }
        
        // Now traverse all showrooms via leaf nodes
        while (node && !found_sp) {
            for (int i = 0; i < node->num_keys; i++) {
                Showroom* showroom = (Showroom*)node->keys[i].key;
                
                if (showroom && showroom->sales_persons && showroom->sales_persons->root) {
                    // Create temp salesperson with target ID
                    SalesPerson temp_sp;
                    temp_sp.id = salesperson_id;
                    
                    // Search for this salesperson in current showroom
                    found_sp = (SalesPerson*)bplusSearch(showroom->sales_persons, &temp_sp);
                    
                    if (found_sp) {
                        // Initialize customer tree if needed
                        if (!found_sp->customer_tree) {
                            found_sp->customer_tree = createBPlusTree(
                                compareCustomerByEMI, 
                                printCustomer, 
                                cloneCustomer, 
                                freeCustomer
                            );
                        }
                        
                        // Add customer to salesperson
                        bplusInsert(found_sp->customer_tree, customer);
                        printf("Customer %s added to salesperson %d\n", customer->name, salesperson_id);
                        return;
                    }
                }
            }
            node = node->leaf_link.next; // Move to next leaf node
        }
    }
    
    // Salesperson not found, free customer and report error
    printf("Error: Could not find salesperson with ID %d for customer %s\n", 
           salesperson_id, customer->name);
    free(customer);
}

// Load showrooms from file
void load_showrooms_from_file() {
    FILE* file = fopen(SHOWROOMS_FILE, "r");
    if (!file) {
        printf("No showrooms data file found. Starting with empty database.\n");
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        process_showroom_line(line);
    }
    
    fclose(file);
}

// Load cars from file
void load_cars_from_file() {
    FILE* file = fopen(CARS_FILE, "r");
    if (!file) {
        printf("No cars data file found.\n");
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        process_car_line(line);
    }
    
    fclose(file);
}

// Load sold cars from file
void load_sold_cars_from_file() {
    FILE* file = fopen(SOLD_CARS_FILE, "r");
    if (!file) {
        printf("No sold cars data file found.\n");
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        process_sold_car_line(line);
    }
    
    fclose(file);
}

// Load salespersons from file
void load_salespersons_from_file() {
    FILE* file = fopen(SALESPERSONS_FILE, "r");
    if (!file) {
        printf("No salespersons data file found.\n");
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        process_salesperson_line(line);
    }
    
    fclose(file);
}

// Load customers from file
void load_customers_from_file() {
    FILE* file = fopen(CUSTOMERS_FILE, "r");
    if (!file) {
        printf("No customers data file found.\n");
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        process_customer_line(line);
    }
    
    fclose(file);
}

// Save car popularity data to file
void save_car_popularity_to_file() {
    FILE* file = fopen(CAR_POPULARITY_FILE, "w");
    if (!file) {
        printf("Error: Could not open file for writing: %s\n", CAR_POPULARITY_FILE);
        return;
    }

    int id = 1; // Starting ID for entries
    
    // Loop through the entire hash table
    for (int i = 0; i < HASH_SIZE; i++) {
        CarPopularityEntry* entry = car_popularity_table[i];
        while (entry) {
            // Write each entry as: id|model_name|count
            fprintf(file, "%d%s%s%s%d\n", 
                   id++, FIELD_SEP, entry->model_name, FIELD_SEP, entry->count);
            entry = entry->next;
        }
    }
    
    fclose(file);
    printf("Car popularity data saved successfully.\n");
}

// Load car popularity data from file
void load_car_popularity_from_file() {
    // First clear any existing data
    free_car_popularity_table();
    
    FILE* file = fopen(CAR_POPULARITY_FILE, "r");
    if (!file) {
        printf("No car popularity data file found. Starting with empty popularity data.\n");
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        
        // Parse the line: id|model_name|popularity_count
        char* token = strtok(line, FIELD_SEP);
        if (!token) continue; // Skip ID
        
        token = strtok(NULL, FIELD_SEP);
        if (!token) continue; // Get model name
        
        char model_name[MAX_STR_LEN];
        strncpy(model_name, token, MAX_STR_LEN - 1);
        model_name[MAX_STR_LEN - 1] = '\0'; // Ensure null-termination
        
        token = strtok(NULL, FIELD_SEP);
        if (!token) continue; // Get count
        int count = atoi(token);
        
        // Create entry directly (more efficient than calling increment_car_popularity multiple times)
        unsigned int index = hash_model(model_name);
        
        CarPopularityEntry* new_entry = (CarPopularityEntry*)malloc(sizeof(CarPopularityEntry));
        if (!new_entry) {
            printf("Memory allocation failed while loading car popularity data\n");
            continue;
        }
        
        strncpy(new_entry->model_name, model_name, MAX_STR_LEN - 1);
        new_entry->model_name[MAX_STR_LEN - 1] = '\0';
        new_entry->count = count;
        new_entry->next = car_popularity_table[index];
        car_popularity_table[index] = new_entry;
    }
    
    fclose(file);
    printf("Car popularity data loaded successfully.\n");
}

// Save all data to files
void save_all_data() {
    printf("Saving data...\n");
    
    // Create empty files first to avoid appending to old data
    ensure_data_directory();
    FILE* f;
    f = fopen(CARS_FILE, "w"); fclose(f);
    f = fopen(SOLD_CARS_FILE, "w"); fclose(f);
    f = fopen(SALESPERSONS_FILE, "w"); fclose(f);
    f = fopen(CUSTOMERS_FILE, "w"); fclose(f);
    
    // Now save all data
    save_showrooms_to_file();
    save_car_popularity_to_file();
    
    printf("Data saved successfully.\n");
}

// Load all data from files
void load_all_data() {
    printf("Loading data...\n");
    
    // First load showrooms as the base structure
    load_showrooms_from_file();
    
    // Next load salespersons as they're directly connected to showrooms
    load_salespersons_from_file();
    
    // Then load cars (both available and sold)
    load_cars_from_file();
    load_sold_cars_from_file();
    
    // Finally load customers as they depend on salespersons
    load_customers_from_file();
    
    // Load car popularity data
    load_car_popularity_from_file();
    
    printf("Data loaded successfully.\n");
}