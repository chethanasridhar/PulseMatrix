#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ─────────────────────────────────────────────────────────────────────────────
//  1. CONFIGURATION CONSTANTS & ARCHITECTURE BLUEPRINTS
// ─────────────────────────────────────────────────────────────────────────────
#define MAX_SYMPTOMS         20
#define MAX_DISEASES         10
#define MAX_SYMPTOM_NAME     50
#define MAX_DISEASE_NAME     50
#define MAX_PATIENT_NAME     50
#define LOG_FILE             "triage_log.txt"
#define SYMPTOM_COUNT        20
#define DISEASE_COUNT        10

typedef struct {
    char symptom[MAX_SYMPTOM_NAME];
    int  duration_days;
    int  severity;
    int  radiates;
} SymptomEntry;

typedef struct {
    int   is_diabetic;        
    float blood_sugar;        
    int   has_thyroid;        
    char  thyroid_type[20];   
    int   has_bp;             
    char  bp_status[20];     
    int   bp_systolic;        
    int   bp_diastolic;       
    int   has_fever;          
    float fever_temp_f;       
    int   fever_duration;     
} MedicalHistory;

typedef struct {
    char  name[MAX_DISEASE_NAME];
    char  severity[10];
    char  recommendation[200];
    int   symptom_flags[SYMPTOM_COUNT];
    int   weights[SYMPTOM_COUNT];
    int   score;
    int   max_possible_score;
    float match_percent;
} Disease;

typedef struct {
    char           name[MAX_PATIENT_NAME];
    int            age;
    float          weight_kg;
    float          height_cm;
    float          bmi;
    char           bmi_category[20];
    char           gender[10];
    MedicalHistory history;
    SymptomEntry   *symptoms;
    int            symptom_count;
    char           timestamp[30];
    char           top_diagnosis[MAX_DISEASE_NAME];
    float          top_score;
    int            risk_elevated;
    char           red_flags[8][120];
    int            red_flag_count;
} Patient;

// Global symptom definitions layer
const char *symptom_bank[SYMPTOM_COUNT] = {
    "fatigue", "cough", "fever", "shortness of breath", "chest pain",
    "headache", "sore throat", "body aches", "loss of taste", "diarrhea",
    "joint pain", "rash", "weight loss", "anxiety", "palpitations",
    "sweating", "nausea", "wheezing", "dizziness", "blurred vision"
};

// ─────────────────────────────────────────────────────────────────────────────
//  2. COMPUTATIONAL CORE ENGINE SUB-SYSTEM
// ─────────────────────────────────────────────────────────────────────────────

void calculate_bmi(Patient *p) {
    if (p->height_cm > 0) {
        float height_m = p->height_cm / 100.0f;
        p->bmi = p->weight_kg / (height_m * height_m);
    } else {
        p->bmi = 0.0f;
    }

    if (p->bmi < 18.5f) strcpy(p->bmi_category, "Underweight");
    else if (p->bmi < 25.0f) strcpy(p->bmi_category, "Normal");
    else if (p->bmi < 30.0f) strcpy(p->bmi_category, "Overweight");
    else strcpy(p->bmi_category, "Obese");
}

void evaluate_red_flags(Patient *p) {
    p->red_flag_count = 0;
    p->risk_elevated = 0;

    // Rule 1: Vulnerable age profile matched with elevated BMI thresholds
    if (p->age >= 60 && p->bmi >= 30.0f) {
        strcpy(p->red_flags[p->red_flag_count++], "Age 60+ with elevated BMI - high vulnerability profile, act quickly");
        p->risk_elevated = 1;
    }

    // Rule 2: Hypertensive Crisis interception
    if (p->history.bp_systolic >= 180 || p->history.bp_diastolic >= 120) {
        strcpy(p->red_flags[p->red_flag_count++], "CRITICAL: Hypertensive Crisis thresholds detected! Immediate monitoring needed.");
        p->risk_elevated = 1;
    }

    // Rule 3: Uncontrolled Diabetes metabolic exception
    if (p->history.is_diabetic && p->history.blood_sugar >= 250.0f) {
        strcpy(p->red_flags[p->red_flag_count++], "ALERT: Critical Hyperglycemia detected. Risk of DKA.");
        p->risk_elevated = 1;
    }

    // Loop to match dangerous symptom presentation pairs
    for (int i = 0; i < p->symptom_count; i++) {
        // Rule 4: Cardiac Red Flag
        if (strcmp(p->symptoms[i].symptom, "chest pain") == 0) {
            if (p->symptoms[i].severity >= 4 || p->symptoms[i].radiates) {
                strcpy(p->red_flags[p->red_flag_count++], "EMERGENCY: High severity or radiating chest pain - Potential Acute Cardiac Event.");
                p->risk_elevated = 1;
            }
        }
        // Rule 5: Hyperthyroid presentation mixed with respiratory strain
        if (strcmp(p->symptoms[i].symptom, "shortness of breath") == 0 && 
            strcmp(p->history.thyroid_type, "Hyperthyroid") == 0) {
            strcpy(p->red_flags[p->red_flag_count++], "Hyperthyroidism + breathing difficulty - thyroid storm risk, see doctor");
            p->risk_elevated = 1;
        }
    }
}

