# PulseMatrix
A high-performance, deterministic C engine designed for autonomous patient triage and clinical risk stratification. The core architecture utilizes a multi-tiered evaluation pipeline that weighs acute symptom profiles against historical physiological baselines while processing heuristic edge-cases for acute systemic emergencies.

![System Architecture Blueprint](demo1.png)

🛠️ System Architecture & Working Mechanism

The diagnostic engine processes patient information through three independent logical stages:

Symptom-Based Scoring Matrix: Patient symptoms are matched against a predefined diagnostic knowledge base. Each symptom is assigned a base weight, which is dynamically adjusted using logical multipliers based on symptom duration and clinical severity (rated on a scale of 1–5).
Comorbidity-Based Probability Adjustment: The system refines disease likelihood by incorporating physiological indicators and existing medical conditions. For example, fasting blood glucose values ≥126 mg/dL increase the likelihood of uncontrolled diabetes, while abnormal systolic and diastolic blood pressure readings influence hypertension-related assessments.
Critical Red-Flag Detection: A separate safety layer operates independently of the scoring engine to identify medical emergencies requiring immediate attention. This includes conditions such as acute radiating chest pain, thyroid storm, and hyperpyrexia with body temperature ≥104°F (40°C), which override normal scoring and trigger urgent alerts.

## 🖥️ Application Interface Walkthrough

### 1. Patient Vitals Intake
The ingestion layer collects demographic variables to compute foundational metabolic baselines (BMI calculation) and activate age-dependent risk multipliers.

![Patient Vitals Intake](web1.png)

### 2. Clinical Risk Profile & Alerts
The engine parses underlying physiological comorbidities alongside acute indicators to flag compounding clinical vulnerabilities.

![Clinical Risk Profile](web2.png)

### 3. Differential Diagnostic Breakdown
 A prioritized matrix array sorts conditional scores to output percentage-based matching probabilities alongside immediate actionable guidelines.

![Differential Diagnosis](web3.png)

### 4. File-System Logging & Export
The backend runs automated physical file serialization, mapping telemetry inputs to an append-only ledger and generating structured, local diagnostic summaries.

![Report Generation](web4.png)

---

💻 Technical Implementation Details
Memory Architecture: Uses modular, nested data structures (Patient, MedicalHistory, and SymptomEntry) to organize patient records efficiently. Dynamic memory allocation is employed where required to manage runtime data while maintaining a lightweight memory footprint.
Algorithmic Complexity:
Symptom Processing: $\mathcal{O}(N \times M)$, where $N$ represents the number of symptoms entered by the patient and $M$ denotes the total number of symptoms in the diagnostic knowledge base.
Diagnosis Ranking: Candidate diagnoses are sorted using the Bubble Sort algorithm. Although Bubble Sort has a worst-case time complexity of $\mathcal{O}(K^2)$, it is well-suited here because the candidate list is intentionally limited to a maximum of 10 entries, resulting in negligible computational overhead.
Data Layer: The application operates without any external database dependencies. All data handling is performed using standard C libraries such as <string.h>, <stdlib.h>, and <time.h>, ensuring portability across GCC-compatible systems.
⚙️ Compilation & Deployment
Compilation

Compile the source file using the GCC compiler. Link the standard mathematics library (-lm) to support mathematical operations such as BMI calculation and numerical risk-score computations:

gcc main.c -o diagnosis_system -lm
```bash
gcc symptom_checker_v2.c -o pulse_matrix -lm
