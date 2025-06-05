#include "functionpointer.h"
#include "essentialfunction.h"

// Global variables
BPlusTree* showroom_tree = NULL;



// Helper functions for loan calculation
double calculate_interest_rate(int months) {
    if (months <= 36) {
        return 8.50;
    } else if (months <= 60) {
        return 8.75;
    } else {
        return 9.00;  // For 84 months
    }
}

double calculate_emi(double loan_amount, double interest_rate, int months) {
    double monthly_rate = interest_rate / (12 * 100);
    double emi = loan_amount * monthly_rate * pow(1 + monthly_rate, months) / (pow(1 + monthly_rate, months) - 1);
    return emi;
}

// Initialize the showroom management system
void init_system() {
    showroom_tree = createBPlusTree(compareShowroomID, printShowroom, cloneShowroom, freeShowroom);
}

// Function to add a new showroom to the system
void add_showroom() {
    int id;
    char name[MAX_STR_LEN];
    char location[MAX_STR_LEN];
    char contact[MAX_MOBILE_LEN];
    
    printf("\n=== Add New Showroom ===\n");
    
    printf("Enter Showroom ID: ");
    scanf("%d", &id);
    getchar(); // Clear input buffer
    
    printf("Enter Showroom Name: ");
    fgets(name, MAX_STR_LEN, stdin);
    name[strcspn(name, "\n")] = 0; // Remove newline character
    
    printf("Enter Showroom Location: ");
    fgets(location, MAX_STR_LEN, stdin);
    location[strcspn(location, "\n")] = 0; // Remove newline character
    
    printf("Enter Contact Number: ");
    fgets(contact, MAX_MOBILE_LEN, stdin);
    contact[strcspn(contact, "\n")] = 0; // Remove newline character
    
    // Create a new showroom
    Showroom* showroom = (Showroom*)malloc(sizeof(Showroom));
    if (!showroom) {
        printf("Memory allocation failed for showroom\n");
        return;
    }
    
    showroom->id = id;
    strcpy(showroom->name, name);
    strcpy(showroom->location, location);
    strcpy(showroom->contact, contact);
    showroom->total_available_cars = 0;
    showroom->total_sold_cars = 0;
    
    // Initialize the B+ trees for the showroom
    showroom->available_cars = createBPlusTree(compareVIN, printCar, cloneCar, freeCar);
    showroom->sold_cars = createBPlusTree(compareVIN, printCar, cloneCar, freeCar);
    showroom->sales_persons = createBPlusTree(compareSalesPersonID, printSalesPerson, cloneSalesPerson, freeSalesPerson);
    
    // Add the showroom to the global tree
    if (!showroom_tree) {
        init_system();
    }
    
    // Check if showroom with same ID already exists
    Showroom* existing = (Showroom*)bplusSearch(showroom_tree, showroom);
    if (existing) {
        printf("A showroom with ID %d already exists.\n", id);
        freeShowroom(showroom);  // Free the memory as we won't use this showroom
        return;
    }
    
    // Insert the showroom into the B+ tree
    bplusInsert(showroom_tree, showroom);
    
    printf("Showroom '%s' added successfully with ID %d.\n", name, id);
    freeShowroom(showroom);  // Free memory as B+ tree has cloned the data
}

