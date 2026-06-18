#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

// ─────────────────────────────────────────────
//  CONSTANTS
// ─────────────────────────────────────────────
#define MAX_SYMPTOMS        20
#define MAX_DISEASES        10
#define MAX_SYMPTOM_NAME    50
#define MAX_DISEASE_NAME    50
#define MAX_PATIENT_NAME    50
#define LOG_FILE            "triage_log.txt"
#define SYMPTOM_COUNT       20
#define DISEASE_COUNT       10

// ─────────────────────────────────────────────
//  STRUCTURES
// ─────────────────────────────────────────────

typedef struct {
    char symptom[MAX_SYMPTOM_NAME];
    int  duration_days;
    int  severity;
    int  radiates;
} SymptomEntry;

// Medical history / pre-conditions
typedef struct 
{
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
    char name[MAX_DISEASE_NAME];
    char severity[10];
    char recommendation[200];
    int  symptom_flags[SYMPTOM_COUNT];
    int  weights[SYMPTOM_COUNT];
    int  score;
    int  max_possible_score;
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
    SymptomEntry  *symptoms;
    int            symptom_count;
    char           timestamp[30];
    char           top_diagnosis[MAX_DISEASE_NAME];
    float          top_score;
    int            risk_elevated;
    char           red_flags[8][120];
    int            red_flag_count;
} Patient;


// ─────────────────────────────────────────────
//  LINKED LIST — Symptom History Log
// ─────────────────────────────────────────────
typedef struct SymptomNode {
    char symptom_name[MAX_SYMPTOM_NAME];
    int  severity;
    int  duration_days;
    struct SymptomNode *next;  // pointer to next node
} SymptomNode;

SymptomNode *symptom_log_head = NULL;

void insert_symptom_log(char *name, int sev, int dur) {
    SymptomNode *newNode = (SymptomNode*)malloc(sizeof(SymptomNode));
    strcpy(newNode->symptom_name, name);
    newNode->severity    = sev;
    newNode->duration_days = dur;
    newNode->next        = symptom_log_head;
    symptom_log_head     = newNode;
}

void print_symptom_log() {
    SymptomNode *curr = symptom_log_head;
    printf("\n  Symptom History Log (Linked List):\n");
    while(curr != NULL) {
        printf("  -> %s | sev:%d | dur:%d days\n",
               curr->symptom_name, curr->severity, curr->duration_days);
        curr = curr->next;
    }
}

void free_symptom_log() {
    SymptomNode *curr = symptom_log_head;
    while(curr != NULL) {
        SymptomNode *temp = curr;
        curr = curr->next;
        free(temp);
    }
    symptom_log_head = NULL;
}
// ─────────────────────────────────────────────
//  GLOBAL SYMPTOM BANK
// ─────────────────────────────────────────────
char symptom_bank[SYMPTOM_COUNT][MAX_SYMPTOM_NAME] = {
    "fever",               // 0
    "cough",               // 1
    "fatigue",             // 2
    "headache",            // 3
    "sore throat",         // 4
    "runny nose",          // 5
    "body ache",           // 6
    "chills",              // 7
    "nausea",              // 8     
    "vomiting",            // 9
    "diarrhea",            // 10
    "chest pain",          // 11
    "shortness of breath", // 12
    "loss of smell",       // 13
    "rash",                // 14
    "joint pain",          // 15
    "dizziness",           // 16
    "abdominal pain",      // 17
    "loss of appetite",    // 18
    "sweating"             // 19
};

