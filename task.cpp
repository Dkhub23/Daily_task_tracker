#define WIN32_LEAN_AND_MEAN
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <string>
#include <algorithm>
#include <windows.h>
#include <conio.h>

// ANSI color codes
#define RED    "\033[31m"
#define GREEN  "\033[32m"
#define YELLOW "\033[33m"
#define CYAN   "\033[36m"
#define BOLD   "\033[1m"
#define RESET  "\033[0m"

using namespace std;

// ✅ Enable ANSI escape codes on Windows console
void enableANSI()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

// ✅ Safe localtime wrapper
tm safeLocaltime(const time_t *t)
{
    tm result;
    memset(&result, 0, sizeof(tm));
    tm *ptr = localtime(t);
    if (ptr) result = *ptr;
    return result;
}

// ✅ Simple password masking on Windows
string getPassword()
{
    string password;
    char ch;
    while ((ch = _getch()) != '\r') // '\r' = Enter key on Windows
    {
        if (ch == '\b') // Backspace
        {
            if (!password.empty())
            {
                password.pop_back();
                cout << "\b \b";
            }
        }
        else
        {
            password += ch;
            cout << '*';
        }
    }
    cout << "\n";
    return password;
}

// ============================================================
//  UserManager — handles registration, login, user list
// ============================================================
class UserManager
{
private:
    const string USER_FILE = "users.txt";

    // Each line in users.txt: username|password
    bool userExists(const string &username)
    {
        ifstream file(USER_FILE);
        if (!file) return false;
        string line;
        while (getline(file, line))
        {
            size_t sep = line.find('|');
            if (sep != string::npos && line.substr(0, sep) == username)
                return true;
        }
        return false;
    }

    string getStoredPassword(const string &username)
    {
        ifstream file(USER_FILE);
        string line;
        while (getline(file, line))
        {
            size_t sep = line.find('|');
            if (sep != string::npos && line.substr(0, sep) == username)
                return line.substr(sep + 1);
        }
        return "";
    }

public:
    // Returns username on success, "" on failure
    string registerUser()
    {
        string username, password, confirm;

        cout << "\n--- REGISTER ---\n";
        cout << "Enter Username: ";
        cin >> username;

        if (username.empty())
        {
            cout << RED << "Username cannot be empty!\n" << RESET;
            return "";
        }
        // Disallow '|' in username (used as separator)
        if (username.find('|') != string::npos)
        {
            cout << RED << "Username cannot contain '|'!\n" << RESET;
            return "";
        }
        if (userExists(username))
        {
            cout << RED << "Username already taken!\n" << RESET;
            return "";
        }

        cout << "Enter Password: ";
        password = getPassword();

        cout << "Confirm Password: ";
        confirm = getPassword();

        if (password != confirm)
        {
            cout << RED << "Passwords do not match!\n" << RESET;
            return "";
        }
        if (password.empty())
        {
            cout << RED << "Password cannot be empty!\n" << RESET;
            return "";
        }

        ofstream file(USER_FILE, ios::app);
        file << username << "|" << password << "\n";
        file.close();

        cout << GREEN << "Registration successful! Welcome, " << username << "!\n" << RESET;
        return username;
    }

    // Returns username on success, "" on failure
    string loginUser()
    {
        string username, password;

        cout << "\n--- LOGIN ---\n";
        cout << "Enter Username: ";
        cin >> username;

        cout << "Enter Password: ";
        password = getPassword();

        if (!userExists(username))
        {
            cout << RED << "User not found!\n" << RESET;
            return "";
        }

        if (getStoredPassword(username) != password)
        {
            cout << RED << "Incorrect password!\n" << RESET;
            return "";
        }

        cout << GREEN << "Login successful! Welcome back, " << username << "!\n" << RESET;
        return username;
    }

    void listUsers()
    {
        ifstream file(USER_FILE);
        if (!file)
        {
            cout << "No users registered yet.\n";
            return;
        }

        cout << "\n--- REGISTERED USERS ---\n";
        string line;
        int count = 1;
        while (getline(file, line))
        {
            size_t sep = line.find('|');
            if (sep != string::npos)
                cout << count++ << ". " << line.substr(0, sep) << "\n";
        }
        if (count == 1) cout << "No users found.\n";
    }
};

// ============================================================
//  Task & DailyTaskTracker (per-user, unchanged logic)
// ============================================================
class Task
{
public:
    string name;
    bool completed;
    time_t deadline;

