// ==========================================================
//OEL  - Hospital Appointment and Patient Management System
// Data structures:
//   1. Singly Linked List : patient records
//   2. Array of structs   : appointment records
//   3. Linked Queue (FIFO): scheduled appointments, in arrival order

// Algorithms:
//   1. Linear Search      : patient by ID and partial name
//   2. Binary Search      : appointment by ID (array kept sorted by ID)
//   3. Bubble Sort        : patients by name (sorts a temporary copy)
//   4. Selection Sort     : appointments by date/time (temporary copy)

// Files:
//   patients.txt
//   appointments.txt
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cctype>
#include <cstdlib>

using namespace std;

const int MAX_APPT = 100;
const string PATIENT_FILE = "patients.txt";
const string APPT_FILE = "appointments.txt";

// -------------------- Data structures --------------------

struct Patient {
    int id;
    string name;
    int age;
    string gender;
    string disease;
    Patient* next;
};

struct Appointment {
    int id;
    int patientId;
    string doctor;
    string date;       // YYYY-MM-DD
    string time;       // HH:MM
    string status;     // Scheduled / Completed / Cancelled
};

struct QNode {
    int apptId;
    QNode* next;
};

Patient* head = NULL;
Appointment appts[MAX_APPT];
int apptCount = 0;
int nextApptId = 1;
QNode* qFront = NULL;
QNode* qRear = NULL;

// -------------------- Function declarations --------------------

void saveAll();
void loadAll();
void clearMemory();

// -------------------- Helper functions --------------------

string trim(const string& s) {
    size_t first = 0;
    size_t last = s.size();

    while (first < last && isspace(static_cast<unsigned char>(s[first])))
        first++;
    while (last > first && isspace(static_cast<unsigned char>(s[last - 1])))
        last--;

    return s.substr(first, last - first);
}

string toLower(string s) {
    for (size_t i = 0; i < s.size(); i++)
        s[i] = static_cast<char>(tolower(static_cast<unsigned char>(s[i])));
    return s;
}


bool validText(const string& s) {
    string t = trim(s);

    if (t.empty()) return false;

    for (char c : t) {
        if (!isalpha(static_cast<unsigned char>(c)) && c != ' ')
            return false;
    }

    return true;
}


string readLine(const string& prompt) {
    cout << prompt;
    string value;

    if (!getline(cin, value)) {
        cout << "\nInput ended. Saving data and exiting.\n";
        saveAll();
        clearMemory();
        exit(0);
    }

    return trim(value);
}

// Returns -1 if the input is not a valid non-negative integer.
int readInt(const string& prompt) {
    string value = readLine(prompt);

    if (value.empty() || value.size() > 9)
        return -1;

    for (size_t i = 0; i < value.size(); i++) {
        if (!isdigit(static_cast<unsigned char>(value[i])))
            return -1;
    }

    return atoi(value.c_str());
}

bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

bool validDate(const string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-')
        return false;

    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 &&
            !isdigit(static_cast<unsigned char>(date[i])))
            return false;
    }

    int year = atoi(date.substr(0, 4).c_str());
    int month = atoi(date.substr(5, 2).c_str());
    int day = atoi(date.substr(8, 2).c_str());

    if (year < 2000 || year > 2100 || month < 1 || month > 12)
        return false;

    const int daysInMonth[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    int maxDay = daysInMonth[month - 1];
    if (month == 2 && isLeapYear(year))
        maxDay = 29;

    return day >= 1 && day <= maxDay;
}

// Hospital appointment hours: 08:00 through 19:59.
bool validTime(const string& time) {
    if (time.size() != 5 || time[2] != ':')
        return false;

    for (int i = 0; i < 5; i++) {
        if (i != 2 && !isdigit(static_cast<unsigned char>(time[i])))
            return false;
    }

    int hour = atoi(time.substr(0, 2).c_str());
    int minute = atoi(time.substr(3, 2).c_str());

    return hour >= 8 && hour <= 19 && minute >= 0 && minute <= 59;
}

bool validGender(const string& gender) {
    string g = toLower(gender);
    return g == "m" || g == "f";
}