// ─────────────────────────────────────────────
//  DISEASE DATABASE
// ─────────────────────────────────────────────
void init_diseases(Disease diseases[]) {
    strcpy(diseases[0].name, "Common Cold");
    strcpy(diseases[0].severity, "LOW");
    strcpy(diseases[0].recommendation, "Rest, stay hydrated, take OTC cold medicine. Should resolve in 7-10 days.");
    int f0[]={1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0};
    int w0[]={3,4,3,3,5,5,0,0,0,0,0,0,0,0,0,0,0,0,2,0};
    memcpy(diseases[0].symptom_flags,f0,sizeof(f0)); memcpy(diseases[0].weights,w0,sizeof(w0));

    strcpy(diseases[1].name, "Influenza (Flu)");
    strcpy(diseases[1].severity, "MEDIUM");
    strcpy(diseases[1].recommendation, "Rest, fluids, fever reducers. See a doctor if symptoms worsen after 5 days.");
    int f1[]={1,1,1,1,1,0,1,1,0,0,0,0,0,0,0,0,1,0,1,1};
    int w1[]={5,4,5,4,3,0,5,5,0,0,0,0,0,0,0,0,3,0,3,4};
    memcpy(diseases[1].symptom_flags,f1,sizeof(f1)); memcpy(diseases[1].weights,w1,sizeof(w1));

    strcpy(diseases[2].name, "COVID-19");
    strcpy(diseases[2].severity, "HIGH");
    strcpy(diseases[2].recommendation, "Isolate immediately, get tested. Seek emergency care if breathing difficulty occurs.");
    int f2[]={1,1,1,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,0};
    int w2[]={4,4,5,3,0,0,4,3,0,0,0,0,5,6,0,0,0,0,3,0};
    memcpy(diseases[2].symptom_flags,f2,sizeof(f2)); memcpy(diseases[2].weights,w2,sizeof(w2));

    strcpy(diseases[3].name, "Gastroenteritis");
    strcpy(diseases[3].severity, "MEDIUM");
    strcpy(diseases[3].recommendation, "Stay hydrated, eat bland foods. See a doctor if vomiting persists beyond 2 days.");
    int f3[]={1,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0};
    int w3[]={3,0,3,2,0,0,0,2,5,5,5,0,0,0,0,0,3,5,4,0};
    memcpy(diseases[3].symptom_flags,f3,sizeof(f3)); memcpy(diseases[3].weights,w3,sizeof(w3));

    strcpy(diseases[4].name, "Pneumonia");
    strcpy(diseases[4].severity, "HIGH");
    strcpy(diseases[4].recommendation, "URGENT: See a doctor immediately. Antibiotics and possibly hospitalization needed.");
    int f4[]={1,1,1,1,0,0,1,1,1,0,0,1,1,0,0,0,0,0,1,1};
    int w4[]={5,5,5,3,0,0,4,4,3,0,0,5,6,0,0,0,0,0,3,4};
    memcpy(diseases[4].symptom_flags,f4,sizeof(f4)); memcpy(diseases[4].weights,w4,sizeof(w4));

    strcpy(diseases[5].name, "Dengue Fever");
    strcpy(diseases[5].severity, "HIGH");
    strcpy(diseases[5].recommendation, "URGENT: Get a blood test immediately. Avoid aspirin/ibuprofen. Stay hydrated.");
    int f5[]={1,0,1,1,0,0,1,1,1,1,0,0,0,0,1,1,1,1,1,1};
    int w5[]={6,0,5,5,0,0,6,4,4,3,0,0,0,0,5,6,4,4,4,4};
    memcpy(diseases[5].symptom_flags,f5,sizeof(f5)); memcpy(diseases[5].weights,w5,sizeof(w5));

    strcpy(diseases[6].name, "Migraine");
    strcpy(diseases[6].severity, "LOW");
    strcpy(diseases[6].recommendation, "Rest in a dark quiet room. Take pain relievers. See a neurologist if recurring.");
    int f6[]={0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,0,1,0};
    int w6[]={0,0,3,6,0,0,0,0,4,3,0,0,0,0,0,0,5,0,3,0};
    memcpy(diseases[6].symptom_flags,f6,sizeof(f6)); memcpy(diseases[6].weights,w6,sizeof(w6));

    strcpy(diseases[7].name, "Typhoid Fever");
    strcpy(diseases[7].severity, "HIGH");
    strcpy(diseases[7].recommendation, "URGENT: See a doctor for blood culture test. Requires antibiotic treatment.");
    int f7[]={1,0,1,1,1,0,1,0,1,0,1,0,0,0,1,0,0,1,1,1};
    int w7[]={6,0,5,4,3,0,4,0,4,0,4,0,0,0,4,0,0,5,5,5};
    memcpy(diseases[7].symptom_flags,f7,sizeof(f7)); memcpy(diseases[7].weights,w7,sizeof(w7));

    strcpy(diseases[8].name, "Asthma / Respiratory");
    strcpy(diseases[8].severity, "HIGH");
    strcpy(diseases[8].recommendation, "EMERGENCY if severe: Use inhaler immediately. Seek emergency care if no relief.");
    int f8[]={0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,1,0,0,1};
    int w8[]={0,5,4,0,0,0,0,0,0,0,0,5,6,0,0,0,4,0,0,3};
    memcpy(diseases[8].symptom_flags,f8,sizeof(f8)); memcpy(diseases[8].weights,w8,sizeof(w8));

    strcpy(diseases[9].name, "Food Poisoning");
    strcpy(diseases[9].severity, "MEDIUM");
    strcpy(diseases[9].recommendation, "Hydrate aggressively. Avoid solid food for 6 hours. See doctor if symptoms persist >48hrs.");
    int f9[]={1,0,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1};
    int w9[]={3,0,3,3,0,0,0,0,6,6,5,4,0,0,0,0,4,5,4,3};
    memcpy(diseases[9].symptom_flags,f9,sizeof(f9)); memcpy(diseases[9].weights,w9,sizeof(w9));

    for(int d=0;d<DISEASE_COUNT;d++){
        diseases[d].max_possible_score=0; diseases[d].score=0; diseases[d].match_percent=0.0;
        for(int s=0;s<SYMPTOM_COUNT;s++)
            if(diseases[d].symptom_flags[s]) diseases[d].max_possible_score+=diseases[d].weights[s];
    }
}

// ─────────────────────────────────────────────
//  UTILITY
// ─────────────────────────────────────────────
void to_lowercase(char *str){ for(int i=0;str[i];i++) str[i]=tolower(str[i]); }
void trim(char *str){
    int start=0,end=strlen(str)-1;
    while(str[start]==' ') start++;
    while(end>start&&(str[end]==' '||str[end]=='\n'||str[end]=='\r')) end--;
    int len=end-start+1; memmove(str,str+start,len); str[len]='\0';
}
void print_line(char ch,int len){ for(int i=0;i<len;i++) printf("%c",ch); printf("\n"); }
void get_timestamp(char *buf){ time_t t=time(NULL); struct tm *ti=localtime(&t); strftime(buf,30,"%Y-%m-%d %H:%M:%S",ti); }
float compute_bmi(float w,float h){ float hm=h/100.0; return w/(hm*hm); }
void bmi_category(float bmi,char *cat){
    if(bmi<18.5) strcpy(cat,"Underweight");
    else if(bmi<25.0) strcpy(cat,"Normal");
    else if(bmi<30.0) strcpy(cat,"Overweight");
    else strcpy(cat,"Obese");
}