    Task(string n = "", bool c = false, time_t d = 0)
        : name(n), completed(c), deadline(d) {}
};

class DailyTaskTracker
{
private:
    vector<Task> tasks;
    string username; // current logged-in user

    // Per-user file names
    string rawDataFile()       { return "tasks_" + username + "_data.txt"; }
    string formattedFile()     { return "tasks_" + username + ".txt"; }

    void saveRawData()
    {
        ofstream file(rawDataFile());
        for (const Task &t : tasks)
        {
            file << t.name << "\n"
                 << t.completed << "\n"
                 << t.deadline << "\n";
        }
        file.close();
    }

    void loadData()
    {
        ifstream file(rawDataFile());
        if (!file) return;

        string name;
        bool completed;
        time_t deadline;

        while (getline(file, name))
        {
            if (name.empty()) continue;
            file >> completed;
            file >> deadline;
            file.ignore();
            tasks.push_back(Task(name, completed, deadline));
        }
        file.close();
    }

    void saveFormattedTable()
    {
        ofstream file(formattedFile());
        file << left << setw(5)  << "No."
             << setw(30) << "Task Name"
             << setw(20) << "Deadline"
             << "Status\n";
        file << "-----------------------------------------------------------------\n";

        for (int i = 0; i < (int)tasks.size(); i++)
        {
            file << left << setw(5)  << i + 1
                 << setw(30) << tasks[i].name
                 << setw(20) << formatTime(tasks[i].deadline)
                 << getStatus(tasks[i]) << "\n";
        }
        file.close();
    }

    string formatTime(time_t t)
    {
        char buffer[100];
        tm timeInfo = safeLocaltime(&t);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &timeInfo);
        return string(buffer);
    }

    time_t getDeadline()
    {
        time_t now = time(0);
        tm t = safeLocaltime(&now);

        int hour, min;
        cout << "Enter deadline time (HH MM): ";
        cin >> hour >> min;

        if (hour < 0 || hour > 23 || min < 0 || min > 59)
        {
            cout << "Invalid time! Defaulting to 1 hour from now.\n";
            return now + 3600;
        }

        t.tm_hour = hour;
        t.tm_min  = min;
        t.tm_sec  = 0;

        time_t d = mktime(&t);
        if (d < now)
            cout << YELLOW << "Warning: That time has already passed today!\n" << RESET;

        return d;
    }

    string getStatus(const Task &t) const
    {
        time_t now = time(0);
        if (t.completed)             return "COMPLETED";
        if (now > t.deadline)        return "EXPIRED";
        if (t.deadline - now <= 300) return "ALMOST";
        return "PENDING";
    }

public:
    DailyTaskTracker(const string &user) : username(user)
    {
        loadData();
        saveFormattedTable();
    }

    void addTask()
    {
        string name;
        cout << "\nEnter Task Name: ";
        getline(cin >> ws, name);

        if (name.empty())
        {
            cout << RED << "Task name cannot be empty!\n" << RESET;
            return;
        }
        for (const Task &t : tasks)
        {
            if (t.name == name && !t.completed)
            {
                cout << YELLOW << "Task already exists and is pending!\n" << RESET;
                return;
            }
        }

        time_t d = getDeadline();
        tasks.push_back(Task(name, false, d));
        saveRawData();
        saveFormattedTable();
        cout << GREEN << "Task added successfully!\n" << RESET;
    }

    void viewTasks()
    {
        if (tasks.empty())
        {
            cout << "\nNo tasks available.\n";
            return;
        }

        cout << "\n========== TASK LIST [" << CYAN << username << RESET << "] ==========\n";
        cout << left << setw(5)  << "No."
             << setw(30) << "Task Name"
             << setw(20) << "Deadline"
             << "Status\n";
        cout << "---------------------------------------------------------------\n";

        for (int i = 0; i < (int)tasks.size(); i++)
        {
            string status = getStatus(tasks[i]);
            cout << left << setw(5)  << i + 1
                 << setw(30) << tasks[i].name
                 << setw(20) << formatTime(tasks[i].deadline);

            if      (status == "EXPIRED")   cout << RED    << "EXPIRED"        << RESET;
            else if (status == "ALMOST")    cout << YELLOW << "SOON (< 5 min)" << RESET;
            else if (status == "PENDING")   cout << YELLOW << "PENDING"        << RESET;
            else                            cout << GREEN  << "COMPLETED"      << RESET;

            cout << "\n";
        }
    }

    void markCompleted()
    {
        if (tasks.empty()) { cout << "\nNo tasks available.\n"; return; }
        viewTasks();

        int index;
        cout << "\nEnter task number to mark completed: ";
        cin >> index;

        if (index < 1 || index > (int)tasks.size())
        {
            cout << RED << "Invalid task number!\n" << RESET; return;
        }
        if (tasks[index - 1].completed)
        {
            cout << YELLOW << "Task already completed!\n" << RESET; return;
        }
        if (time(0) > tasks[index - 1].deadline)
        {
            cout << RED << "Cannot complete: task already expired!\n" << RESET; return;
        }

        tasks[index - 1].completed = true;
        saveRawData();
        saveFormattedTable();
        cout << GREEN << "Task marked as completed!\n" << RESET;
    }

    void deleteTask()
    {
        if (tasks.empty()) { cout << "\nNo tasks available.\n"; return; }
        viewTasks();

        int index;
        cout << "\nEnter task number to delete: ";
        cin >> index;

        if (index < 1 || index > (int)tasks.size())
        {
            cout << RED << "Invalid task number!\n" << RESET; return;
        }

        tasks.erase(tasks.begin() + index - 1);
        saveRawData();
        saveFormattedTable();
        cout << GREEN << "Task deleted successfully!\n" << RESET;
    }

    void showProductivity()
    {
        if (tasks.empty()) { cout << "\nNo tasks available.\n"; return; }

        int completedCount = 0;
        for (const Task &t : tasks)
            if (t.completed) completedCount++;

        double productivity = (double)completedCount / (double)tasks.size() * 100.0;
        cout << fixed << setprecision(2);
        cout << "\n[" << CYAN << username << RESET << "] Today's Productivity: "
             << GREEN << productivity << "%" << RESET << "\n";
    }
};

