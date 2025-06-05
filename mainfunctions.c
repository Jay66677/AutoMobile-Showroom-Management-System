#include "essentialfunction.h"

// Function to display showroom inventory details
void display_showroom_inventory() {
    int showroom_id;
    
    printf("\n=== Display Showroom Inventory ===\n");
    
    // Get showroom ID
    printf("Enter Showroom ID: ");
    scanf("%d", &showroom_id);
    getchar(); // Clear input buffer
    
    // Create a temporary showroom object to search the tree
    Showroom temp;
    temp.id = showroom_id;
    
    // Search for the showroom
    Showroom* showroom = (Showroom*)bplusSearch(showroom_tree, &temp);
    if (!showroom) {
        printf("Showroom with ID %d not found.\n", showroom_id);
        return;
    }
    
    // Display showroom details
    printf("\nShowroom Details:\n");
    printf("ID: %d\n", showroom->id);
    printf("Name: %s\n", showroom->name);
    printf("Location: %s\n", showroom->location);
    printf("Contact: %s\n", showroom->contact);
    printf("Available Cars: %d\n", showroom->total_available_cars);
    printf("Sold Cars: %d\n", showroom->total_sold_cars);
    
    // Display available cars
    printf("\nAvailable Cars:\n");
    if (!showroom->available_cars || !showroom->available_cars->root) {
        printf("No available cars in this showroom.\n");
    } else {
        // Find the leftmost leaf node (first car)
        BTreeNode* node = showroom->available_cars->root;
        while (!node->is_leaf) {
            node = node->children[0];
        }
        
        // Traverse all leaf nodes to print all cars
        int count = 0;
        while (node) {
            for (int i = 0; i < node->num_keys; i++) {
                count++;
                printf("%d. ", count);
                showroom->available_cars->print(node->keys[i].key);
                printf("\n");
            }
            node = node->leaf_link.next;
        }
        
        if (count == 0) {
            printf("No available cars in this showroom.\n");
        }
    }
    
    // Display sales personnel
    printf("\nSales Personnel:\n");
    if (!showroom->sales_persons || !showroom->sales_persons->root) {
        printf("No sales personnel in this showroom.\n");
    } else {
        // Find the leftmost leaf node (first salesperson)
        BTreeNode* node = showroom->sales_persons->root;
        while (!node->is_leaf) {
            node = node->children[0];
        }
        
        // Traverse all leaf nodes to print all salespeople
        int count = 0;
        while (node) {
            for (int i = 0; i < node->num_keys; i++) {
                count++;
                printf("%d. ", count);
                showroom->sales_persons->print(node->keys[i].key);
                printf("\n");
            }
            node = node->leaf_link.next;
        }
        
        if (count == 0) {
            printf("No sales personnel in this showroom.\n");
        }
    }
}




// Helper function to count total nodes in a B+ tree
int count_tree_nodes(BPlusTree* tree) {
    if (!tree || !tree->root) {
        return 0;
    }
    
    int count = 0;
    
    // Find the leftmost leaf node
    BTreeNode* node = tree->root;
    while (!node->is_leaf) {
        node = node->children[0];
    }
    
    // Traverse all leaf nodes to count all keys
    while (node) {
        count += node->num_keys;
        node = node->leaf_link.next;
    }
    
    return count;
}

// Helper function to get all keys from a B+ tree with deep copy cloning
void** getAllKeysInTree(BPlusTree* tree, int* count) {
    if (!tree || !tree->root) {
        *count = 0;
        return NULL;
    }
    
    // First, count all keys in the tree
    int total_keys = count_tree_nodes(tree);
    *count = total_keys;
    
    if (total_keys == 0) {
        return NULL;
    }
    
    // Allocate memory for all keys
    void** keys = (void**)malloc(total_keys * sizeof(void*));
    if (!keys) {
        printf("Memory allocation failed for keys array\n");
        *count = 0;
        return NULL;
    }
    
    // Find the leftmost leaf node
    BTreeNode* node = tree->root;
    while (!node->is_leaf) {
        node = node->children[0];
    }
    
    // Traverse all leaf nodes to collect all keys
    // Using the clone function to create deep copies
    int index = 0;
    while (node && index < total_keys) {
        for (int i = 0; i < node->num_keys && index < total_keys; i++) {
            // Create a deep copy of each key using the tree's clone function
            keys[index++] = tree->clone(node->keys[i].key);
        }
        node = node->leaf_link.next;
    }
    
    return keys;
}

// Helper function to merge B+ trees with proper count tracking
void merge_tree(BPlusTree *dest, BPlusTree *src, int *count) {
    if (!src || !src->root) return;
    if (!dest) return;
    
    // Get all items from source tree with deep copies
    int item_count = 0;
    void** items = getAllKeysInTree(src, &item_count);
    
    if (!items) return;
    
    // Insert each item into destination tree if not already present
    int added = 0;
    for (int i = 0; i < item_count; i++) {
        void* key = items[i];
        // Check if key already exists in destination tree
        if (!bplusSearch(dest, key)) {
            bplusInsert(dest, key);
            added++;
        }
        // Free the temporary copy since bplusInsert makes its own copy
        dest->free_func(key);
    }
    
    // Update count if provided
    if (count) {
        *count += added;
    }
    
    // Free the temporary array
    free(items);
}