void print_header(){
    printf("\n");
    print_line('=',62);
    printf("   __  __          _ _           _   _____        _   \n");
    printf("  |  \\/  | ___  __| (_)_ __ ___ | | |_   _| ___ | |_ \n");
    printf("  | |\\/| |/ _ \\/ _` | | '_ ` _ \\| |   | || / _ \\| __|\n");
    printf("  | |  | |  __/ (_| | | | | | | | |   | || | (_) | |_ \n");
    printf("  |_|  |_|\\___|\\__,_|_|_| |_| |_|_|   |_| \\___/ \\__|\n");
    printf("\n");
    printf("            AI-Powered Health Triage Bot  v3.0\n");
    print_line('=',62);
    printf("\n");
}

// ─────────────────────────────────────────────
//  COLLECT MEDICAL HISTORY
// ─────────────────────────────────────────────
void collect_medical_history(MedicalHistory *h) {
    char yn[5];
    int choice;

    print_line('-',62);
    printf("  MEDICAL HISTORY\n");
    print_line('-',62);

    // ── Fever ──
    printf("\n  Do you currently have a fever? (y/n): ");
    fgets(yn,5,stdin); trim(yn); to_lowercase(yn);
    h->has_fever = (yn[0]=='y') ? 1 : 0;
    if(h->has_fever){
        printf("  Fever temperature (in F): ");
        scanf("%f",&h->fever_temp_f); getchar();
        printf("  How many days have you had the fever?: ");
        scanf("%d",&h->fever_duration); getchar();
        if(h->fever_temp_f>=103.0)
            printf("  [!] High fever detected (%.1fF). Flagged as risk factor.\n", h->fever_temp_f);
    } else {
        h->fever_temp_f=0; h->fever_duration=0;
    }

    // ── Diabetic ──
    printf("\n  Are you diabetic? (y/n): ");
    fgets(yn,5,stdin); trim(yn); to_lowercase(yn);
    h->is_diabetic = (yn[0]=='y') ? 1 : 0;
    if(h->is_diabetic){
        printf("  Fasting blood sugar level (mg/dL): ");
        scanf("%f",&h->blood_sugar); getchar();
        if(h->blood_sugar>=126.0)
            printf("  [!] Blood sugar >=126 mg/dL. Uncontrolled diabetes — elevated risk.\n");
        else if(h->blood_sugar>=100.0)
            printf("  [!] Pre-diabetic range (100-125 mg/dL). Moderate risk.\n");
        else
            printf("  Blood sugar in controlled range.\n");
    } else {
        h->blood_sugar=0;
    }

    // ── Thyroid ──
    printf("\n  Do you have a thyroid condition?\n");
    printf("  1. No\n  2. Hypothyroidism (underactive)\n  3. Hyperthyroidism (overactive)\n");
    printf("  Your choice: ");
    scanf("%d",&choice); getchar();
    h->has_thyroid = choice-1;
    if(h->has_thyroid==1)      strcpy(h->thyroid_type,"Hypothyroid");
    else if(h->has_thyroid==2) strcpy(h->thyroid_type,"Hyperthyroid");
    else                       strcpy(h->thyroid_type,"None");

    // ── Blood Pressure ──
    printf("\n  Blood pressure status:\n");
    printf("  1. Normal\n  2. High (Hypertension)\n  3. Low (Hypotension)\n  4. Unknown\n");
    printf("  Your choice: ");
    scanf("%d",&choice); getchar();
    if(choice==1){ h->has_bp=2; strcpy(h->bp_status,"Normal"); }
    else if(choice==2){ h->has_bp=3; strcpy(h->bp_status,"High"); }
    else if(choice==3){ h->has_bp=1; strcpy(h->bp_status,"Low"); }
    else { h->has_bp=0; strcpy(h->bp_status,"Unknown"); }

    if(h->has_bp==3||h->has_bp==1){
        printf("  Enter systolic (top number, e.g. 130): ");
        scanf("%d",&h->bp_systolic); getchar();
        printf("  Enter diastolic (bottom number, e.g. 85): ");
        scanf("%d",&h->bp_diastolic); getchar();
    } else {
        h->bp_systolic=0; h->bp_diastolic=0;
    }

    print_line('-',62);
    printf("  Medical history recorded.\n");
}

