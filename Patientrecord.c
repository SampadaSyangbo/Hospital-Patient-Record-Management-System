#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MEDICINES 10     // Max medicines/tests per patient
#define MAX_NAME_LEN 50      // Max length for names, disease, etc.
#define FILE_NAME "patients.dat" // File to store patient records

// Structure to hold medicine/test name
typedef struct {
    char name[MAX_NAME_LEN];
} Medicine;

// Structure to hold patient information
typedef struct {
    int id;                         // Patient ID
    char name[MAX_NAME_LEN];        // Patient name
    int age;                        // Patient age
    char disease[MAX_NAME_LEN];     // Disease diagnosis
    char admitted_date[11];         // Admission date, format yyyy-mm-dd
    Medicine medicines[MAX_MEDICINES]; // Array of medicines/tests prescribed
    int med_count;                  // Number of medicines/tests prescribed
    float billing;                  // Patient billing amount
} Patient;

// Function declarations
void addPatient();
void updatePatient();
void searchPatient();
void savePatient(const Patient *p);
int loadPatients(Patient **patients);
void listPatients();

// Utility to input a string with prompt and removing newline
void inputString(char *prompt, char *buffer, int size) {
    printf("%s", prompt);
    fgets(buffer, size, stdin);
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline character from input
}

int main() {
    int choice;

    while (1) {
        // Display menu options
        printf("\n--- Hospital Patient Record System ---\n");
        printf("1. Add New Patient\n");
        printf("2. Update Treatment/Billing\n");
        printf("3. Search Patient\n");
        printf("4. List All Patients\n");
        printf("5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        getchar(); // consume leftover newline from input buffer

        // Handle user's choice
        switch (choice) {
            case 1:
                addPatient();    // Add a new patient record
                break;
            case 2:
                updatePatient(); // Update treatment/billing for existing patient
                break;
            case 3:
                searchPatient(); // Search patients by ID or disease
                break;
            case 4:
                listPatients();  // Show a list of all patients
                break;
            case 5:
                printf("Exiting...\n");
                exit(0);         // Exit program
            default:
                printf("Invalid option, try again.\n");
        }
    }
    return 0;
}

// Function to add a new patient record and save to file
void addPatient() {
    FILE *fp = fopen(FILE_NAME, "ab"); // Open file in append-binary mode
    if (!fp) {
        perror("Cannot open file");
        return;
    }

    Patient p;

    // Take input details for the patient
    printf("Enter patient ID: ");
    scanf("%d", &p.id);
    getchar(); // consume newline left after scanf

    // Check for duplicate ID
    Patient *patients = NULL;
    int count = loadPatients(&patients);
    for (int i = 0; i < count; i++) {
        if (patients[i].id == p.id) {
            printf("Error: Patient ID %d already exists.\n", p.id);
            free(patients);
            fclose(fp);
            return;
        }
    }
    free(patients);

    inputString("Enter patient name: ", p.name, MAX_NAME_LEN);

    printf("Enter age: ");
    scanf("%d", &p.age);
    getchar();

    inputString("Enter disease: ", p.disease, MAX_NAME_LEN);
    inputString("Enter admitted date (yyyy-mm-dd): ", p.admitted_date, 11);

    printf("Enter number of medicines/tests prescribed (max %d): ", MAX_MEDICINES);
    scanf("%d", &p.med_count);
    getchar();

    // Input medicine/test names into array
    for (int i = 0; i < p.med_count; i++) {
        char medName[MAX_NAME_LEN];
        printf("Enter medicine/test name %d: ", i + 1);
        fgets(medName, MAX_NAME_LEN, stdin);
        medName[strcspn(medName, "\n")] = 0; // Remove newline
        // Copy into patient medicine array
        strncpy(p.medicines[i].name, medName, MAX_NAME_LEN);
    }

    printf("Enter initial billing amount: ");
    scanf("%f", &p.billing);
    getchar();

    // Write patient struct to file
    fwrite(&p, sizeof(Patient), 1, fp);
    fclose(fp);

    printf("Patient added successfully.\n");
}

// Function to update treatment (add medicine/test) or billing of a patient by ID
void updatePatient() {
    int id;
    printf("Enter patient ID to update: ");
    scanf("%d", &id);
    getchar();

    FILE *fp = fopen(FILE_NAME, "r+b"); // Open file for reading and writing binary
    if (!fp) {
        perror("Cannot open file");
        return;
    }

    Patient p;
    int found = 0;
    long pos;

    // Search for patient by ID in file
    while (fread(&p, sizeof(Patient), 1, fp)) {
        if (p.id == id) {
            found = 1;
            pos = ftell(fp) - sizeof(Patient); // Remember position to update later
            break;
        }
    }

    if (!found) {
        printf("Patient with ID %d not found.\n", id);
        fclose(fp);
        return;
    }

    printf("Patient found: %s, Disease: %s\n", p.name, p.disease);

    // Menu for update options
    printf("\n-- Update Options --\n");
    printf("1. Add new medicine/test\n");
    printf("2. Update billing amount\n");
    printf("3. Cancel\n");
    printf("Choose an option: ");

    int choice;
    scanf("%d", &choice);
    getchar();

    switch (choice) {
        case 1:
            // Add new medicine/test if array not full
            if (p.med_count >= MAX_MEDICINES) {
                printf("Medicine/test list is full, cannot add more.\n");
            } else {
                char medName[MAX_NAME_LEN];
                printf("Enter new medicine/test name: ");
                fgets(medName, MAX_NAME_LEN, stdin);
                medName[strcspn(medName, "\n")] = 0;

                strncpy(p.medicines[p.med_count].name, medName, MAX_NAME_LEN);
                p.med_count++;
                printf("Medicine/test added.\n");
            }
            break;

        case 2:
            // Update billing by adding amount to total billing
            printf("Current billing amount: %.2f\n", p.billing);
            float amount;
            printf("Enter new billing amount to add: ");
            scanf("%f", &amount);
            getchar();
            p.billing += amount;
            printf("Billing updated. New total: %.2f\n", p.billing);
            break;

        case 3:
            printf("Update cancelled.\n");
            fclose(fp);
            return;

        default:
            printf("Invalid choice.\n");
            fclose(fp);
            return;
    }

    // Move file pointer to patient's record position and overwrite it
    fseek(fp, pos, SEEK_SET);
    fwrite(&p, sizeof(Patient), 1, fp);
    fclose(fp);
}

// Function to search patient by ID or by disease substring
void searchPatient() {
    int choice;
    printf("Search by:\n1. ID\n2. Disease\nChoose: ");
    scanf("%d", &choice);
    getchar();

    Patient *patients = NULL;
    int count = loadPatients(&patients);
    if (count <= 0) {
        printf("No records to search.\n");
        return;
    }

    if (choice == 1) {
        int id;
        printf("Enter patient ID: ");
        scanf("%d", &id);
        getchar();

        int found = 0;
        // Loop through loaded patients and match ID
        for (int i = 0; i < count; i++) {
            if (patients[i].id == id) {
                Patient *p = &patients[i];
                // Print patient details
                printf("ID: %d\nName: %s\nAge: %d\nDisease: %s\nAdmitted: %s\nBilling: %.2f\n",
                       p->id, p->name, p->age, p->disease, p->admitted_date, p->billing);
                printf("Medicines/Tests: ");
                for (int m = 0; m < p->med_count; m++) {
                    printf("%s%s", p->medicines[m].name, (m == p->med_count - 1) ? "\n" : ", ");
                }
                found = 1;
                break;
            }
        }
        if (!found)
            printf("Patient with ID %d not found.\n", id);
    }
    else if (choice == 2) {
        char diseaseQuery[MAX_NAME_LEN];
        inputString("Enter disease name: ", diseaseQuery, MAX_NAME_LEN);

        int found = 0;
        // Loop through patients and check if disease substring exists in patient's disease field
        for (int i = 0; i < count; i++) {
            if (strstr(patients[i].disease, diseaseQuery) != NULL) {
                Patient *p = &patients[i];
                // Print patient details
                printf("\nID: %d\nName: %s\nAge: %d\nDisease: %s\nAdmitted: %s\nBilling: %.2f\n",
                       p->id, p->name, p->age, p->disease, p->admitted_date, p->billing);
                printf("Medicines/Tests: ");
                for (int m = 0; m < p->med_count; m++) {
                    printf("%s%s", p->medicines[m].name, (m == p->med_count - 1) ? "\n" : ", ");
                }
                found = 1;
            }
        }
        if (!found)
            printf("No patients found with disease containing '%s'.\n", diseaseQuery);
    }
    else {
        printf("Invalid option.\n");
    }

    free(patients);
}

// Function to load all patients from file into a dynamic array, returns count loaded
int loadPatients(Patient **patients) {
    FILE *fp = fopen(FILE_NAME, "rb");  // Open file in read-binary mode
    if (!fp) {
        return 0; // File doesn't exist or can't be opened; no records yet
    }

    fseek(fp, 0, SEEK_END);    // Move to end of file
    long size = ftell(fp);     // Get file size
    rewind(fp);                // Move back to beginning

    int count = size / sizeof(Patient);  // Calculate number of patient records
    *patients = malloc(count * sizeof(Patient));  // Allocate memory dynamically

    if (*patients == NULL) {
        fclose(fp);
        printf("Memory allocation failed.\n");
        return 0;
    }

    fread(*patients, sizeof(Patient), count, fp);  // Read all patient data
    fclose(fp);
    return count;
}

// Function to list all patient records briefly
void listPatients() {
    Patient *patients = NULL;
    int count = loadPatients(&patients);

    if (count <= 0) {
        printf("No patient records.\n");
        return;
    }

    printf("\n--- All Patients ---\n");
    // Loop through patients and print basic info
    for (int i = 0; i < count; i++) {
        Patient *p = &patients[i];
        printf("ID:%d | Name: %s | Disease: %s | Admitted: %s | Billing: %.2f\n",
               p->id, p->name, p->disease, p->admitted_date, p->billing);
    }

    free(patients);  // Free allocated memory
}