// Updated helper function to merge car trees
void merge_car_tree(BPlusTree *dest, BPlusTree *src, int *count) {
    if (!src || !src->root) return;
    if (!dest) return;
    
    // Get all cars from source tree with deep copies
    int car_count = 0;
    Car** cars = (Car**)getAllKeysInTree(src, &car_count);
    
    if (!cars) return;
    
    // Insert each car into destination tree if not already present
    int added = 0;
    for (int i = 0; i < car_count; i++) {
        Car* car = cars[i];
        // Check if car already exists in destination tree
        if (!bplusSearch(dest, car)) {
            bplusInsert(dest, car);
            added++;
            printf("  Added car with VIN: %s\n", car->VIN);
        } else {
            printf("  Car with VIN %s already exists, skipping.\n", car->VIN);
        }
        // Free the temporary copy
        freeCar(car);
    }
    
    // Update count if provided
    if (count) {
        *count += added;
    }
    
    // Free the temporary array
    free(cars);
}

// Enhanced version of merge_sales_persons function with better conflict handling
void merge_sales_persons(BPlusTree *dest, BPlusTree *src) {
    if (!src || !src->root) return;
    if (!dest) return;
    
    // Get all sales persons from source tree with deep copies
    int sp_count = 0;
    SalesPerson** sales_persons = (SalesPerson**)getAllKeysInTree(src, &sp_count);
    
    if (!sales_persons) return;
    
    // Insert each sales person into destination tree if not already present
    for (int i = 0; i < sp_count; i++) {
        SalesPerson* sp_src = sales_persons[i];
        SalesPerson temp;
        temp.id = sp_src->id;
        
        // Check if sales person ID already exists
        SalesPerson* existing = (SalesPerson*)bplusSearch(dest, &temp);
        if (!existing) {
            // Simply insert the new sales person
            bplusInsert(dest, sp_src);
            printf("  Added sales person ID: %d - %s\n", sp_src->id, sp_src->name);
        } else {
            // Handle conflict by showing details and asking for resolution
            printf("  Conflict: Sales Person ID %d already exists.\n", sp_src->id);
            printf("  Existing: %s, Target: %.2f, Achieved: %.2f\n", 
                   existing->name, existing->target_sales, existing->achieved_sales);
            printf("  New: %s, Target: %.2f, Achieved: %.2f\n", 
                   sp_src->name, sp_src->target_sales, sp_src->achieved_sales);
            
            char choice;
            printf("  Keep existing (e), replace with new (n), or merge data (m)? ");
            scanf(" %c", &choice);  // Note the space before %c to skip whitespace
            getchar(); // Clear input buffer
            
            if (choice == 'n' || choice == 'N') {
                // Replace existing with new - need to delete first
                bplusDelete(dest, &temp);
                bplusInsert(dest, sp_src);
                printf("  Replaced with new sales person.\n");
            } else if (choice == 'm' || choice == 'M') {
                // Create a merged sales person
                SalesPerson* merged = (SalesPerson*)malloc(sizeof(SalesPerson));
                if (!merged) {
                    printf("  Memory allocation failed for merged sales person.\n");
                    continue;
                }
                
                // Copy basic information
                merged->id = sp_src->id;
                strcpy(merged->name, sp_src->name);
                
                // Combine targets and achievements
                merged->target_sales = existing->target_sales + sp_src->target_sales;
                merged->achieved_sales = existing->achieved_sales + sp_src->achieved_sales;
                merged->commission = existing->commission + sp_src->commission;
                
                // Create new trees for the merged sales person
                merged->customer_tree = createBPlusTree(compareCustomerByEMI, printCustomer, cloneCustomer, freeCustomer);
                merged->sold_car_tree = createBPlusTree(compareVIN, printSoldCar, cloneSoldCar, freeSoldCar);
                
                if (!merged->customer_tree || !merged->sold_car_tree) {
                    printf("  Memory allocation failed for merged sales person trees.\n");
                    if (merged->customer_tree) freeBPlusTree(merged->customer_tree);
                    if (merged->sold_car_tree) freeBPlusTree(merged->sold_car_tree);
                    free(merged);
                    continue;
                }
                
                // First delete the existing entry before inserting merged one to avoid ID conflicts
                bplusDelete(dest, &temp);
                
                // Merge customer trees if they exist
                if (existing->customer_tree)
                    merge_tree(merged->customer_tree, existing->customer_tree, NULL);
                if (sp_src->customer_tree)
                    merge_tree(merged->customer_tree, sp_src->customer_tree, NULL);
                
                // Merge sold car trees if they exist
                if (existing->sold_car_tree)
                    merge_tree(merged->sold_car_tree, existing->sold_car_tree, NULL);
                if (sp_src->sold_car_tree)
                    merge_tree(merged->sold_car_tree, sp_src->sold_car_tree, NULL);
                
                // Insert the merged sales person
                bplusInsert(dest, merged);
                
                // Free the temporary object (clone was made during insertion)
                freeSalesPerson(merged);
                
                printf("  Merged sales person data successfully.\n");
            } else {
                printf("  Keeping existing sales person.\n");
            }
        }
        
        // Free the temporary copy
        freeSalesPerson(sp_src);
    }
    
    // Free the temporary array
    free(sales_persons);
}