// ─────────────────────────────────────────────
//  MEDICAL HISTORY SCORE MODIFIERS
// ─────────────────────────────────────────────
void apply_medical_history_modifiers(Disease diseases[], Patient *p) {
    MedicalHistory *h = &p->history;

    for(int d=0;d<DISEASE_COUNT;d++){

        // ── HIGH FEVER boosts Typhoid, Dengue, Pneumonia, Flu ──
        if(h->has_fever && h->fever_temp_f>=103.0){
            if(strcmp(diseases[d].name,"Typhoid Fever")==0)      diseases[d].score=(int)(diseases[d].score*1.30);
            if(strcmp(diseases[d].name,"Dengue Fever")==0)       diseases[d].score=(int)(diseases[d].score*1.25);
            if(strcmp(diseases[d].name,"Pneumonia")==0)          diseases[d].score=(int)(diseases[d].score*1.20);
            if(strcmp(diseases[d].name,"Influenza (Flu)")==0)    diseases[d].score=(int)(diseases[d].score*1.15);
        }
        // Prolonged fever (5+ days) boosts Typhoid strongly
        if(h->has_fever && h->fever_duration>=5){
            if(strcmp(diseases[d].name,"Typhoid Fever")==0)      diseases[d].score=(int)(diseases[d].score*1.20);
            if(strcmp(diseases[d].name,"Dengue Fever")==0)       diseases[d].score=(int)(diseases[d].score*1.10);
        }

        // ── DIABETES boosts infection-related diseases ──
        if(h->is_diabetic){
            float sugar_mult = 1.0;
            if(h->blood_sugar>=126.0)      sugar_mult=1.25;
            else if(h->blood_sugar>=100.0) sugar_mult=1.10;
            else                           sugar_mult=1.05;

            if(strcmp(diseases[d].name,"Pneumonia")==0)          diseases[d].score=(int)(diseases[d].score*sugar_mult);
            if(strcmp(diseases[d].name,"Typhoid Fever")==0)      diseases[d].score=(int)(diseases[d].score*sugar_mult);
            if(strcmp(diseases[d].name,"Food Poisoning")==0)     diseases[d].score=(int)(diseases[d].score*(sugar_mult*0.9));
            if(strcmp(diseases[d].name,"Gastroenteritis")==0)    diseases[d].score=(int)(diseases[d].score*(sugar_mult*0.9));
            // Diabetics are more vulnerable to respiratory infections
            if(strcmp(diseases[d].name,"Asthma / Respiratory")==0) diseases[d].score=(int)(diseases[d].score*1.10);
        }

        // ── HYPOTHYROID boosts fatigue-heavy diseases ──
        if(h->has_thyroid==1){
            if(strcmp(diseases[d].name,"Influenza (Flu)")==0)    diseases[d].score=(int)(diseases[d].score*1.10);
            if(strcmp(diseases[d].name,"Migraine")==0)           diseases[d].score=(int)(diseases[d].score*1.15);
            // Hypothyroid patients have slower recovery — boost severity
            if(strcmp(diseases[d].severity,"HIGH")==0)           diseases[d].score=(int)(diseases[d].score*1.05);
        }
        // ── HYPERTHYROID boosts cardiac/respiratory concerns ──
        if(h->has_thyroid==2){
            if(strcmp(diseases[d].name,"Asthma / Respiratory")==0) diseases[d].score=(int)(diseases[d].score*1.20);
            if(strcmp(diseases[d].name,"Pneumonia")==0)             diseases[d].score=(int)(diseases[d].score*1.10);
        }

        // ── HIGH BP boosts cardiac/respiratory ──
        if(h->has_bp==3){
            if(strcmp(diseases[d].name,"Asthma / Respiratory")==0) diseases[d].score=(int)(diseases[d].score*1.25);
            if(strcmp(diseases[d].name,"Pneumonia")==0)             diseases[d].score=(int)(diseases[d].score*1.15);
            if(strcmp(diseases[d].name,"Migraine")==0)              diseases[d].score=(int)(diseases[d].score*1.20);
            // Very high BP (systolic>160) is dangerous with chest pain
            if(p->history.bp_systolic>160){
                if(strcmp(diseases[d].name,"Asthma / Respiratory")==0) diseases[d].score=(int)(diseases[d].score*1.10);
            }
        }
        // ── LOW BP boosts dizziness/fatigue conditions ──
        if(h->has_bp==1){
            if(strcmp(diseases[d].name,"Dengue Fever")==0)       diseases[d].score=(int)(diseases[d].score*1.15);
            if(strcmp(diseases[d].name,"Gastroenteritis")==0)    diseases[d].score=(int)(diseases[d].score*1.10);
            if(strcmp(diseases[d].name,"Food Poisoning")==0)     diseases[d].score=(int)(diseases[d].score*1.10);
        }

        // Recalculate match percent
        if(diseases[d].max_possible_score>0)
            diseases[d].match_percent=(float)diseases[d].score/diseases[d].max_possible_score*100.0;
        if(diseases[d].match_percent>99.0) diseases[d].match_percent=99.0;
    }
}