// Function to add a new car to a showroom's available cars
void add_new_stock() {
    int showroom_id;
    Car car;
    
    printf("\n=== Add New Car Stock ===\n");
    
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
    
    // Get car details
    printf("Enter Vehicle Identification Number (VIN): ");
    fgets(car.VIN, MAX_VIN_LEN, stdin);
    car.VIN[strcspn(car.VIN, "\n")] = 0; // Remove newline
    
    // Check if car with this VIN already exists
    Car* existing_car = (Car*)bplusSearch(showroom->available_cars, &car);
    if (existing_car) {
        printf("Car with VIN %s already exists in this showroom.\n", car.VIN);
        return;
    }
    
    printf("Enter Car Model Name: ");
    fgets(car.name, MAX_STR_LEN, stdin);
    car.name[strcspn(car.name, "\n")] = 0;
    
    printf("Enter Car Color: ");
    fgets(car.color, MAX_STR_LEN, stdin);
    car.color[strcspn(car.color, "\n")] = 0;
    
    printf("Enter Car Price (in lakhs): ");
    scanf("%lf", &car.price);
    getchar(); // Clear input buffer
    
    printf("Enter Fuel Type (Petrol, Diesel, Electric, etc.): ");
    fgets(car.fuel_type, MAX_STR_LEN, stdin);
    car.fuel_type[strcspn(car.fuel_type, "\n")] = 0;
    
    printf("Enter Car Type (Sedan, SUV, Hatchback, etc.): ");
    fgets(car.car_type, MAX_STR_LEN, stdin);
    car.car_type[strcspn(car.car_type, "\n")] = 0;
    
    // Add the car to the showroom's available cars
    bplusInsert(showroom->available_cars, &car);
    showroom->total_available_cars++;
    
    printf("Car with VIN %s added successfully to showroom %d.\n", car.VIN, showroom_id);
    
    // Display the added car
    printf("\nCar Details:\n");
    printf("VIN: %s\n", car.VIN);
    printf("Model: %s\n", car.name);
    printf("Color: %s\n", car.color);
    printf("Price: %.2f lakhs\n", car.price);
    printf("Fuel Type: %s\n", car.fuel_type);
    printf("Car Type: %s\n", car.car_type);
}

// Function to recruit a salesperson for a specific showroom
void recruit_salesperson() {
    int showroom_id;
    SalesPerson sales_person;
    
    printf("\n=== Recruit New Sales Person ===\n");
    
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
    
    // Get salesperson details
    printf("Enter Sales Person ID: ");
    scanf("%d", &sales_person.id);
    getchar(); // Clear input buffer
    
    // Check if salesperson with this ID already exists
    SalesPerson* existing_sp = (SalesPerson*)bplusSearch(showroom->sales_persons, &sales_person);
    if (existing_sp) {
        printf("Sales Person with ID %d already exists in this showroom.\n", sales_person.id);
        return;
    }
    
    printf("Enter Sales Person Name: ");
    fgets(sales_person.name, MAX_STR_LEN, stdin);
    sales_person.name[strcspn(sales_person.name, "\n")] = 0;
    
    printf("Enter Monthly Target Sales (in lakhs): ");
    scanf("%lf", &sales_person.target_sales);
    getchar(); // Clear input buffer
    
    // Initialize other fields
    sales_person.achieved_sales = 0.0;
    sales_person.commission = 0.0;
    
    // Initialize B+ trees for the sales person
    sales_person.customer_tree = createBPlusTree(compareCustomerByEMI, printCustomer, cloneCustomer, freeCustomer);
    sales_person.sold_car_tree = createBPlusTree(compareVIN, printSoldCar, cloneSoldCar, freeSoldCar);
    
    // Add the salesperson to the showroom
    bplusInsert(showroom->sales_persons, &sales_person);
    
    printf("Sales Person '%s' with ID %d recruited successfully for showroom %d.\n", 
           sales_person.name, sales_person.id, showroom_id);
    
    // Display the added salesperson
    printf("\nSales Person Details:\n");
    printf("ID: %d\n", sales_person.id);
    printf("Name: %s\n", sales_person.name);
    printf("Target Sales: %.2f lakhs\n", sales_person.target_sales);
    printf("Achieved Sales: %.2f lakhs\n", sales_person.achieved_sales);
    printf("Commission: %.2f lakhs\n", sales_person.commission);
    
    // Free the trees as they've been cloned during insertion
    freeBPlusTree(sales_person.customer_tree);
    freeBPlusTree(sales_person.sold_car_tree);
}