bool validStatus(const string& status) {
    return status == "Scheduled" ||
           status == "Completed" ||
           status == "Cancelled";
}

// -------------------- Patient linked-list module --------------------

// Linear search by patient ID.
Patient* findPatient(int id) {
    Patient* current = head;

    while (current != NULL) {
        if (current->id == id)
            return current;
        current = current->next;
    }

    return NULL;
}

int patientCount() {
    int count = 0;
    Patient* current = head;

    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

void printPatientHeader() {
    cout << left << setw(8) << "ID"
         << setw(22) << "Name"
         << setw(6) << "Age"
         << setw(8) << "Gender"
         << setw(20) << "Disease" << '\n';
    cout << string(64, '-') << '\n';
}

void printPatient(const Patient& patient) {
    cout << left << setw(8) << patient.id
         << setw(22) << patient.name
         << setw(6) << patient.age
         << setw(8) << patient.gender
         << setw(20) << patient.disease << '\n';
}

void registerPatient() {
    int id = readInt("Enter Patient ID (1-999999): ");

    if (id < 1 || id > 999999) {
        cout << "Error: Invalid patient ID.\n";
        return;
    }

    if (findPatient(id) != NULL) {
        cout << "Error: Patient ID already exists.\n";
        return;
    }

    string name = readLine("Enter Name: ");
    if (!validText(name)) {
        cout << "Error: Invalid name. The name cannot be empty or contain '|'.\n";
        return;
    }

    int age = readInt("Enter Age (0-120): ");
    if (age < 0 || age > 120) {
        cout << "Error: Invalid age.\n";
        return;
    }

    string gender = toLower(readLine("Enter Gender (M/F): "));
    if (!validGender(gender)) {
        cout << "Error: Gender must be M or F.\n";
        return;
    }

    string disease = readLine("Enter Disease/Problem: ");
    if (!validText(disease)) {
        cout << "Error: Invalid disease/problem.\n";
        return;
    }

    Patient* patient = new Patient;
    patient->id = id;
    patient->name = name;
    patient->age = age;
    patient->gender = (gender == "m") ? "M" : "F";
    patient->disease = disease;
    patient->next = NULL;

    // Insert at the end of the singly linked list.
    if (head == NULL) {
        head = patient;
    } else {
        Patient* current = head;
        while (current->next != NULL)
            current = current->next;
        current->next = patient;
    }

    saveAll();
    cout << "Patient registered successfully.\n";
}

void updatePatient() {
    int id = readInt("Enter Patient ID to update: ");
    Patient* patient = findPatient(id);

    if (patient == NULL) {
        cout << "Error: Patient not found.\n";
        return;
    }

    cout << "\nCurrent patient record:\n";
    printPatientHeader();
    printPatient(*patient);
    cout << "Press Enter to keep an existing value.\n";

    string name = readLine("New Name: ");
    if (!name.empty() && !validText(name)) {
        cout << "Error: Invalid name. Nothing was updated.\n";
        return;
    }

    string ageText = readLine("New Age: ");
    int age = patient->age;

    if (!ageText.empty()) {
        bool digitsOnly = ageText.size() <= 3;

        for (size_t i = 0; i < ageText.size(); i++) {
            if (!isdigit(static_cast<unsigned char>(ageText[i])))
                digitsOnly = false;
        }

        if (!digitsOnly) {
            cout << "Error: Invalid age. Nothing was updated.\n";
            return;
        }

        age = atoi(ageText.c_str());
        if (age < 0 || age > 120) {
            cout << "Error: Age must be between 0 and 120.\n";
            return;
        }
    }

    string disease = readLine("New Disease/Problem: ");
    if (!disease.empty() && !validText(disease)) {
        cout << "Error: Invalid disease. Nothing was updated.\n";
        return;
    }

    if (!name.empty())
        patient->name = name;

    patient->age = age;

    if (!disease.empty())
        patient->disease = disease;

    saveAll();
    cout << "Patient record updated successfully.\n";
}

bool patientHasScheduledAppointment(int patientId) {
    for (int i = 0; i < apptCount; i++) {
        if (appts[i].patientId == patientId &&
            appts[i].status == "Scheduled")
            return true;
    }

    return false;
}

void deletePatient() {
    int id = readInt("Enter Patient ID to delete: ");

    Patient* current = head;
    Patient* previous = NULL;

    while (current != NULL && current->id != id) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        cout << "Error: Patient not found.\n";
        return;
    }

    if (patientHasScheduledAppointment(id)) {
        cout << "Error: Patient has scheduled appointments. Cancel them first.\n";
        return;
    }

    if (previous == NULL)
        head = current->next;
    else
        previous->next = current->next;

    delete current;
    saveAll();

    cout << "Patient deleted successfully.\n";
}