// ─────────────────────────────────────────────
//  RED FLAG DETECTION
// ─────────────────────────────────────────────
void check_red_flags(Patient *p){
    p->red_flag_count=0;
    MedicalHistory *h=&p->history;

    int has_chest=0,has_sob=0,has_fever=0,has_rash=0,has_joint=0,has_vomit=0;
    int chest_rad=0,chest_sev=0,sob_acute=0;

    for(int i=0;i<p->symptom_count;i++){
        char *s=p->symptoms[i].symptom;
        if(strcmp(s,"chest pain")==0){
            has_chest=1;
            if(p->symptoms[i].radiates)    chest_rad=1;
            if(p->symptoms[i].severity>=4) chest_sev=1;
        }
        if(strcmp(s,"shortness of breath")==0){ has_sob=1; if(p->symptoms[i].duration_days==0) sob_acute=1; }
        if(strcmp(s,"fever")==0)       has_fever=1;
        if(strcmp(s,"rash")==0)        has_rash=1;
        if(strcmp(s,"joint pain")==0)  has_joint=1;
        if(strcmp(s,"vomiting")==0)    has_vomit=1;
    }

    if(has_chest&&chest_rad&&chest_sev)
        strcpy(p->red_flags[p->red_flag_count++],"Chest pain radiating to arm/jaw — possible cardiac event");
    if(has_chest&&has_sob&&sob_acute)
        strcpy(p->red_flags[p->red_flag_count++],"Acute chest pain + breathlessness — seek emergency care");
    if(has_fever&&has_rash&&has_joint)
        strcpy(p->red_flags[p->red_flag_count++],"Fever + rash + joint pain — possible Dengue, get blood test");
    if(p->age>=60&&p->risk_elevated)
        strcpy(p->red_flags[p->red_flag_count++],"Age 60+ with elevated BMI — high vulnerability, act fast");
    if(has_vomit&&has_fever)
        strcpy(p->red_flags[p->red_flag_count++],"Fever with vomiting — risk of dehydration, monitor closely");

    // Medical history red flags
    if(h->is_diabetic&&h->blood_sugar>=126.0&&has_fever)
        strcpy(p->red_flags[p->red_flag_count++],"Uncontrolled diabetes + fever — infection risk is critically elevated");
    if(h->has_bp==3&&h->bp_systolic>160&&has_chest)
        strcpy(p->red_flags[p->red_flag_count++],"Severely high BP (>160) + chest pain — hypertensive emergency risk");
    if(h->has_thyroid==2&&has_sob)
        strcpy(p->red_flags[p->red_flag_count++],"Hyperthyroidism + breathing difficulty — thyroid storm risk, see doctor");
    if(h->has_fever&&h->fever_temp_f>=104.0)
        strcpy(p->red_flags[p->red_flag_count++],"Fever >= 104F — dangerously high, seek medical attention immediately");
}

// ─────────────────────────────────────────────
//  FOLLOW-UP QUESTIONS
// ─────────────────────────────────────────────
void ask_followup(SymptomEntry *entry){
    printf("\n  *** FOLLOW-UP: %s ***\n",entry->symptom);
    int choice;
    printf("  How long have you had this?\n  1.<24hrs  2.1-3days  3.4-7days  4.7+days\n  Choice: ");
    scanf("%d",&choice); getchar();
    entry->duration_days=choice-1;
    printf("  Severity (1=mild to 5=severe): ");
    scanf("%d",&entry->severity); getchar();
    if(entry->severity<1) entry->severity=1;
    if(entry->severity>5) entry->severity=5;

    if(strcmp(entry->symptom,"chest pain")==0){
        char yn[5];
        printf("  Does it radiate to arm or jaw? (y/n): ");
        fgets(yn,5,stdin); trim(yn); to_lowercase(yn);
        entry->radiates=(yn[0]=='y')?1:0;
        if(entry->radiates){
            printf("\n"); print_line('!',62);
            printf("  !! CARDIAC WARNING: Radiating chest pain detected     !!\n");
            printf("  !! Consider calling emergency services immediately    !!\n");
            print_line('!',62); printf("\n");
        }
    }
}

// ─────────────────────────────────────────────
//  SCORE ENGINE
// ─────────────────────────────────────────────
void compute_scores(Disease diseases[],Patient *p){
    for(int d=0;d<DISEASE_COUNT;d++) diseases[d].score=0;
    for(int i=0;i<p->symptom_count;i++){
        for(int s=0;s<SYMPTOM_COUNT;s++){
            if(strcmp(p->symptoms[i].symptom,symptom_bank[s])==0){
                for(int d=0;d<DISEASE_COUNT;d++){
                    if(diseases[d].symptom_flags[s]){
                        int w=diseases[d].weights[s];
                        if(p->symptoms[i].duration_days==2) w=(int)(w*1.2);
                        else if(p->symptoms[i].duration_days==3) w=(int)(w*1.4);
                        if(p->symptoms[i].severity>=4) w=(int)(w*1.15);
                        diseases[d].score+=w;
                    }
                }
            }
        }
    }
    for(int d=0;d<DISEASE_COUNT;d++){
        if(p->risk_elevated&&strcmp(diseases[d].severity,"HIGH")==0)
            diseases[d].score=(int)(diseases[d].score*1.1);
        if(diseases[d].max_possible_score>0)
            diseases[d].match_percent=(float)diseases[d].score/diseases[d].max_possible_score*100.0;
        if(diseases[d].match_percent>99.0) diseases[d].match_percent=99.0;
    }
    // Apply medical history modifiers on top
    apply_medical_history_modifiers(diseases,p);
}

void sort_diseases(Disease diseases[]){
    Disease temp;
    for(int i=0;i<DISEASE_COUNT-1;i++)
        for(int j=0;j<DISEASE_COUNT-i-1;j++)
            if(diseases[j].match_percent<diseases[j+1].match_percent){
                temp=diseases[j]; diseases[j]=diseases[j+1]; diseases[j+1]=temp;
            }
}