// Standard Library comparison implementation for qsort() execution
int compare_diseases(const void *a, const void *b) {
    float percent_a = ((Disease *)a)->match_percent;
    float percent_b = ((Disease *)b)->match_percent;
    return (percent_b > percent_a) - (percent_b < percent_a);
}

void run_triage_matrix(Patient *p, Disease diseases[]) {
    for (int d = 0; d < DISEASE_COUNT; d++) {
        diseases[d].score = 0;
        diseases[d].max_possible_score = 0;

        // Calculate maximum potential denominator base score
        for (int s = 0; s < SYMPTOM_COUNT; s++) {
            if (diseases[d].symptom_flags[s]) {
                diseases[d].max_possible_score += diseases[d].weights[s] * 5; 
            }
        }

        // Apply interactive patient parameter weights
        for (int i = 0; i < p->symptom_count; i++) {
            for (int s = 0; s < SYMPTOM_COUNT; s++) {
                if (strcmp(p->symptoms[i].symptom, symptom_bank[s]) == 0) {
                    if (diseases[d].symptom_flags[s]) {
                        int base_weight = diseases[d].weights[s];
                        int calculated_score = base_weight * p->symptoms[i].severity;

                        if (p->symptoms[i].duration_days >= 7) {
                            calculated_score += 2;
                        }
                        diseases[d].score += calculated_score;
                    }
                }
            }
        }

        // Apply dynamic comorbidity modifiers directly to curves
        if (p->history.is_diabetic && strcmp(diseases[d].name, "COVID-19") == 0) {
            diseases[d].score += 5;
        }
        if (strcmp(p->history.bp_status, "High") == 0 && strcmp(diseases[d].name, "Asthma / Respiratory") == 0) {
            diseases[d].score += 4;
        }

        // Guard against zero divisors and scale percentages cleanly
        if (diseases[d].max_possible_score > 0) {
            diseases[d].match_percent = ((float)diseases[d].score / diseases[d].max_possible_score) * 100.0f;
            if (diseases[d].match_percent > 100.0f) diseases[d].match_percent = 100.0f;
        } else {
            diseases[d].match_percent = 0.0f;
        }
    }

    // Call optimized standard library qsort instead of slow O(N^2) loops
    qsort(diseases, DISEASE_COUNT, sizeof(Disease), compare_diseases);

    strcpy(p->top_diagnosis, diseases[0].name);
    p->top_score = diseases[0].match_percent;
}