// Function to merge two showrooms based on user-provided IDs
void merge_showrooms() {
    int id1, id2, new_id;
    char name[MAX_STR_LEN], location[MAX_STR_LEN], contact[MAX_MOBILE_LEN];
    
    printf("\n=== Merge Showrooms ===\n");
    
    // Get the IDs of the showrooms to merge
    printf("Enter first showroom ID: ");
    scanf("%d", &id1);
    getchar(); // Clear input buffer
    
    printf("Enter second showroom ID: ");
    scanf("%d", &id2);
    getchar();
    
    // Create temporary showrooms for searching
    Showroom temp1 = { .id = id1 };
    Showroom temp2 = { .id = id2 };
    
    // Search for the showrooms in the global tree
    Showroom* showroom1 = (Showroom*)bplusSearch(showroom_tree, &temp1);
    Showroom* showroom2 = (Showroom*)bplusSearch(showroom_tree, &temp2);
    
    if (!showroom1 || !showroom2) {
        printf("Error: One or both showrooms not found.\n");
        return;
    }
    
    // Get details for the new merged showroom
    printf("\nEnter details for the merged showroom:\n");
    printf("New Showroom ID: ");
    scanf("%d", &new_id);
    getchar();
    
    // Check if new ID already exists
    Showroom temp_new = { .id = new_id };
    if (bplusSearch(showroom_tree, &temp_new)) {
        printf("Error: Showroom ID %d already exists.\n", new_id);
        return;
    }
    
    printf("New Showroom Name: ");
    fgets(name, MAX_STR_LEN, stdin);
    name[strcspn(name, "\n")] = 0;
    
    printf("New Showroom Location: ");
    fgets(location, MAX_STR_LEN, stdin);
    location[strcspn(location, "\n")] = 0;
    
    printf("New Showroom Contact Number: ");
    fgets(contact, MAX_MOBILE_LEN, stdin);
    contact[strcspn(contact, "\n")] = 0;
    
    // Create the new merged showroom
    Showroom* new_showroom = (Showroom*)malloc(sizeof(Showroom));
    if (!new_showroom) {
        printf("Memory allocation failed for new showroom\n");
        return;
    }
    
    // Initialize the new showroom
    new_showroom->id = new_id;
    strcpy(new_showroom->name, name);
    strcpy(new_showroom->location, location);
    strcpy(new_showroom->contact, contact);
    new_showroom->total_available_cars = 0;
    new_showroom->total_sold_cars = 0;
    
    // Initialize B+ trees for the new showroom
    new_showroom->available_cars = createBPlusTree(compareVIN, printCar, cloneCar, freeCar);
    new_showroom->sold_cars = createBPlusTree(compareVIN, printCar, cloneCar, freeCar);
    new_showroom->sales_persons = createBPlusTree(compareSalesPersonID, printSalesPerson, cloneSalesPerson, freeSalesPerson);
    
    if (!new_showroom->available_cars || !new_showroom->sold_cars || !new_showroom->sales_persons) {
        printf("Memory allocation failed for B+ trees\n");
        if (new_showroom->available_cars) freeBPlusTree(new_showroom->available_cars);
        if (new_showroom->sold_cars) freeBPlusTree(new_showroom->sold_cars);
        if (new_showroom->sales_persons) freeBPlusTree(new_showroom->sales_persons);
        free(new_showroom);
        return;
    }
    
    // Merge available cars
    printf("Merging available cars...\n");
    if (showroom1->available_cars)
        merge_car_tree(new_showroom->available_cars, showroom1->available_cars, &new_showroom->total_available_cars);
    if (showroom2->available_cars)
        merge_car_tree(new_showroom->available_cars, showroom2->available_cars, &new_showroom->total_available_cars);
    
    // Merge sold cars
    printf("Merging sold cars...\n");
    if (showroom1->sold_cars)
        merge_car_tree(new_showroom->sold_cars, showroom1->sold_cars, &new_showroom->total_sold_cars);
    if (showroom2->sold_cars)
        merge_car_tree(new_showroom->sold_cars, showroom2->sold_cars, &new_showroom->total_sold_cars);
    
    // Merge sales persons (handling potential ID conflicts)
    printf("Merging sales personnel...\n");
    if (showroom1->sales_persons)
        merge_sales_persons(new_showroom->sales_persons, showroom1->sales_persons);
    if (showroom2->sales_persons)
        merge_sales_persons(new_showroom->sales_persons, showroom2->sales_persons);
    
    // Insert the new showroom into the global tree
    bplusInsert(showroom_tree, new_showroom);
    
    // Print summary of the merge
    printf("\nMerge Summary:\n");
    printf("New Showroom ID: %d\n", new_showroom->id);
    printf("Name: %s\n", new_showroom->name);
    printf("Location: %s\n", new_showroom->location);
    printf("Available Cars: %d\n", new_showroom->total_available_cars);
    printf("Sold Cars: %d\n", new_showroom->total_sold_cars);
    
    // Count sales persons
    int sales_person_count = count_tree_nodes(new_showroom->sales_persons);
    printf("Sales Personnel: %d\n", sales_person_count);
    
    printf("\nShowrooms %d and %d successfully merged into new showroom %d.\n", id1, id2, new_id);
    
    // Option to delete original showrooms
    char delete_original;
    printf("\nDo you want to delete the original showrooms? (y/n): ");
    scanf(" %c", &delete_original);  // Note the space before %c to skip whitespace
    getchar(); // Clear input buffer
    
    if (delete_original == 'y' || delete_original == 'Y') {
        // Delete original showrooms from the global tree
        bplusDelete(showroom_tree, &temp1);
        bplusDelete(showroom_tree, &temp2);
        printf("Original showrooms deleted.\n");
    }

    // Display updated inventory
    printf("\nDisplaying merged showroom inventory:\n");
    temp_new.id = new_id;  // Set to display the new showroom
    Showroom* merged_showroom = (Showroom*)bplusSearch(showroom_tree, &temp_new);
    if (merged_showroom) {
        // Display the new showroom details
        printf("\nShowroom Details:\n");
        printf("ID: %d\n", merged_showroom->id);
        printf("Name: %s\n", merged_showroom->name);
        printf("Location: %s\n", merged_showroom->location);
        printf("Contact: %s\n", merged_showroom->contact);
        printf("Available Cars: %d\n", merged_showroom->total_available_cars);
        printf("Sold Cars: %d\n", merged_showroom->total_sold_cars);
        
            
    } else {
        printf("Error: Could not find newly created showroom for display.\n");
    }
    
    // No need to free new_showroom here as it's now owned by the tree
    // The bplusInsert function has already made a copy of it
}