// ─────────────────────────────────────────────
//  DISPLAY RESULTS
// ─────────────────────────────────────────────
void display_results(Disease diseases[],Patient *p){
    printf("\n"); print_line('=',62);
    printf("  DIAGNOSIS REPORT — %s | Age:%d | %s | BMI:%.1f (%s)\n",
           p->name,p->age,p->gender,p->bmi,p->bmi_category);
    print_line('=',62);

    // Medical history summary
    MedicalHistory *h=&p->history;
    printf("\n  Medical profile:\n");
    printf("  Diabetic  : %s", h->is_diabetic ? "Yes" : "No");
    if(h->is_diabetic) printf(" (blood sugar: %.1f mg/dL)",h->blood_sugar);
    printf("\n");
    printf("  Thyroid   : %s\n",h->thyroid_type);
    printf("  BP status : %s",h->bp_status);
    if(h->bp_systolic>0) printf(" (%d/%d mmHg)",h->bp_systolic,h->bp_diastolic);
    printf("\n");
    if(h->has_fever)
        printf("  Fever     : %.1fF for %d day(s)\n",h->fever_temp_f,h->fever_duration);

    printf("\n  Symptoms entered:\n");
    for(int i=0;i<p->symptom_count;i++){
        char dur[20];
        switch(p->symptoms[i].duration_days){
            case 0:strcpy(dur,"<24hrs");break; case 1:strcpy(dur,"1-3days");break;
            case 2:strcpy(dur,"4-7days");break; default:strcpy(dur,"7+days");break;
        }
        printf("  - %-25s sev:%d/5 | %s",p->symptoms[i].symptom,p->symptoms[i].severity,dur);
        if(p->symptoms[i].radiates) printf(" | RADIATES");
        printf("\n");
    }

    printf("\n"); print_line('-',62);
    printf("  %-28s %-10s %s\n","CONDITION","SEVERITY","MATCH %");
    print_line('-',62);

    int shown=0;
    for(int d=0;d<DISEASE_COUNT&&shown<5;d++){
        if(diseases[d].match_percent>=10.0){
            int bar=(int)(diseases[d].match_percent/5);
            char b[25]="";
            for(int i=0;i<bar&&i<20;i++) strcat(b,"#");
            printf("  %-28s %-10s %5.1f%%\n",diseases[d].name,diseases[d].severity,diseases[d].match_percent);
            printf("  [%-20s]\n",b);
            shown++;
        }
    }
    if(shown==0) printf("  No strong matches found. Please consult a doctor.\n");
    print_line('-',62);

    if(p->red_flag_count>0){
        printf("\n"); print_line('!',62);
        printf("  !! HIGH RISK SIGNALS DETECTED                          !!\n");
        print_line('!',62);
        for(int i=0;i<p->red_flag_count;i++) printf("  >> %s\n",p->red_flags[i]);
        print_line('!',62);
    }

    if(diseases[0].match_percent>=10.0){
        printf("\n  TOP MATCH   : %s (%.1f%%)\n",diseases[0].name,diseases[0].match_percent);
        printf("  SEVERITY    : %s\n",diseases[0].severity);
        printf("\n  RECOMMENDATION:\n  %s\n",diseases[0].recommendation);
    }
    print_line('=',62);

    strcpy(p->top_diagnosis,diseases[0].name);
    p->top_score=diseases[0].match_percent;
}

// ─────────────────────────────────────────────
//  REPORT EXPORT
// ─────────────────────────────────────────────
void export_report(Disease diseases[],Patient *p){
    char filename[80],safename[MAX_PATIENT_NAME];
    strcpy(safename,p->name);
    for(int i=0;safename[i];i++) if(safename[i]==' ') safename[i]='_';
    sprintf(filename,"%s_report.txt",safename);

    FILE *fp=fopen(filename,"w");
    if(!fp){ printf("  [!] Could not create report.\n"); return; }

    fprintf(fp,"=============================================\n");
    fprintf(fp,"        MEDITOT HEALTH TRIAGE REPORT\n");
    fprintf(fp,"=============================================\n");
    fprintf(fp,"Date & Time  : %s\n",p->timestamp);
    fprintf(fp,"Patient Name : %s\n",p->name);
    fprintf(fp,"Age / Gender : %d / %s\n",p->age,p->gender);
    fprintf(fp,"BMI          : %.1f (%s)\n",p->bmi,p->bmi_category);
    fprintf(fp,"Risk Profile : %s\n",p->risk_elevated?"ELEVATED":"NORMAL");
    fprintf(fp,"---------------------------------------------\n");
    fprintf(fp,"MEDICAL HISTORY:\n");
    MedicalHistory *h=&p->history;
    fprintf(fp,"  Diabetic  : %s",h->is_diabetic?"Yes":"No");
    if(h->is_diabetic) fprintf(fp," (%.1f mg/dL)",h->blood_sugar);
    fprintf(fp,"\n");
    fprintf(fp,"  Thyroid   : %s\n",h->thyroid_type);
    fprintf(fp,"  BP Status : %s",h->bp_status);
    if(h->bp_systolic>0) fprintf(fp," (%d/%d mmHg)",h->bp_systolic,h->bp_diastolic);
    fprintf(fp,"\n");
    if(h->has_fever) fprintf(fp,"  Fever     : %.1fF for %d day(s)\n",h->fever_temp_f,h->fever_duration);
    fprintf(fp,"---------------------------------------------\n");
    fprintf(fp,"SYMPTOMS REPORTED:\n");
    for(int i=0;i<p->symptom_count;i++){
        char dur[20];
        switch(p->symptoms[i].duration_days){
            case 0:strcpy(dur,"<24hrs");break; case 1:strcpy(dur,"1-3days");break;
            case 2:strcpy(dur,"4-7days");break; default:strcpy(dur,"7+days");break;
        }
        fprintf(fp,"  - %-25s sev:%d/5 | %s",p->symptoms[i].symptom,p->symptoms[i].severity,dur);
        if(p->symptoms[i].radiates) fprintf(fp," | RADIATES");
        fprintf(fp,"\n");
    }
    fprintf(fp,"---------------------------------------------\n");
    fprintf(fp,"DIAGNOSIS RESULTS:\n");
    int shown=0;
    for(int d=0;d<DISEASE_COUNT&&shown<3;d++){
        if(diseases[d].match_percent>=10.0){
            fprintf(fp,"  %d. %-24s — %-8s — %.1f%%\n",shown+1,diseases[d].name,diseases[d].severity,diseases[d].match_percent);
            shown++;
        }
    }
    fprintf(fp,"---------------------------------------------\n");
    fprintf(fp,"TOP DIAGNOSIS  : %s\n",p->top_diagnosis);
    fprintf(fp,"RECOMMENDATION : %s\n",diseases[0].recommendation);
    if(p->red_flag_count>0){
        fprintf(fp,"---------------------------------------------\n");
        fprintf(fp,"!! HIGH RISK SIGNALS DETECTED:\n");
        for(int i=0;i<p->red_flag_count;i++) fprintf(fp,"  >> %s\n",p->red_flags[i]);
    }
    fprintf(fp,"=============================================\n");
    fclose(fp);
    printf("\n  [Report exported to %s]\n",filename);
}