// Function to handle car purchase by a customer
void car_purchase() {
    int showroom_id, salesperson_id;
    char car_vin[MAX_VIN_LEN];
    char payment_type[MAX_STR_LEN];
    
    printf("\n=== Car Purchase ===\n");
    
    // Get showroom ID
    printf("Enter Showroom ID: ");
    scanf("%d", &showroom_id);
    getchar(); // Clear input buffer
    
    // Create a temporary showroom object to search the tree
    Showroom temp_showroom;
    temp_showroom.id = showroom_id;
    
    // Search for the showroom
    Showroom* showroom = (Showroom*)bplusSearch(showroom_tree, &temp_showroom);
    if (!showroom) {
        printf("Showroom with ID %d not found.\n", showroom_id);
        return;
    }
    
    // Get salesperson ID
    printf("Enter Salesperson ID: ");
    scanf("%d", &salesperson_id);
    getchar(); // Clear input buffer
    
    // Create a temporary salesperson object to search in the showroom's salesperson tree
    SalesPerson temp_salesperson;
    temp_salesperson.id = salesperson_id;
    
    // Search for the salesperson in the showroom
    SalesPerson* salesperson = (SalesPerson*)bplusSearch(showroom->sales_persons, &temp_salesperson);
    if (!salesperson) {
        printf("Salesperson with ID %d not found in this showroom.\n", salesperson_id);
        return;
    }
    
    // Get car VIN
    printf("Enter Car VIN to purchase: ");
    fgets(car_vin, MAX_VIN_LEN, stdin);
    car_vin[strcspn(car_vin, "\n")] = 0; // Remove newline
    
    // Create a temporary car object to search
    Car temp_car;
    strcpy(temp_car.VIN, car_vin);
    
    // Search for the car in the showroom's available cars
    Car* car = (Car*)bplusSearch(showroom->available_cars, &temp_car);
    if (!car) {
        printf("Car with VIN %s not found in this showroom's available cars.\n", car_vin);
        return;
    }
    
    // Display car details for confirmation
    printf("\nCar Details:\n");
    printf("VIN: %s\n", car->VIN);
    printf("Model: %s\n", car->name);
    printf("Color: %s\n", car->color);
    printf("Price: %.2f lakhs\n", car->price);
    printf("Fuel Type: %s\n", car->fuel_type);
    printf("Car Type: %s\n", car->car_type);
    
    // Get payment details
    printf("\nEnter Payment Type (Cash/Loan): ");
    fgets(payment_type, MAX_STR_LEN, stdin);
    payment_type[strcspn(payment_type, "\n")] = 0; // Remove newline
    
    // Create a sold car record
    SoldCar sold_car;
    strcpy(sold_car.VIN, car_vin);
    strcpy(sold_car.payment_type, payment_type);

    
    // If payment is through loan, get loan details
    if (strcmp(payment_type, "Loan") == 0) {
        double min_down_payment = car->price * (MIN_DOWN_PAYMENT_PERCENT / 100.0);
        printf("Minimum down payment required (%.2f%%): %.2f lakhs\n", 
               MIN_DOWN_PAYMENT_PERCENT, min_down_payment);
        
        do {
            printf("Enter Down Payment (in lakhs): ");
            scanf("%lf", &sold_car.down_payment);
            getchar(); // Clear input buffer
            
            
            if (sold_car.down_payment < min_down_payment) {
                printf("Down payment must be at least %.2f lakhs (%.2f%% of car price).\n",
                       min_down_payment, MIN_DOWN_PAYMENT_PERCENT);
            }
        } while (sold_car.down_payment < min_down_payment);
        
        int valid_loan_periods[3] = {36, 60, 84}; // 3, 5, or 7 years
        int period_choice;
        
        printf("Select Loan Period:\n");
        printf("1. 36 months (3 years) - Interest Rate: 8.50%%\n");
        printf("2. 60 months (5 years) - Interest Rate: 8.75%%\n");
        printf("3. 84 months (7 years) - Interest Rate: 9.00%%\n");
        printf("Enter choice (1-3): ");
        scanf("%d", &period_choice);
        getchar(); // Clear input buffer
        
        if (period_choice < 1 || period_choice > 3) {
            period_choice = 1; // Default to 36 months if invalid choice
            printf("Invalid choice. Defaulting to 36 months.\n");
        }
        
        // Set loan period based on user choice
        sold_car.loan_period_months = valid_loan_periods[period_choice - 1];
        
        // Calculate loan amount (car price - down payment)
        sold_car.loan_amount = car->price - sold_car.down_payment;
        
        // Calculate interest rate based on loan period
        sold_car.interest_rate = calculate_interest_rate(sold_car.loan_period_months);
        
        // Calculate EMI
        sold_car.monthly_emi = calculate_emi(sold_car.loan_amount, sold_car.interest_rate, sold_car.loan_period_months);
        
        printf("\nLoan Details:\n");
        printf("Loan Amount: %.2f lakhs\n", sold_car.loan_amount);
        printf("Interest Rate: %.2f%%\n", sold_car.interest_rate);
        printf("Loan Period: %d months\n", sold_car.loan_period_months);
        printf("Monthly EMI: %.2f\n", sold_car.monthly_emi);
    } else {
        // For cash payment, set loan-related fields to 0
        sold_car.down_payment = car->price; // Full amount is paid
        sold_car.loan_amount = 0;
        sold_car.loan_period_months = 0;
        sold_car.interest_rate = 0;
        sold_car.monthly_emi = 0;
    }
    
    // Now get customer details
    Customer customer;
    strcpy(customer.car_VIN, car_vin);
    customer.actual_aoumnt_paid=car->price;
    customer.loan_months=sold_car.loan_period_months;
    
    printf("\nEnter Customer Details:\n");
    printf("Name: ");
    fgets(customer.name, MAX_STR_LEN, stdin);
    customer.name[strcspn(customer.name, "\n")] = 0; // Remove newline
    
    printf("Mobile Number: ");
    fgets(customer.mobile, MAX_MOBILE_LEN, stdin);
    customer.mobile[strcspn(customer.mobile, "\n")] = 0; // Remove newline
    
    printf("Address: ");
    fgets(customer.address, MAX_STR_LEN, stdin);
    customer.address[strcspn(customer.address, "\n")] = 0; // Remove newline
    
    printf("Registration Number: ");
    fgets(customer.reg_number, MAX_REG_NUM_LEN, stdin);
    customer.reg_number[strcspn(customer.reg_number, "\n")] = 0; // Remove newline

    // Add current date information
    time_t t = time(NULL);
    struct tm* current_time = localtime(&t);
    customer.purchase_day = current_time->tm_mday;
    customer.purchase_month = current_time->tm_mon + 1; // tm_mon is 0-11
    customer.purchase_year = current_time->tm_year + 1900;

    
    // Confirm purchase
    char confirm;
    printf("\nConfirm car purchase (y/n): ");
    scanf("%c", &confirm);
    getchar(); // Clear input buffer
    
    if (confirm != 'y' && confirm != 'Y') {
        printf("Purchase cancelled.\n");
        return;
    }
    
    // Add sold car to salesperson's sold_car_tree
    bplusInsert(salesperson->sold_car_tree, &sold_car);

    //Add sold car to showroom data 
    bplusInsert(showroom->sold_cars,&sold_car);
    
    // Add customer to salesperson's customer_tree
    bplusInsert(salesperson->customer_tree, &customer);
    
    // Update car popularity hashtable with the sold car's model
    increment_car_popularity(car->name);
    
    // Update salesperson's achieved sales
    salesperson->achieved_sales += car->price;
    
    // Calculate commission (assuming 2% of car price)
    double commission = car->price * 0.02;
    salesperson->commission += commission;
    
    // Remove car from available cars
    bplusDelete(showroom->available_cars, &temp_car);
    
    // Update showroom statistics
    showroom->total_available_cars--;
    showroom->total_sold_cars++;
    
    printf("\nCar purchase successful!\n");
    printf("Car: %s %s\n", car->name, car->color);
    printf("Customer: %s\n", customer.name);
    printf("Salesperson: %s (ID: %d)\n", salesperson->name, salesperson->id);
    printf("Total Price: %.2f lakhs\n", car->price);
    printf("Payment Method: %s\n", payment_type);
    
    if (strcmp(payment_type, "Loan") == 0) {
        printf("Down Payment: %.2f lakhs\n", sold_car.down_payment);
        printf("Loan Amount: %.2f lakhs\n", sold_car.loan_amount);
        printf("Monthly EMI: %.2f for %d months\n", sold_car.monthly_emi, sold_car.loan_period_months);
    }
    
    printf("Registration Number: %s\n", customer.reg_number);
    printf("Salesperson Commission: %.2f lakhs\n", commission);
}