// Linear search by partial name, ignoring letter case.
void searchPatientByName() {
    string key = toLower(readLine("Enter patient name (or part of name): "));

    if (key.empty()) {
        cout << "Error: Search text cannot be empty.\n";
        return;
    }

    bool found = false;

    for (Patient* current = head; current != NULL; current = current->next) {
        if (toLower(current->name).find(key) != string::npos) {
            if (!found)
                printPatientHeader();

            printPatient(*current);
            found = true;
        }
    }

    if (!found)
        cout << "No patient found with that name.\n";
}

void searchPatientById() {
    int id = readInt("Enter Patient ID to search: ");
    Patient* patient = findPatient(id);

    if (patient == NULL) {
        cout << "Patient not found.\n";
        return;
    }

    printPatientHeader();
    printPatient(*patient);
}

void displayPatients() {
    if (head == NULL) {
        cout << "No patient records.\n";
        return;
    }

    printPatientHeader();

    for (Patient* current = head; current != NULL; current = current->next)
        printPatient(*current);

    cout << "Total patients: " << patientCount() << '\n';
}

// Bubble sort by patient name. Sorting is performed on a copy,
// so the original linked-list order is not changed.
void displayPatientsSorted() {
    int count = patientCount();

    if (count == 0) {
        cout << "No patient records.\n";
        return;
    }

    Patient* array = new Patient[count];
    int index = 0;

    for (Patient* current = head; current != NULL; current = current->next)
        array[index++] = *current;

    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - 1 - i; j++) {
            if (toLower(array[j].name) > toLower(array[j + 1].name)) {
                Patient temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }

    cout << "\nPatients sorted by name:\n";
    printPatientHeader();

    for (int i = 0; i < count; i++)
        printPatient(array[i]);

    delete[] array;
}

// -------------------- Linked queue module --------------------

void enqueue(int appointmentId) {
    QNode* node = new QNode;
    node->apptId = appointmentId;
    node->next = NULL;

    if (qRear == NULL) {
        qFront = qRear = node;
    } else {
        qRear->next = node;
        qRear = node;
    }
}

// Removes and returns the front appointment ID.
// Returns -1 when the queue is empty.
int dequeue() {
    if (qFront == NULL)
        return -1;

    QNode* temp = qFront;
    int id = temp->apptId;

    qFront = qFront->next;

    if (qFront == NULL)
        qRear = NULL;

    delete temp;
    return id;
}

bool removeFromQueue(int appointmentId) {
    QNode* current = qFront;
    QNode* previous = NULL;

    while (current != NULL && current->apptId != appointmentId) {
        previous = current;
        current = current->next;
    }

    if (current == NULL)
        return false;

    if (previous == NULL)
        qFront = current->next;
    else
        previous->next = current->next;

    if (current == qRear)
        qRear = previous;

    delete current;
    return true;
}

void clearQueue() {
    while (qFront != NULL)
        dequeue();

    qRear = NULL;
}

// -------------------- Appointment array module --------------------

// Binary search. The appointment array is kept sorted by ID.
int findAppointmentIndex(int id) {
    int low = 0;
    int high = apptCount - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;

        if (appts[mid].id == id)
            return mid;

        if (appts[mid].id < id)
            low = mid + 1;
        else
            high = mid - 1;
    }

    return -1;
}