// Global hash table
CarPopularityEntry* car_popularity_table[HASH_SIZE] = {NULL};

// Hash function for strings (car models)
unsigned int hash_model(const char* str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash * 31) + (*str++);
    }
    return hash % HASH_SIZE;
}

// Function to increment count for a car model in hashtable
void increment_car_popularity(const char* model_name) {
    unsigned int index = hash_model(model_name);
    
    // Check if model already exists in table
    CarPopularityEntry* entry = car_popularity_table[index];
    while (entry) {
        if (strcmp(entry->model_name, model_name) == 0) {
            // Found the model, increment count
            entry->count++;
            return;
        }
        entry = entry->next;
    }
    
    // Model not found, create new entry
    CarPopularityEntry* new_entry = (CarPopularityEntry*)malloc(sizeof(CarPopularityEntry));
    if (!new_entry) {
        printf("Memory allocation failed for car popularity tracking\n");
        return;
    }
    
    strncpy(new_entry->model_name, model_name, MAX_STR_LEN - 1);
    new_entry->model_name[MAX_STR_LEN - 1] = '\0';  // Ensure null-termination
    new_entry->count = 1;
    new_entry->next = car_popularity_table[index];
    car_popularity_table[index] = new_entry;
}

// Function to find the most popular car model
char* find_most_popular_car(int* max_count) {
    static char most_popular[MAX_STR_LEN] = "";
    *max_count = 0;
    
    for (int i = 0; i < HASH_SIZE; i++) {
        CarPopularityEntry* entry = car_popularity_table[i];
        while (entry) {
            if (entry->count > *max_count) {
                *max_count = entry->count;
                strncpy(most_popular, entry->model_name, MAX_STR_LEN - 1);
                most_popular[MAX_STR_LEN - 1] = '\0';  // Ensure null-termination
            }
            entry = entry->next;
        }
    }
    
    return most_popular;
}

// Function to free the hashtable memory
void free_car_popularity_table() {
    for (int i = 0; i < HASH_SIZE; i++) {
        CarPopularityEntry* current = car_popularity_table[i];
        while (current != NULL) {
            CarPopularityEntry* temp = current;
            current = current->next;
            free(temp);
        }
        car_popularity_table[i] = NULL;
    }
}

// Function to display all car models and their popularity
void display_car_popularity() {
    printf("\n=== Car Popularity Statistics ===\n");
    int total_models = 0;
    
    for (int i = 0; i < HASH_SIZE; i++) {
        CarPopularityEntry* entry = car_popularity_table[i];
        while (entry) {
            printf("Model: %-20s | Sold: %d\n", entry->model_name, entry->count);
            total_models++;
            entry = entry->next;
        }
    }
    
    if (total_models == 0) {
        printf("No cars have been sold yet.\n");
    } else {
        int max_count;
        char* most_popular = find_most_popular_car(&max_count);
        printf("\nMost Popular Model: %s (Sold: %d)\n", most_popular, max_count);
    }
}






