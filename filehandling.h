#ifndef FILE_HANDLING_H
#define FILE_HANDLING_H

#include "essentialfunction.h"

// File paths
#define SHOWROOMS_FILE "data/showrooms.txt"
#define CARS_FILE "data/cars.txt"
#define SOLD_CARS_FILE "data/sold_cars.txt"
#define SALESPERSONS_FILE "data/salespersons.txt"
#define CUSTOMERS_FILE "data/customers.txt"
#define CAR_POPULARITY_FILE "data/car_popularity.txt"

// Function to ensure data directory exists
void ensure_data_directory();

// Save functions
void save_showrooms_to_file();
void save_cars_to_file(Showroom* showroom, FILE* file);
void save_sold_cars_to_file(Showroom* showroom, FILE* file);
void save_salespersons_to_file(Showroom* showroom, FILE* file);
void save_customers_to_file(SalesPerson* salesperson, FILE* file);
void save_car_popularity_to_file();

// Load functions
void load_showrooms_from_file();
void load_cars_from_file();
void load_sold_cars_from_file();
void load_salespersons_from_file();
void load_customers_from_file();
void load_car_popularity_from_file();

// Main file handling functions (added explicit declaration here)
void save_all_data();
void load_all_data();

#endif