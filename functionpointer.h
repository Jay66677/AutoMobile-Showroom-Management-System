#ifndef FUNCTION_POINTER_H
#define FUNCTION_POINTER_H

#include "b+treetemplate.h"

#define MAX_STR_LEN 100
#define MAX_VIN_LEN 20
#define MAX_REG_NUM_LEN 15
#define MAX_MOBILE_LEN 15
#define MIN_DOWN_PAYMENT_PERCENT 20.0
#define HASH_SIZE 101  

typedef struct CarPopularityEntry {
    char model_name[MAX_STR_LEN];
    int count;
    struct CarPopularityEntry* next;  // For collision handling 
} CarPopularityEntry;


// Structure for Car
typedef struct {
    char VIN[MAX_VIN_LEN];          // Vehicle Identification Number (Primary Key)
    char name[MAX_STR_LEN];         // Car Model Name
    char color[MAX_STR_LEN];        // Color of Car
    double price;                   // Price in lakhs of rupees
    char fuel_type[MAX_STR_LEN];    // Petrol, Diesel, Electric, etc.
    char car_type[MAX_STR_LEN];     // Type of car 
} Car;

// Structure for Sold Car
typedef struct {
    char VIN[MAX_VIN_LEN];
    char payment_type[MAX_STR_LEN];     // Cash or Loan
    
    // Loan details
    double down_payment;
    int loan_period_months;
    double loan_amount;
    double interest_rate;
    double monthly_emi;
} SoldCar;

// Structure for Customer
typedef struct {
    char name[MAX_STR_LEN];
    char mobile[MAX_MOBILE_LEN];
    char address[MAX_STR_LEN];
    char car_VIN[MAX_VIN_LEN];
    char reg_number[MAX_REG_NUM_LEN];
    double actual_aoumnt_paid;
    //date fields
    int purchase_day;
    int purchase_month;
    int purchase_year;
    //EMI field for range search
    int loan_months;
} Customer;

// Structure for Sales Person
typedef struct {
    int id;
    char name[MAX_STR_LEN];
    double target_sales;          // Monthly target in lakhs
    double achieved_sales;        // Achieved in lakhs
    double commission;            // Calculated commission
    
    // Trees for customers and sold cars for this sales person
    BPlusTree* customer_tree;
    BPlusTree* sold_car_tree;
} SalesPerson;

// Structure for Showroom
typedef struct {
    int id;                         // Unique Showroom ID
    char name[MAX_STR_LEN];
    char location[MAX_STR_LEN];
    char contact[MAX_MOBILE_LEN];
    
    // Trees
    BPlusTree* available_cars;
    BPlusTree* sold_cars;
    BPlusTree* sales_persons;
    
    int total_available_cars;
    int total_sold_cars;
} Showroom;

// Function prototypes for B+ tree operations
int compareInt(const void* a, const void* b);
int compareStr(const void* a, const void* b);
void printInt(const void* data);
void printStr(const void* data);
void* cloneInt(const void* data);
void* cloneStr(const void* data);
void freeInt(void* data);
void freeStr(void* data);

// Car related functions
int compareVIN(const void* a, const void* b);
void printCar(const void* data);
void* cloneCar(const void* data);
void freeCar(void* data);

// SoldCar related functions
void printSoldCar(const void* data);
void* cloneSoldCar(const void* data);
void freeSoldCar(void* data);

// Customer related functions
int compareCustomerByEMI(const void* a, const void* b); //need compare
void printCustomer(const void* data);
void* cloneCustomer(const void* data);
void freeCustomer(void* data);

// SalesPerson related functions
int compareSalesPersonID(const void* a, const void* b);
void printSalesPerson(const void* data);
void* cloneSalesPerson(const void* data);
void freeSalesPerson(void* data);

// Showroom related functions
int compareShowroomID(const void* a, const void* b);
void printShowroom(const void* data);
void* cloneShowroom(const void* data);
void freeShowroom(void* data);


#endif