void initialize_disease_matrix(Disease diseases[]) {
    // 1. COVID-19 Initialization
    strcpy(diseases[0].name, "COVID-19");
    strcpy(diseases[0].severity, "HIGH");
    strcpy(diseases[0].recommendation, "Isolate immediately, get tested. Seek emergency care if breathing difficulty occurs.");
    int f0[20] = {1,1,1,1,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0};
    int w0[20] = {2,3,3,4,0,2,2,2,3,2,0,0,0,0,0,0,0,0,0,0};
    memcpy(diseases[0].symptom_flags, f0, sizeof(f0));
    memcpy(diseases[0].weights, w0, sizeof(w0));

    // 2. Influenza (Flu)
    strcpy(diseases[1].name, "Influenza (Flu)");
    strcpy(diseases[1].severity, "MEDIUM");
    strcpy(diseases[1].recommendation, "Rest, stay hydrated, and take antipyretics. Consult a doctor if fever persists past 5 days.");
    int f1[20] = {1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0};
    int w1[20] = {3,2,3,0,0,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0};
    memcpy(diseases[1].symptom_flags, f1, sizeof(f1));
    memcpy(diseases[1].weights, w1, sizeof(w1));

    // 3. Asthma / Respiratory
    strcpy(diseases[2].name, "Asthma / Respiratory");
    strcpy(diseases[2].severity, "HIGH");
    strcpy(diseases[2].recommendation, "EMERGENCY if severe: Use inhaler immediately. Seek emergency care if no relief.");
    int f2[20] = {1,1,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0};
    int w2[20] = {2,3,0,5,0,0,0,0,0,0,0,0,0,2,0,0,0,4,0,0};
    memcpy(diseases[2].symptom_flags, f2, sizeof(f2));
    memcpy(diseases[2].weights, w2, sizeof(w2));

    // 4. Dengue Fever
    strcpy(diseases[3].name, "Dengue Fever");
    strcpy(diseases[3].severity, "HIGH");
    strcpy(diseases[3].recommendation, "Avoid NSAIDs (like Ibuprofen). Take Paracetamol and check platelet count immediately.");
    int f3[20] = {1,0,1,0,0,3,0,1,0,0,1,1,0,0,0,0,1,0,1,0};
    int w3[20] = {3,0,4,0,0,3,0,3,0,0,4,3,0,0,0,0,2,0,2,0};
    memcpy(diseases[3].symptom_flags, f3, sizeof(f3));
    memcpy(diseases[3].weights, w3, sizeof(w3));

    // Pad structural array boundaries safely for items 5-10
    for (int i = 4; i < DISEASE_COUNT; i++) {
        sprintf(diseases[i].name, "Condition Profile %d", i + 1);
        strcpy(diseases[i].severity, "LOW");
        strcpy(diseases[i].recommendation, "Monitor condition changes and follow standard baseline observation guidelines.");
        memset(diseases[i].symptom_flags, 0, sizeof(diseases[i].symptom_flags));
        memset(diseases[i].weights, 0, sizeof(diseases[i].weights));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  3. BOUNDARY INTERFACE & REPORT GENERATION LAYER
// ─────────────────────────────────────────────────────────────────────────────

void generate_report_file(Patient *p, Disease diseases[]) {
    char filename[100];
    snprintf(filename, sizeof(filename), "%s_report.txt", p->name);
    FILE *f = fopen(filename, "w");
    if (!f) return;

    fprintf(f, "===============================================\n");
    fprintf(f, "          MEDITOT HEALTH TRIAGE REPORT         \n");
    fprintf(f, "===============================================\n");
    fprintf(f, "Date & Time   : %s\n", p->timestamp);
    fprintf(f, "Patient Name  : %s\n", p->name);
    fprintf(f, "Age / Gender  : %d / %s\n", p->age, p->gender);
    fprintf(f, "BMI           : %.1f (%s)\n", p->bmi, p->bmi_category);
    fprintf(f, "Risk Profile  : %s\n", p->risk_elevated ? "ELEVATED" : "STANDARD");
    fprintf(f, "-----------------------------------------------\n");
    fprintf(f, "DIAGNOSIS RESULTS:\n");
    
    for (int i = 0; i < 3; i++) {
        fprintf(f, "  %d. %-22s - %-6s - %.1f%%\n", i+1, diseases[i].name, diseases[i].severity, diseases[i].match_percent);
    }
    
    fprintf(f, "-----------------------------------------------\n");
    fprintf(f, "TOP DIAGNOSIS  : %s\n", p->top_diagnosis);
    fprintf(f, "RECOMMENDATION : %s\n", diseases[0].recommendation);
    fprintf(f, "===============================================\n");
    fclose(f);
}

int main() {
    Patient patient;
    Disease diseases[DISEASE_COUNT];

    // Safe bounds intake parsing data
    strncpy(patient.name, "Amanda", MAX_PATIENT_NAME);
    patient.age = 98;
    patient.weight_kg = 82.5f;
    patient.height_cm = 160.0f;
    strncpy(patient.gender, "Female", 10);

    patient.history.is_diabetic = 0;
    patient.history.blood_sugar = 95.0f;
    patient.history.has_thyroid = 1;
    strncpy(patient.history.thyroid_type, "Hyperthyroid", 20);
    patient.history.has_bp = 1;
    strncpy(patient.history.bp_status, "High", 20);
    patient.history.bp_systolic = 145;
    patient.history.bp_diastolic = 87;

    // Secure dynamic heap allocations for arrays
    patient.symptoms = malloc(sizeof(SymptomEntry) * 2);
    patient.symptom_count = 2;

    strncpy(patient.symptoms[0].symptom, "fatigue", MAX_SYMPTOM_NAME);
    patient.symptoms[0].severity = 4;
    patient.symptoms[0].duration_days = 8;
    patient.symptoms[0].radiates = 0;

    strncpy(patient.symptoms[1].symptom, "shortness of breath", MAX_SYMPTOM_NAME);
    patient.symptoms[1].severity = 3;
    patient.symptoms[1].duration_days = 3;
    patient.symptoms[1].radiates = 0;

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(patient.timestamp, sizeof(patient.timestamp), "%d/%m/%Y, %H:%M:%S", tm_info);

    // Operational runtime execution paths
    calculate_bmi(&patient);
    initialize_disease_matrix(diseases);
    evaluate_red_flags(&patient);
    run_triage_matrix(&patient, diseases);

    printf("Triage Execution Complete for Patient: %s\n", patient.name);
    printf("Primary Triage Classification: %s (Probability Curve Match: %.2f%%)\n", patient.top_diagnosis, patient.top_score);

    generate_report_file(&patient, diseases);

    // Garbage collection rules
    free(patient.symptoms);
    return 0;
}