// ─────────────────────────────────────────────
//  FILE LOGGING
// ─────────────────────────────────────────────
void log_session(Patient *p){
    FILE *fp=fopen(LOG_FILE,"a");
    if(!fp){ printf("  [!] Could not open log.\n"); return; }
    fprintf(fp,"TIMESTAMP|%s|NAME|%s|AGE|%d|GENDER|%s|BMI|%.1f|DIAGNOSIS|%s|SCORE|%.1f\n",
            p->timestamp,p->name,p->age,p->gender,p->bmi,p->top_diagnosis,p->top_score);
    fclose(fp);
    printf("  [Session saved to %s]\n",LOG_FILE);
}

// ─────────────────────────────────────────────
//  VIEW LOGS
// ─────────────────────────────────────────────
void view_logs(){
    FILE *fp=fopen(LOG_FILE,"r");
    if(!fp){ printf("\n  No session logs found yet.\n"); return; }
    printf("\n"); print_line('=',62); printf("  PAST SESSION LOGS\n"); print_line('=',62);
    char line[400]; int count=0;
    while(fgets(line,sizeof(line),fp)){
        char ts[30],name[50],gender[10],diag[50]; int age; float bmi,score;
        sscanf(line,"TIMESTAMP|%29[^|]|NAME|%49[^|]|AGE|%d|GENDER|%9[^|]|BMI|%f|DIAGNOSIS|%49[^|]|SCORE|%f",
               ts,name,&age,gender,&bmi,diag,&score);
        printf("  [%d] %s\n      %s | Age:%d | %s | BMI:%.1f\n      Diagnosis: %s (%.1f%%)\n\n",
               ++count,ts,name,age,gender,bmi,diag,score);
    }
    if(count==0) printf("  No records found.\n");
    print_line('-',62); printf("  Total records: %d\n",count);
    fclose(fp);
}

// ─────────────────────────────────────────────
//  SEARCH PATIENT
// ─────────────────────────────────────────────
void search_patient(){
    FILE *fp=fopen(LOG_FILE,"r");
    if(!fp){ printf("\n  No logs found yet.\n"); return; }
    char search[MAX_PATIENT_NAME];
    printf("\n  Enter patient name to search: ");
    fgets(search,MAX_PATIENT_NAME,stdin); trim(search); to_lowercase(search);
    printf("\n"); print_line('-',62);
    printf("  Results for \"%s\":\n",search); print_line('-',62);
    char line[400],last_ts[30]="",last_diag[50]=""; int found=0;
    while(fgets(line,sizeof(line),fp)){
        char ts[30],name[50],gender[10],diag[50]; int age; float bmi,score;
        sscanf(line,"TIMESTAMP|%29[^|]|NAME|%49[^|]|AGE|%d|GENDER|%9[^|]|BMI|%f|DIAGNOSIS|%49[^|]|SCORE|%f",
               ts,name,&age,gender,&bmi,diag,&score);
        char nl[50]; strcpy(nl,name); to_lowercase(nl);
        if(strstr(nl,search)){
            printf("  [%d] %s — %s (%.1f%%)\n",++found,ts,diag,score);
            strcpy(last_ts,ts); strcpy(last_diag,diag);
        }
    }
    fclose(fp);
    if(found==0) printf("  No records found for \"%s\".\n",search);
    else{
        print_line('-',62);
        printf("  %d record(s) found.\n",found);
        printf("\n  Welcome back %s! Last visited: %s\n",search,last_ts);
        printf("  Previous diagnosis: %s. Are symptoms persisting?\n",last_diag);
    }
}