// ============================================================
//  Auth Menu
// ============================================================
string authMenu(UserManager &um)
{
    int choice;
    string loggedInUser;

    do {
        cout << "\n=================================\n";
        cout << BOLD << "      DAILY TASK TRACKER\n" << RESET;
        cout << "=================================\n";
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "3. List Users\n";
        cout << "4. Exit\n";
        cout << "=================================\n";
        cout << "Enter Choice: ";

        if (!(cin >> choice))
        {
            cout << RED << "Invalid input!\n" << RESET;
            cin.clear();
            cin.ignore(1000, '\n');
            choice = 0;
            continue;
        }

        switch (choice)
        {
        case 1: loggedInUser = um.loginUser();    break;
        case 2: loggedInUser = um.registerUser(); break;
        case 3: um.listUsers();                   break;
        case 4: cout << "Goodbye!\n"; exit(0);
        default: cout << RED << "Invalid choice!\n" << RESET;
        }

    } while (loggedInUser.empty());

    return loggedInUser;
}

// ============================================================
//  Main Task Menu
// ============================================================
void taskMenu(const string &username)
{
    DailyTaskTracker tracker(username);
    int choice;

    do {
        cout << "\n=================================\n";
        cout << BOLD << "  DAILY TASK TRACKER" << RESET
             << " [" << CYAN << username << RESET << "]\n";
        cout << "=================================\n";
        cout << "1. Add Task\n";
        cout << "2. View Tasks\n";
        cout << "3. Mark Task Completed\n";
        cout << "4. Delete Task\n";
        cout << "5. Show Productivity Score\n";
        cout << "6. Switch User / Logout\n";
        cout << "7. Exit\n";
        cout << "=================================\n";
        cout << "Enter Choice: ";

        if (!(cin >> choice))
        {
            cout << RED << "\nInvalid input! Please enter a number.\n" << RESET;
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        switch (choice)
        {
        case 1: tracker.addTask();          break;
        case 2: tracker.viewTasks();        break;
        case 3: tracker.markCompleted();    break;
        case 4: tracker.deleteTask();       break;
        case 5: tracker.showProductivity(); break;
        case 6: cout << YELLOW << "Logging out...\n" << RESET; return; // back to auth menu
        case 7: cout << "Goodbye!\n"; exit(0);
        default: cout << RED << "\nInvalid choice!\n" << RESET;
        }

    } while (true);
}

// ============================================================
//  Entry Point
// ============================================================
int main()
{
    enableANSI();
    UserManager um;

    while (true) // allows switching users without restarting
    {
        string user = authMenu(um);
        taskMenu(user);
    }

    return 0;
}