void printAppointmentHeader() {
    cout << left << setw(8) << "AppID"
         << setw(11) << "PatientID"
         << setw(18) << "Doctor"
         << setw(12) << "Date"
         << setw(8) << "Time"
         << setw(12) << "Status" << '\n';

    cout << string(69, '-') << '\n';
}

void printAppointment(const Appointment& appointment) {
    cout << left << setw(8) << appointment.id
         << setw(11) << appointment.patientId
         << setw(18) << appointment.doctor
         << setw(12) << appointment.date
         << setw(8) << appointment.time
         << setw(12) << appointment.status << '\n';
}

bool doctorHasAppointment(const string& doctor,
                          const string& date,
                          const string& time) {
    for (int i = 0; i < apptCount; i++) {
        if (appts[i].status == "Scheduled" &&
            toLower(appts[i].doctor) == toLower(doctor) &&
            appts[i].date == date &&
            appts[i].time == time)
            return true;
    }

    return false;
}

void bookAppointment() {
    if (apptCount >= MAX_APPT) {
        cout << "Error: Maximum appointment limit (" << MAX_APPT
             << ") reached.\n";
        return;
    }

    int patientId = readInt("Enter Patient ID: ");

    if (findPatient(patientId) == NULL) {
        cout << "Error: Patient not found. Register the patient first.\n";
        return;
    }

    string doctor = readLine("Enter Doctor Name: ");
    if (!validText(doctor)) {
        cout << "Error: Invalid doctor name.\n";
        return;
    }

    string date = readLine("Enter Date (YYYY-MM-DD): ");
    if (!validDate(date)) {
        cout << "Error: Invalid date. Use a valid date from 2000 to 2100.\n";
        return;
    }

    string time = readLine("Enter Time (HH:MM, 08:00-19:59): ");
    if (!validTime(time)) {
        cout << "Error: Invalid time. Hospital hours are 08:00-19:59.\n";
        return;
    }

    if (doctorHasAppointment(doctor, date, time)) {
        cout << "Error: This doctor already has an appointment at that date and time.\n";
        return;
    }

    Appointment appointment;
    appointment.id = nextApptId++;
    appointment.patientId = patientId;
    appointment.doctor = doctor;
    appointment.date = date;
    appointment.time = time;
    appointment.status = "Scheduled";

    // IDs are generated in increasing order, so appending keeps
    // the array sorted by appointment ID for binary search.
    appts[apptCount] = appointment;
    apptCount++;

    enqueue(appointment.id);
    saveAll();

    cout << "Appointment booked successfully. Appointment ID = "
         << appointment.id << '\n';
}

void processNextAppointment() {
    if (qFront == NULL) {
        cout << "Queue is empty. No appointment to process.\n";
        return;
    }

    int id = dequeue();
    int index = findAppointmentIndex(id);

    // Defensive check in case the queue and appointment array
    // become inconsistent.
    if (index == -1) {
        cout << "Error: Appointment record was not found.\n";
        return;
    }

    if (appts[index].status != "Scheduled") {
        cout << "This appointment is no longer scheduled. Skipped.\n";
        return;
    }

    appts[index].status = "Completed";
    saveAll();

    cout << "Appointment processed successfully (Completed):\n";
    printAppointmentHeader();
    printAppointment(appts[index]);
}

void cancelAppointment() {
    int id = readInt("Enter Appointment ID to cancel: ");
    int index = findAppointmentIndex(id);

    if (index == -1) {
        cout << "Error: Appointment not found. Cannot cancel unavailable appointment.\n";
        return;
    }

    if (appts[index].status != "Scheduled") {
        cout << "Error: Appointment is already " << appts[index].status
             << ". Only scheduled appointments can be cancelled.\n";
        return;
    }

    removeFromQueue(id);
    appts[index].status = "Cancelled";
    saveAll();

    cout << "Appointment cancelled successfully.\n";
}

void viewAppointmentQueue() {
    if (qFront == NULL) {
        cout << "Appointment queue is empty.\n";
        return;
    }

    cout << "\nWaiting appointments (front of queue first):\n";
    printAppointmentHeader();

    for (QNode* current = qFront; current != NULL; current = current->next) {
        int index = findAppointmentIndex(current->apptId);

        if (index != -1 && appts[index].status == "Scheduled")
            printAppointment(appts[index]);
    }
}