// Function to find the most successful salesperson in a specific showroom
void find_most_successful_SP() {
    if (!showroom_tree || !showroom_tree->root) {
        printf("No showrooms registered in the system.\n");
        return;
    }
    
    // Get showroom ID from user
    int showroom_id;
    printf("\nEnter Showroom ID: ");
    scanf("%d", &showroom_id);
    
    // Create temporary showroom for search
    Showroom temp_showroom;
    temp_showroom.id = showroom_id;
    
    // Search for the showroom
    Showroom* target_showroom = (Showroom*)bplusSearch(showroom_tree, &temp_showroom);
    if (!target_showroom) {
        printf("Showroom with ID %d not found.\n", showroom_id);
        return;
    }
    
    // Check if this showroom has any salespeople
    if (!target_showroom->sales_persons || !target_showroom->sales_persons->root) {
        printf("No sales persons found in Showroom %d: %s.\n", target_showroom->id, target_showroom->name);
        return;
    }
    
    SalesPerson* best_sp = NULL;
    double highest_sales = -1;
    
    // Start with the leftmost leaf node in the salesperson tree
    BTreeNode* sp_node = target_showroom->sales_persons->root;
    while (!sp_node->is_leaf) {
        sp_node = sp_node->children[0];
    }
    
    // Traverse all salespeople in this showroom
    while (sp_node) {
        for (int j = 0; j < sp_node->num_keys; j++) {
            SalesPerson* current_sp = (SalesPerson*)sp_node->keys[j].key;
            
            // Check if this salesperson has higher sales
            if (current_sp->achieved_sales > highest_sales) {
                highest_sales = current_sp->achieved_sales;
                best_sp = current_sp;
            }
        }
        sp_node = sp_node->leaf_link.next;
    }
    
    // Print results and award incentive
    if (best_sp) {
        double incentive = best_sp->achieved_sales * 0.01; // 1% of achieved sales
        
        printf("\n=== Most Successful Sales Person in %s (ID: %d) ===\n", 
               target_showroom->name, target_showroom->id);
        printf("ID: %d\n", best_sp->id);
        printf("Name: %s\n", best_sp->name);
        printf("Target Sales: %.2f lakhs\n", best_sp->target_sales);
        printf("Achieved Sales: %.2f lakhs\n", best_sp->achieved_sales);
        printf("Current Commission: %.2f lakhs\n", best_sp->commission);
        printf("\n");
        printf("--- Awarding Additional Incentive ---\n");
        printf("Additional Incentive (1%%): %.2f lakhs\n", incentive);
        printf("Total Earnings: %.2f lakhs\n", best_sp->commission + incentive);
        
        // Update the salesperson's commission
        best_sp->commission += incentive;
        printf("\nIncentive awarded successfully!\n");
    } else {
        printf("No sales persons found in Showroom %d: %s.\n", 
               target_showroom->id, target_showroom->name);
    }
}






// Helper structure to track monthly sales
void predict_next_month_sales() {
    if (!showroom_tree || !showroom_tree->root) {
        printf("No showrooms registered in the system.\n");
        return;
    }
    
    // Get showroom ID from user
    int showroom_ID;
    printf("Enter Showroom ID for sales prediction: ");
    scanf("%d", &showroom_ID);
    
    // Create a temporary showroom to search for the specific ID
    Showroom temp_showroom = {0};
    temp_showroom.id = showroom_ID;
    
    // Search for the specific showroom
    Showroom* target_showroom = (Showroom*)bplusSearch(showroom_tree, &temp_showroom);
    if (!target_showroom) {
        printf("Showroom with ID %d not found.\n", showroom_ID);
        return;
    }
    
    // Get current date
    time_t t = time(NULL);
    struct tm* current_time = localtime(&t);
    int current_month = current_time->tm_mon + 1; // tm_mon is 0-11
    int current_year = current_time->tm_year + 1900;
    
    // Prepare array to store last 6 months of sales (or fewer if not enough data)
    MonthlySales sales_history[6] = {0};
    
    // Initialize the sales history array with the last 6 months
    for (int i = 0; i < 6; i++) {
        sales_history[i].month = current_month - i;
        sales_history[i].year = current_year;
        
        // Adjust year if we go back to previous year
        if (sales_history[i].month <= 0) {
            sales_history[i].month += 12;
            sales_history[i].year--;
        }
    }
    
    // Process each salesperson in the target showroom
    if (target_showroom->sales_persons && target_showroom->sales_persons->root) {
        BTreeNode* sp_node = target_showroom->sales_persons->root;
        while (!sp_node->is_leaf) {
            sp_node = sp_node->children[0];
        }
        
        // Traverse all salespeople in this showroom
        while (sp_node) {
            for (int j = 0; j < sp_node->num_keys; j++) {
                SalesPerson* current_sp = (SalesPerson*)sp_node->keys[j].key;
                
                // Process all customers of this salesperson to get sales data
                if (current_sp->customer_tree && current_sp->customer_tree->root) {
                    BTreeNode* customer_node = current_sp->customer_tree->root;
                    while (!customer_node->is_leaf) {
                        customer_node = customer_node->children[0];
                    }
                    
                    // Traverse all customers
                    while (customer_node) {
                        for (int k = 0; k < customer_node->num_keys; k++) {
                            Customer* current_customer = (Customer*)customer_node->keys[k].key;
                            
                            // Check purchase date to see if it falls within our 6 month window
                            for (int m = 0; m < 6; m++) {
                                if (current_customer->purchase_month == sales_history[m].month && 
                                    current_customer->purchase_year == sales_history[m].year) {
                                    sales_history[m].sales_count++;
                                    
                                    // Add the customer's actual_amount_paid to the sales value
                                    sales_history[m].sales_value += current_customer->actual_aoumnt_paid;
                                    
                                    // We've found a match, no need to check other months
                                    break;
                                }
                            }
                        }
                        customer_node = customer_node->leaf_link.next;
                    }
                }
            }
            sp_node = sp_node->leaf_link.next;
        }
    }
    
    // Display the sales history for this showroom
    printf("\n=== Sales History for Showroom ID %d ===\n", showroom_ID);
    printf("Month\tYear\tUnits Sold\tValue \n");
    for (int i = 5; i >= 0; i--) {
        printf("%d\t%d\t%d\t\t%.2f\n", 
               sales_history[i].month, sales_history[i].year, 
               sales_history[i].sales_count, sales_history[i].sales_value);
    }
    
    // Calculate simple average 
    double total_units = 0;
    double total_value = 0;
    int months_with_data = 0;
    
    for (int i = 0; i < 6; i++) {
        total_units += sales_history[i].sales_count;
        total_value += sales_history[i].sales_value;
        if (sales_history[i].sales_count > 0) {
            months_with_data++;
        }
    }
    
    // Calculate next month and year
    int next_month = current_month + 1;
    int next_year = current_year;
    if (next_month > 12) {
        next_month = 1;
        next_year++;
    }
    
    // Calculate prediction - even if no cars were sold
    double predicted_units = 0;
    double predicted_value = 0;
    
    // If we have data, calculate average, otherwise predict zero
    if (months_with_data > 0) {
        predicted_units = total_units / 6.0; // Always divide by 6 months, even if some months had zero sales
        predicted_value = total_value / 6.0;
    }
    
    // Display prediction
    printf("\n=== Sales Prediction for %d/%d (Showroom ID %d) ===\n", next_month, next_year, showroom_ID);
    printf("Predicted Units: %.1f\n", predicted_units);
    printf("Predicted Value: %.2f \n", predicted_value);
    if (predicted_units > 0) {
        printf("Average Price Per Unit: %.2f \n", predicted_value / predicted_units);
    } else {
        printf("Average Price Per Unit: 0.00 \n");
    }
}







