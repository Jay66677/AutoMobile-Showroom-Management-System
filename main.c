#include "essentialfunction.h"
#include "filehandling.h"

// Main function with menu for testing
int main() {
    int choice;
    
    // Initialize the system
    init_system();
    
    // Load data from files
    load_all_data();
    
    printf("Car Showroom Management System\n");
    
    do {
        printf("\n=== Main Menu ===\n");
        printf("1. Add Showroom\n");
        printf("2. Display All Showrooms\n");
        printf("3. Add New Car Stock\n");
        printf("4. Recruit Sales Person\n");
        printf("5. Car Purchase\n");
        printf("6. Merge Showrooms\n");
        printf("7. Display Showroom Inventory\n");
        printf("8. Find Most Successful Sales Person\n");
        printf("9. Predict Next Month's Sales\n");
        printf("10. Find Car by VIN\n");
        printf("11. Search Sales Persons by Sales Range\n");
        printf("12. Display Car Popularity Statistics\n");
        printf("13. Display the details of cars within given EMI plan\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                add_showroom();
                break;
            case 2:
                printf("\n=== All Showrooms ===\n");
                if (!showroom_tree || !showroom_tree->root) {
                    printf("No showrooms have been added yet.\n");
                } else {
                    // Find the leftmost leaf node (first showroom)
                    BTreeNode* node = showroom_tree->root;
                    while (!node->is_leaf) {
                        node = node->children[0];
                    }
                    
                    // Traverse all leaf nodes to print all showrooms
                    int count = 0;
                    while (node) {
                        for (int i = 0; i < node->num_keys; i++) {
                            count++;
                            printf("%d. ", count);
                            showroom_tree->print(node->keys[i].key);
                            printf("\n");
                        }
                        node = node->leaf_link.next;
                    }
                    
                    if (count == 0) {
                        printf("No showrooms have been added yet.\n");
                    }
                }
                break;
            case 3:
                add_new_stock();
                break;
            case 4:
                recruit_salesperson();
                break;
            case 5:
                car_purchase();
                break;
            case 6:
                merge_showrooms();
                break;
            case 7:
                display_showroom_inventory();
                break;
            case 8:
                find_most_successful_SP();
                break;
            case 9:
                predict_next_month_sales();
                break;
            case 10:
                find_car_by_VIN();
                break;
            case 11:
                search_salespersons_by_sales_range();
                break;
            case 12:
                display_car_popularity();
                break;
            case 13:
                list_customers_with_emi_in_range();
                break;
            case 0:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);
    
    // Save data to files before exiting
    save_all_data();
    
    // Free memory before exiting
    if (showroom_tree) {
        freeBPlusTree(showroom_tree);
    }

    free_car_popularity_table();
    
    return 0;
}