// ─────────────────────────────────────────────
//  SYMPTOM HELPERS
// ─────────────────────────────────────────────
void show_symptom_list(){
    printf("\n  Available symptoms:\n"); print_line('-',62);
    for(int i=0;i<SYMPTOM_COUNT;i++){
        printf("  %2d. %-25s",i+1,symptom_bank[i]);
        if((i+1)%2==0) printf("\n");
    }
    if(SYMPTOM_COUNT%2!=0) printf("\n");
    print_line('-',62);
}
int is_valid_symptom(char *s){ for(int i=0;i<SYMPTOM_COUNT;i++) if(strcmp(s,symptom_bank[i])==0) return 1; return 0; }
int is_duplicate(SymptomEntry *e,int c,char *s){ for(int i=0;i<c;i++) if(strcmp(e[i].symptom,s)==0) return 1; return 0; }

// ─────────────────────────────────────────────
//  COLLECT PATIENT INFO
// ─────────────────────────────────────────────
void collect_patient_info(Patient *p){
    printf("\n  Enter patient name   : "); fgets(p->name,MAX_PATIENT_NAME,stdin); trim(p->name);
    printf("  Enter age            : "); scanf("%d",&p->age); getchar();
    printf("  Enter weight (kg)    : "); scanf("%f",&p->weight_kg); getchar();
    printf("  Enter height (cm)    : "); scanf("%f",&p->height_cm); getchar();
    printf("  Enter gender (M/F/O) : "); fgets(p->gender,10,stdin); trim(p->gender);

    p->bmi=compute_bmi(p->weight_kg,p->height_cm);
    bmi_category(p->bmi,p->bmi_category);
    get_timestamp(p->timestamp);
    p->symptom_count=0; p->red_flag_count=0;
    p->risk_elevated=(p->age>=60||p->bmi>=30.0)?1:0;

    printf("\n  BMI: %.1f — %s.\n",p->bmi,p->bmi_category);
    if(p->risk_elevated) printf("  [!] Elevated risk profile (Age/BMI). Multiplier active.\n");

    // Collect medical history
    collect_medical_history(&p->history);

    p->symptoms=(SymptomEntry*)malloc(MAX_SYMPTOMS*sizeof(SymptomEntry));
    if(!p->symptoms){ printf("  [!] Memory allocation failed.\n"); exit(1); }
}

// ─────────────────────────────────────────────
//  COLLECT SYMPTOMS
// ─────────────────────────────────────────────
void collect_symptoms(Patient *p){
    show_symptom_list();
    printf("\n  Enter symptoms one by one. Type 'done' when finished.\n\n");
    char input[MAX_SYMPTOM_NAME];
    while(p->symptom_count<MAX_SYMPTOMS){
        printf("  Symptom %d: ",p->symptom_count+1);
        fgets(input,MAX_SYMPTOM_NAME,stdin); trim(input); to_lowercase(input);
        if(strcmp(input,"done")==0){
            if(p->symptom_count==0){ printf("  [!] Enter at least one symptom.\n"); continue; }
            break;
        }
        if(is_duplicate(p->symptoms,p->symptom_count,input)){ printf("  [!] Already entered.\n"); continue; }
        if(!is_valid_symptom(input)){ printf("  [!] Not recognized. Pick from the list.\n"); continue; }
        strcpy(p->symptoms[p->symptom_count].symptom,input);
        p->symptoms[p->symptom_count].duration_days=0;
        p->symptoms[p->symptom_count].severity=1;
        p->symptoms[p->symptom_count].radiates=0;
        ask_followup(&p->symptoms[p->symptom_count]);
        p->symptom_count++;
        printf("  [+] Added: %s\n\n",input);
    }
}

// ─────────────────────────────────────────────
//  MAIN MENU
// ─────────────────────────────────────────────
void main_menu(){
    int choice;
    Disease diseases[DISEASE_COUNT];
    init_diseases(diseases);

    while(1){
        printf("\n"); print_line('-',62);
        printf("  MAIN MENU\n"); print_line('-',62);
        printf("  1. Start New Diagnosis\n");
        printf("  2. View Past Session Logs\n");
        printf("  3. Search Patient by Name\n");
        printf("  4. View Symptom List\n");
        printf("  5. Exit\n");
        print_line('-',62);
        printf("  Your choice: ");
        scanf("%d",&choice); getchar();

        switch(choice){
            case 1:{
                Patient patient;
                collect_patient_info(&patient);
                collect_symptoms(&patient);
                printf("\n  Analyzing symptoms");
                for(int i=0;i<5;i++){ printf("."); fflush(stdout); for(volatile long j=0;j<50000000L;j++); }
                printf(" Done!\n");
                init_diseases(diseases);
                compute_scores(diseases,&patient);
                sort_diseases(diseases);
                check_red_flags(&patient);
                display_results(diseases,&patient);
                export_report(diseases,&patient);
                log_session(&patient);
                free(patient.symptoms);
                break;
            }
            case 2: view_logs();         break;
            case 3: search_patient();    break;
            case 4: show_symptom_list(); break;
            case 5: printf("\n  Thank you for using MediTot. Stay healthy!\n\n"); exit(0);
            default: printf("  [!] Invalid choice. Enter 1-5.\n");
        }
    }
}

// ─────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────
int main(){
    print_header();
    main_menu();
    return 0;
}