void find_car_by_VIN() {
    char target_VIN[MAX_VIN_LEN];
    int found = 0;
    
    printf("\n=== Find Car by VIN ===\n");
    printf("Enter the VIN number: ");
    scanf("%s", target_VIN);
    
    // Check if showroom_tree is initialized
    if (!showroom_tree || !showroom_tree->root) {
        printf("No showrooms have been added yet.\n");
        return;
    }
    
    // Find the leftmost leaf node (first showroom)
    BTreeNode* node = showroom_tree->root;
    while (!node->is_leaf) {
        node = node->children[0];
    }
    
    // Traverse all showrooms
    while (node) {
        for (int i = 0; i < node->num_keys; i++) {
            Showroom* showroom = (Showroom*)node->keys[i].key;
            
            // Check available cars
            if (showroom->available_cars && showroom->available_cars->root) {
                Car* car = (Car*)bplusSearch(showroom->available_cars, &target_VIN);
                if (car) {
                    printf("\nCAR FOUND IN STOCK at %s showroom:\n", showroom->name);
                    printf("VIN: %s\n", car->VIN);
                    printf("Model: %s\n", car->name);
                    printf("Color: %s\n", car->color);
                    printf("Price: %.2f lakhs\n", car->price);
                    printf("Fuel Type: %s\n", car->fuel_type);
                    printf("Car Type: %s\n", car->car_type);
                    printf("Status: Available for purchase\n");
                    found = 1;
                }
            }
            
            // Check sold cars
            if (showroom->sold_cars && showroom->sold_cars->root) {
                SoldCar* sold_car = (SoldCar*)bplusSearch(showroom->sold_cars, &target_VIN);
                if (sold_car) {
                    printf("\nCAR FOUND (SOLD) at %s showroom:\n", showroom->name);
                    printf("VIN: %s\n", sold_car->VIN);
                    printf("Payment Type: %s\n", sold_car->payment_type);
                    
                    if (strcmp(sold_car->payment_type, "Loan") == 0) {
                        printf("Down Payment: %.2f lakhs\n", sold_car->down_payment);
                        printf("Loan Period: %d months\n", sold_car->loan_period_months);
                        printf("Loan Amount: %.2f lakhs\n", sold_car->loan_amount);
                        printf("Interest Rate: %.2f%%\n", sold_car->interest_rate);
                        printf("Monthly EMI: %.2f\n", sold_car->monthly_emi);
                    }
                    
                    // Look for customer information via sales persons
                    if (showroom->sales_persons && showroom->sales_persons->root) {
                        BTreeNode* sp_node = showroom->sales_persons->root;
                        while (!sp_node->is_leaf) {
                            sp_node = sp_node->children[0];
                        }
                        
                        int customer_found = 0;
                        while (sp_node && !customer_found) {
                            for (int j = 0; j < sp_node->num_keys && !customer_found; j++) {
                                SalesPerson* sp = (SalesPerson*)sp_node->keys[j].key;
                                
                                if (sp->customer_tree && sp->customer_tree->root) {
                                    // Look for customer with this VIN
                                    BTreeNode* cust_node = sp->customer_tree->root;
                                    while (!cust_node->is_leaf) {
                                        cust_node = cust_node->children[0];
                                    }
                                    
                                    while (cust_node && !customer_found) {
                                        for (int k = 0; k < cust_node->num_keys; k++) {
                                            Customer* customer = (Customer*)cust_node->keys[k].key;
                                            if (strcmp(customer->car_VIN, target_VIN) == 0) {
                                                printf("\nCustomer Details:\n");
                                                printf("Name: %s\n", customer->name);
                                                printf("Mobile: %s\n", customer->mobile);
                                                printf("Address: %s\n", customer->address);
                                                printf("Registration Number: %s\n", customer->reg_number);
                                                printf("Amount Paid: %.2f lakhs\n", customer->actual_aoumnt_paid);
                                                printf("Purchase Date: %d/%d/%d\n", 
                                                       customer->purchase_day,
                                                       customer->purchase_month,
                                                       customer->purchase_year);
                                                printf("Sales Person: %s (ID: %d)\n", sp->name, sp->id);
                                                customer_found = 1;
                                                break;
                                            }
                                        }
                                        cust_node = cust_node->leaf_link.next;
                                    }
                                }
                            }
                            sp_node = sp_node->leaf_link.next;
                        }
                    }
                    found = 1;
                }
            }
        }
        node = node->leaf_link.next;
    }
    
    if (!found) {
        printf("\nNo car found with VIN: %s\n", target_VIN);
    }
}