// Selection sort by date and then time, using a temporary copy.
void viewAllAppointmentsSorted() {
    if (apptCount == 0) {
        cout << "No appointments recorded.\n";
        return;
    }

    Appointment temp[MAX_APPT];

    for (int i = 0; i < apptCount; i++)
        temp[i] = appts[i];

    for (int i = 0; i < apptCount - 1; i++) {
        int minIndex = i;

        for (int j = i + 1; j < apptCount; j++) {
            string currentKey = temp[j].date + " " + temp[j].time;
            string minKey = temp[minIndex].date + " " + temp[minIndex].time;

            if (currentKey < minKey)
                minIndex = j;
        }

        if (minIndex != i) {
            Appointment swapTemp = temp[i];
            temp[i] = temp[minIndex];
            temp[minIndex] = swapTemp;
        }
    }

    cout << "\nAll appointments sorted by date and time:\n";
    printAppointmentHeader();

    for (int i = 0; i < apptCount; i++)
        printAppointment(temp[i]);
}

// -------------------- File handling --------------------
void saveAll() {
    ofstream patientFile(PATIENT_FILE.c_str());

    if (!patientFile) {
        cout << "Error: Could not open " << PATIENT_FILE << " for writing.\n";
        return;
    }

    for (Patient* current = head; current != NULL; current = current->next) {
        patientFile << current->id << '|'
                    << current->name << '|'
                    << current->age << '|'
                    << current->gender << '|'
                    << current->disease << '\n';
    }

    patientFile.close();

    ofstream appointmentFile(APPT_FILE.c_str());

    if (!appointmentFile) {
        cout << "Error: Could not open " << APPT_FILE << " for writing.\n";
        return;
    }

    for (int i = 0; i < apptCount; i++) {
        appointmentFile << appts[i].id << '|'
                        << appts[i].patientId << '|'
                        << appts[i].doctor << '|'
                        << appts[i].date << '|'
                        << appts[i].time << '|'
                        << appts[i].status << '\n';
    }

    appointmentFile.close();
}

// Sorts the loaded appointment array by ID using insertion sort.
// This restores the condition required by binary search, even if
// records in appointments.txt were manually rearranged.
void sortAppointmentsByIdAfterLoading() {
    for (int i = 1; i < apptCount; i++) {
        Appointment key = appts[i];
        int j = i - 1;

        while (j >= 0 && appts[j].id > key.id) {
            appts[j + 1] = appts[j];
            j--;
        }

        appts[j + 1] = key;
    }
}

