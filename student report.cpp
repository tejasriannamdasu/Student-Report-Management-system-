#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <termios.h>
#include <unistd.h>
using namespace std;

// ---------------------------------------------------
// STRUCTURES
// ---------------------------------------------------

struct User {
    string username;
    string password;
    string role;
};

struct Student {
    string regno;
    string name;
    int m1, m2, m3;
};

// ---------------------------------------------------
// FUNCTION HEADERS
// ---------------------------------------------------

string getPasswordMasked(const string &prompt);

bool login(User &active);

void adminPanel(User &active);
void staffPanel(User &active);
void studentPanel(User &active);
void parentPanel(User &active);
void accountantPanel(User &active);

void addStudent();
void showAllStudents();
void modifyStudent();
void removeStudent();
void fetchStudent(const string &roll);
void findStudent();

vector<Student> readStudents();
void writeStudents(const vector<Student> &all);

void changePass(User &active);

// NEW: Add new admin
void createNewAdmin();

void ensureAdminExists();

// ---------------------------------------------------
// WORKING PASSWORD MASKING (MAC+LINUX)
// ---------------------------------------------------

string getPasswordMasked(const string &prompt) {
    cout << prompt;

    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    string password;
    char ch;

    while (true) {
        ch = getchar();
        if (ch == '\n' || ch == '\r') break;
        else if (ch == 127 || ch == 8) {   // Backspace
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
            }
        } else {
            password.push_back(ch);
            cout << "*";
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    cout << endl;

    return password;
}

// ---------------------------------------------------
// MAIN
// ---------------------------------------------------

int main() {
    ensureAdminExists();

    while (true) {
        cout << "\n=============================\n"
             << "     STUDENT RECORD SYSTEM\n"
             << "=============================\n";

        User current;
        if (!login(current)) {
            cout << "Login failed! Try again.\n\n";
            continue;
        }

        if (current.role == "admin") adminPanel(current);
        else if (current.role == "staff") staffPanel(current);
        else if (current.role == "student") studentPanel(current);
        else if (current.role == "parent") parentPanel(current);
        else if (current.role == "accountant") accountantPanel(current);
        else {
            cout << "Unknown role.\n";
            break;
        }
    }
}

// ---------------------------------------------------
// ENSURE DEFAULT ADMIN EXISTS
// ---------------------------------------------------

void ensureAdminExists() {
    ifstream fin("credentials.txt");
    bool exists = false;

    if (fin) {
        string u, p, r;
        while (fin >> u >> p >> r) {
            if (u == "admin" && r == "admin") {
                exists = true;
                break;
            }
        }
    }
    fin.close();

    if (!exists) {
        ofstream fout("credentials.txt", ios::app);
        fout << "admin admin@123 admin\n";
        fout.close();
        cout << "[System] Default admin created (admin / admin@123)\n";
    }
}

// ---------------------------------------------------
// LOGIN (AUTO ROLE DETECTION)
// ---------------------------------------------------

bool login(User &active) {
    string user, pass;

    cout << "\n----- SIGN IN -----\n";
    cout << "Username: ";
    cin >> user;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    pass = getPasswordMasked("Password: ");

    ifstream fin("credentials.txt");
    if (!fin) {
        cout << "Error: credentials.txt not found!\n";
        return false;
    }

    string u, p, r;
    while (fin >> u >> p >> r) {
        if (u == user && p == pass) {
            active = {u, p, r};
            cout << "Logged in as " << r << "!\n\n";
            return true;
        }
    }

    return false;
}

// ---------------------------------------------------
// ADMIN PANEL
// ---------------------------------------------------

void adminPanel(User &active) {
    int ch;
    do {
        cout << "===== ADMIN MENU =====\n"
             << "1. Add Student\n"
             << "2. View All Students\n"
             << "3. Update Student\n"
             << "4. Delete Student\n"
             << "5. Search Student\n"
             << "6. Change Password\n"
             << "7. Create New Admin\n"
             << "8. Logout\n"
             << "Choice: ";
        cin >> ch;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch(ch) {
            case 1: addStudent(); break;
            case 2: showAllStudents(); break;
            case 3: modifyStudent(); break;
            case 4: removeStudent(); break;
            case 5: findStudent(); break;
            case 6: changePass(active); break;
            case 7: createNewAdmin(); break;
            case 8: cout << "Logging out...\n"; break;
            default: cout << "Invalid choice.\n"; break;
        }

        cout << "\n";
    } while(ch != 8);
}

// ---------------------------------------------------
// CREATE NEW ADMIN
// ---------------------------------------------------

void createNewAdmin() {
    string uname, pass;

    cout << "Enter new admin username: ";
    cin >> uname;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    pass = getPasswordMasked("Enter password: ");

    ofstream fout("credentials.txt", ios::app);
    fout << uname << " " << pass << " admin\n";

    cout << "New admin created!\n";
}

// ---------------------------------------------------
// OTHER ROLE PANELS
// ---------------------------------------------------

void staffPanel(User &active) {
    int c;
    do {
        cout << "===== STAFF MENU =====\n"
             << "1. View All Students\n"
             << "2. Update Student Marks\n"
             << "3. Change Password\n"
             << "4. Logout\n"
             << "Choice: ";
        cin >> c;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch(c) {
            case 1: showAllStudents(); break;
            case 2: modifyStudent(); break;
            case 3: changePass(active); break;
            case 4: cout << "Logging out...\n"; break;
            default: cout << "Invalid option.\n"; break;
        }
        cout << "\n";
    } while(c != 4);
}

void studentPanel(User &active) {
    int c;
    do {
        cout << "===== STUDENT MENU =====\n"
             << "1. View My Record\n"
             << "2. Change Password\n"
             << "3. Logout\n"
             << "Choice: ";
        cin >> c;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch(c) {
            case 1: fetchStudent(active.username); break;
            case 2: changePass(active); break;
            case 3: cout << "Logging out...\n"; break;
            default: cout << "Invalid choice.\n"; break;
        }
        cout << "\n";
    } while(c != 3);
}

void parentPanel(User &active) {
    int c;
    do {
        cout << "===== PARENT MENU =====\n"
             << "1. View Child Record\n"
             << "2. Change Password\n"
             << "3. Logout\n"
             << "Choice: ";
        cin >> c;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch(c) {
            case 1: {
                string reg;
                cout << "Enter child regno: ";
                cin >> reg;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                fetchStudent(reg);
                break;
            }
            case 2: changePass(active); break;
            case 3: cout << "Logging out...\n"; break;
            default: cout << "Invalid choice.\n"; break;
        }
        cout << "\n";
    } while(c != 3);
}

void accountantPanel(User &active) {
    int c;
    do {
        cout << "===== ACCOUNTANT MENU =====\n"
             << "1. View All Students\n"
             << "2. Change Password\n"
             << "3. Logout\n"
             << "Choice: ";
        cin >> c;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch(c) {
            case 1: showAllStudents(); break;
            case 2: changePass(active); break;
            case 3: cout << "Logging out...\n"; break;
            default: cout << "Invalid choice.\n"; break;
        }
        cout << "\n";
    } while(c != 3);
}

// ---------------------------------------------------
// FILE HANDLING FOR STUDENTS
// ---------------------------------------------------

vector<Student> readStudents() {
    vector<Student> list;
    ifstream fin("students.txt");
    if (!fin) return list;

    Student s;
    while (fin >> s.regno >> ws && getline(fin, s.name, '\t') && fin >> s.m1 >> s.m2 >> s.m3)
        list.push_back(s);

    return list;
}

void writeStudents(const vector<Student> &all) {
    ofstream fout("students.txt");
    for (auto &s : all)
        fout << s.regno << "\t" << s.name << "\t"
             << s.m1 << "\t" << s.m2 << "\t" << s.m3 << "\n";
}

// ---------------------------------------------------
// STUDENT FEATURES
// ---------------------------------------------------

void addStudent() {
    Student s;

    cout << "Regno: ";
    cin >> s.regno;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Name: ";
    getline(cin, s.name);

    cout << "Marks (3): ";
    cin >> s.m1 >> s.m2 >> s.m3;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    ofstream fout("students.txt", ios::app);
    fout << s.regno << "\t" << s.name << "\t"
         << s.m1 << "\t" << s.m2 << "\t" << s.m3 << "\n";

    string tempPass = s.regno + "@123";
    ofstream cred("credentials.txt", ios::app);
    cred << s.regno << " " << tempPass << " student\n";

    cout << "Student added.\n";
    cout << "Login → " << s.regno << "\n";
    cout << "Password → " << tempPass << "\n";

    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void showAllStudents() {
    auto all = readStudents();
    if (all.empty()) {
        cout << "No student data.\n";
        return;
    }

    cout << "RegNo\tName\tM1\tM2\tM3\n";
    for (auto &s : all)
        cout << s.regno << "\t" << s.name << "\t"
             << s.m1 << "\t" << s.m2 << "\t" << s.m3 << "\n";
}

void modifyStudent() {
    string r;
    cout << "Enter regno: ";
    cin >> r;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    auto all = readStudents();
    bool ok = false;

    for (auto &s : all) {
        if (s.regno == r) {
            ok = true;
            cout << "New name: ";
            getline(cin, s.name);
            cout << "New marks (3): ";
            cin >> s.m1 >> s.m2 >> s.m3;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
    }

    if (!ok) {
        cout << "Not found.\n";
        return;
    }

    writeStudents(all);
    cout << "Updated.\n";
}

void removeStudent() {
    string r;
    cout << "Enter regno: ";
    cin >> r;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    auto all = readStudents();
    vector<Student> newList;
    bool ok = false;

    for (auto &s : all) {
        if (s.regno == r) ok = true;
        else newList.push_back(s);
    }

    if (!ok) {
        cout << "Not found.\n";
        return;
    }

    writeStudents(newList);
    cout << "Deleted.\n";
}

void fetchStudent(const string &roll) {
    auto list = readStudents();
    for (auto &s : list) {
        if (s.regno == roll) {
            cout << "RegNo: " << s.regno << "\n";
            cout << "Name : " << s.name << "\n";
            cout << "Marks: " << s.m1 << " " << s.m2 << " " << s.m3 << "\n";
            return;
        }
    }
    cout << "Record not found.\n";
}

void findStudent() {
    int c;
    cout << "Search by:\n1. RegNo\n2. Name\nChoice: ";
    cin >> c;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    auto all = readStudents();
    if (all.empty()) {
        cout << "No data.\n";
        return;
    }

    if (c == 1) {
        string r;
        cout << "Enter regno: ";
        cin >> r;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        for (auto &s : all)
            if (s.regno == r) {
                cout << s.regno << " " << s.name << " "
                     << s.m1 << " " << s.m2 << " " << s.m3 << "\n";
                return;
            }

        cout << "Not found.\n";
    }
    else if (c == 2) {
        string nm;
        cout << "Enter name: ";
        cin >> nm;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        bool any = false;
        for (auto &s : all)
            if (s.name == nm) {
                cout << s.regno << " " << s.name << " "
                     << s.m1 << " " << s.m2 << " " << s.m3 << "\n";
                any = true;
            }

        if (!any) cout << "Not found.\n";
    }
}

// ---------------------------------------------------
// CHANGE PASSWORD
// ---------------------------------------------------

void changePass(User &active) {
    string old = getPasswordMasked("Current password: ");
    if (old != active.password) {
        cout << "Incorrect.\n";
        return;
    }

    string np = getPasswordMasked("New password: ");
    string cp = getPasswordMasked("Confirm password: ");

    if (np != cp) {
        cout << "Mismatch.\n";
        return;
    }

    ifstream fin("credentials.txt");
    vector<User> all;
    string u, p, r;

    while (fin >> u >> p >> r)
        all.push_back({u, p, r});
    fin.close();

    for (auto &x : all)
        if (x.username == active.username && x.role == active.role)
            x.password = np;

    ofstream fout("credentials.txt");
    for (auto &x : all)
        fout << x.username << " " << x.password << " " << x.role << "\n";

    active.password = np;
    cout << "Password changed.\n";
}