int count_nodes_in_tree(BTreeNode* node) {
    if (!node) return 0;
    
    int count = 0;
    if (node->is_leaf) {
        count = node->num_keys;
    } else {
        for (int i = 0; i <= node->num_keys; i++) {
            count += count_nodes_in_tree(node->children[i]);
        }
    }
    return count;
}

void search_salespersons_by_sales_range() {
    double min_sales, max_sales;
    int found = 0;
    
    printf("\n=== Search Sales Persons by Sales Range ===\n");
    printf("Enter minimum sales value (in lakhs): ");
    scanf("%lf", &min_sales);
    printf("Enter maximum sales value (in lakhs): ");
    scanf("%lf", &max_sales);
    
    printf("\nSales Persons with achieved sales between %.2f and %.2f lakhs:\n", min_sales, max_sales);
    printf("----------------------------------------------------------------\n");
    
    // Check if showroom_tree is initialized
    if (!showroom_tree || !showroom_tree->root) {
        printf("No showrooms have been added yet.\n");
        return;
    }
    
    // Find the leftmost leaf node (first showroom)
    BTreeNode* node = showroom_tree->root;
    while (!node->is_leaf) {
        node = node->children[0];
    }
    
    // Traverse all showrooms
    while (node) {
        for (int i = 0; i < node->num_keys; i++) {
            Showroom* showroom = (Showroom*)node->keys[i].key;
            
            if (showroom->sales_persons && showroom->sales_persons->root) {
                // Find the leftmost leaf node of sales persons
                BTreeNode* sp_node = showroom->sales_persons->root;
                while (!sp_node->is_leaf) {
                    sp_node = sp_node->children[0];
                }
                
                // Traverse all sales persons in this showroom
                while (sp_node) {
                    for (int j = 0; j < sp_node->num_keys; j++) {
                        SalesPerson* sp = (SalesPerson*)sp_node->keys[j].key;
                        
                        // Check if this sales person is within the range
                        if (sp->achieved_sales >= min_sales && sp->achieved_sales <= max_sales) {
                            found++;
                            printf("ID: %d, Name: %s, Showroom: %s\n", sp->id, sp->name, showroom->name);
                            printf("   Target: %.2f lakhs, Achieved: %.2f lakhs, Commission: %.2f\n", 
                                  sp->target_sales, sp->achieved_sales, sp->commission);
                            printf("   Cars Sold: %d\n", 
                                  sp->sold_car_tree && sp->sold_car_tree->root ? 
                                  count_nodes_in_tree(sp->sold_car_tree->root) : 0);
                            printf("----------------------------------------------------------------\n");
                        }
                    }
                    sp_node = sp_node->leaf_link.next;
                }
            }
        }
        node = node->leaf_link.next;
    }
    
    if (!found) {
        printf("No sales persons found within the specified sales range.\n");
    } else {
        printf("Total %d sales persons found within the specified range.\n", found);
    }
}