void loadAll() {
    // Load patients first, because appointments refer to patient IDs.
    ifstream patientFile(PATIENT_FILE.c_str());
    string line;
    Patient* tail = NULL;

    while (getline(patientFile, line)) {
        if (trim(line).empty())
            continue;

        stringstream stream(line);
        string idText, name, ageText, gender, disease;

        if (!getline(stream, idText, '|') ||
            !getline(stream, name, '|') ||
            !getline(stream, ageText, '|') ||
            !getline(stream, gender, '|') ||
            !getline(stream, disease, '|')) {
            cout << "Warning: Skipped malformed patient record.\n";
            continue;
        }

        int id = atoi(idText.c_str());
        int age = atoi(ageText.c_str());

        if (id < 1 || id > 999999 || age < 0 || age > 120 ||
            !validText(name) || !validGender(gender) ||
            !validText(disease) || findPatient(id) != NULL) {
            cout << "Warning: Skipped invalid/duplicate patient record.\n";
            continue;
        }

        Patient* patient = new Patient;
        patient->id = id;
        patient->name = name;
        patient->age = age;
        patient->gender = (toLower(gender) == "m") ? "M" : "F";
        patient->disease = disease;
        patient->next = NULL;

        if (head == NULL)
            head = tail = patient;
        else {
            tail->next = patient;
            tail = patient;
        }
    }

    patientFile.close();

    // Load appointment records.
    ifstream appointmentFile(APPT_FILE.c_str());

    while (apptCount < MAX_APPT && getline(appointmentFile, line)) {
        if (trim(line).empty())
            continue;

        stringstream stream(line);
        string idText, patientIdText, doctor, date, time, status;

        if (!getline(stream, idText, '|') ||
            !getline(stream, patientIdText, '|') ||
            !getline(stream, doctor, '|') ||
            !getline(stream, date, '|') ||
            !getline(stream, time, '|') ||
            !getline(stream, status, '|')) {
            cout << "Warning: Skipped malformed appointment record.\n";
            continue;
        }

        int id = atoi(idText.c_str());
        int patientId = atoi(patientIdText.c_str());

        bool duplicateId = false;
        for (int i = 0; i < apptCount; i++) {
            if (appts[i].id == id) {
                duplicateId = true;
                break;
            }
        }

        if (id < 1 || patientId < 1 || findPatient(patientId) == NULL ||
            !validText(doctor) || !validDate(date) || !validTime(time) ||
            !validStatus(status) || duplicateId) {
            cout << "Warning: Skipped invalid/duplicate appointment record.\n";
            continue;
        }

        Appointment appointment;
        appointment.id = id;
        appointment.patientId = patientId;
        appointment.doctor = doctor;
        appointment.date = date;
        appointment.time = time;
        appointment.status = status;

        appts[apptCount++] = appointment;

        if (id >= nextApptId)
            nextApptId = id + 1;
    }

    appointmentFile.close();

    // Restore ascending ID order before using binary search.
    sortAppointmentsByIdAfterLoading();

    // Rebuild the FIFO queue in appointment-ID order. IDs are assigned
    // in booking order, so this preserves the original arrival order.
    clearQueue();

    for (int i = 0; i < apptCount; i++) {
        if (appts[i].status == "Scheduled")
            enqueue(appts[i].id);
    }
}

// -------------------- Memory cleanup --------------------

void clearMemory() {
    while (head != NULL) {
        Patient* temp = head;
        head = head->next;
        delete temp;
    }

    clearQueue();
}

// -------------------- Menu and main --------------------

void showMenu() {
    cout << "\n===============================================\n";
    cout << " HOSPITAL APPOINTMENT & PATIENT MANAGEMENT\n";
    cout << "===============================================\n";
    cout << " 1. Register Patient\n";
    cout << " 2. Update Patient\n";
    cout << " 3. Delete Patient\n";
    cout << " 4. Search Patient by ID\n";
    cout << " 5. Search Patient by Name\n";
    cout << " 6. Display All Patients\n";
    cout << " 7. Display Patients Sorted by Name\n";
    cout << " 8. Book Appointment\n";
    cout << " 9. Process Next Appointment (FIFO)\n";
    cout << "10. Cancel Appointment\n";
    cout << "11. View Appointment Queue\n";
    cout << "12. View All Appointments Sorted by Date/Time\n";
    cout << " 0. Save and Exit\n";
    cout << "===============================================\n";
}

int main() {
    loadAll();

    cout << "Hospital Management System started.\n";
    cout << "Patient records loaded: " << patientCount() << '\n';
    cout << "Appointment records loaded: " << apptCount << '\n';

    int choice = -1;

    while (choice != 0) {
        showMenu();
        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1:
                registerPatient();
                break;

            case 2:
                updatePatient();
                break;

            case 3:
                deletePatient();
                break;

            case 4:
                searchPatientById();
                break;

            case 5:
                searchPatientByName();
                break;

            case 6:
                displayPatients();
                break;

            case 7:
                displayPatientsSorted();
                break;

            case 8:
                bookAppointment();
                break;

            case 9:
                processNextAppointment();
                break;

            case 10:
                cancelAppointment();
                break;

            case 11:
                viewAppointmentQueue();
                break;

            case 12:
                viewAllAppointmentsSorted();
                break;

            case 0:
                saveAll();
                cout << "Data saved. Goodbye!\n";
                break;

            default:
                cout << "Invalid choice. Please enter a number from 0 to 12.\n";
        }
    }

    clearMemory();
    return 0;
}