// Process function for customer range search - defined outside the main function
void process_customer_in_range(void* key, void* user_data) {
    Customer* customer = (Customer*)key;
    
    // This struct holds all necessary context for processing
    typedef struct {
        int* count;           // Pointer to total count
        int* showroom_matches; // Pointer to showroom count
        Showroom* showroom;    // Current showroom
    } RangeSearchContext;
    
    RangeSearchContext* context = (RangeSearchContext*)user_data;
    SoldCar* sold_car = NULL;
    
    // Find the sold car in the showroom's sold_cars tree
    if (context->showroom->sold_cars && context->showroom->sold_cars->root) {
        sold_car = (SoldCar*)bplusSearch(context->showroom->sold_cars, customer->car_VIN);
    }
    
    if (sold_car && strcmp(sold_car->payment_type, "Loan") == 0) {
        (*context->count)++;
        (*context->showroom_matches)++;
        
        printf("Customer: %s\n", customer->name);
        printf("  Mobile: %s\n", customer->mobile);
        printf("  Car VIN: %s\n", customer->car_VIN);
        printf("  EMI Period: %d months\n", sold_car->loan_period_months);
        printf("  Monthly EMI: %.2f\n", sold_car->monthly_emi);
        printf("  Down Payment: %.2f\n", sold_car->down_payment);
        printf("  Loan Amount: %.2f\n", sold_car->loan_amount);
        printf("  Interest Rate: %.2f%%\n\n", sold_car->interest_rate);
    }
}

// Main function to list customers with EMI plans within a user-specified range
void list_customers_with_emi_in_range() {
    // Get range from user
    int min_months, max_months;
    
    printf("\n=== Search Customers by EMI Plan Duration ===\n");
    printf("Enter minimum months (inclusive): ");
    if (scanf("%d", &min_months) != 1) {
        printf("Invalid input. Please enter a valid number.\n");
        // Clear input buffer
        while (getchar() != '\n');
        return;
    }
    
    printf("Enter maximum months (inclusive): ");
    if (scanf("%d", &max_months) != 1) {
        printf("Invalid input. Please enter a valid number.\n");
        // Clear input buffer
        while (getchar() != '\n');
        return;
    }
    
    // Validate input
    if (min_months > max_months) {
        printf("Error: Maximum months must be greater than or equal to minimum months.\n");
        return;
    }
    
    printf("\n=== Customers with EMI Plans Between %d-%d Months ===\n", 
           min_months, max_months);
    
    if (!showroom_tree || !showroom_tree->root) {
        printf("No showrooms available.\n");
        return;
    }
    
    int customer_count = 0;
    
    // Find the leftmost leaf node (first showroom)
    BTreeNode* showroom_node = showroom_tree->root;
    while (!showroom_node->is_leaf) {
        showroom_node = showroom_node->children[0];
    }
    
    // Traverse all showrooms
    while (showroom_node) {
        for (int i = 0; i < showroom_node->num_keys; i++) {
            Showroom* showroom = (Showroom*)showroom_node->keys[i].key;
            
            printf("Checking Showroom: %s (ID: %d)\n", showroom->name, showroom->id);
            
            // Skip if no salespersons
            if (!showroom->sales_persons || !showroom->sales_persons->root) {
                printf("  No salespersons in this showroom.\n\n");
                continue;
            }
            
            int showroom_matches = 0;
            
            // Find the leftmost leaf node (first salesperson)
            BTreeNode* sp_node = showroom->sales_persons->root;
            while (!sp_node->is_leaf) {
                sp_node = sp_node->children[0];
            }
            
            // Traverse all salespersons in this showroom
            while (sp_node) {
                for (int j = 0; j < sp_node->num_keys; j++) {
                    SalesPerson* sp = (SalesPerson*)sp_node->keys[j].key;
                    
                    if (!sp || !sp->customer_tree || !sp->customer_tree->root) {
                        continue;  // Skip if no customers
                    }
                    
                    printf("  Checking Salesperson: %s (ID: %d)\n", sp->name, sp->id);
                    
                    // Create context data for the process function
                    typedef struct {
                        int* count;
                        int* showroom_matches;
                        Showroom* showroom;
                    } RangeSearchContext;
                    
                    RangeSearchContext context = {&customer_count, &showroom_matches, showroom};
                    
                    // Create temporary Customer objects for range bounds
                    Customer min_customer, max_customer;
                    min_customer.loan_months = min_months;
                    max_customer.loan_months = max_months;
                    
                    // Perform range search on this salesperson's customer tree
                    int before_count = customer_count;
                    bplusRangeSearch(sp->customer_tree, &min_customer, &max_customer, 
                                     process_customer_in_range, &context);
                    
                    // Report results for this salesperson
                    int sp_matches = customer_count - before_count;
                    if (sp_matches == 0) {
                        printf("    No customers with EMI plans between %d-%d months found with this salesperson.\n", 
                               min_months, max_months);
                    } else {
                        printf("    Found %d customers with matching EMI plans.\n", sp_matches);
                    }
                }
                sp_node = sp_node->leaf_link.next;
            }
            
            // Report if no matches found in this showroom
            if (showroom_matches == 0) {
                printf("  No customers with EMI plans between %d-%d months found in this showroom.\n\n", 
                       min_months, max_months);
            } else {
                printf("  Total customers in showroom with matching EMI plans: %d\n\n", showroom_matches);
            }
        }
        showroom_node = showroom_node->leaf_link.next;
    }
    
    if (customer_count == 0) {
        printf("\nNo customers with EMI plans between %d-%d months found in any showroom.\n", 
               min_months, max_months);
    } else {
        printf("\nTotal customers found across all showrooms: %d\n", customer_count);
    }
}