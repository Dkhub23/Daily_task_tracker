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
#include <windows.h>
 
// ANSI color codes
#define RED    "\033[31m"
#define GREEN  "\033[32m"
#define YELLOW "\033[33m"
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
 
// ✅ Safe localtime wrapper (avoids localtime_s / localtime_r portability issues)
tm safeLocaltime(const time_t *t)
{
    tm result;
    memset(&result, 0, sizeof(tm));
    tm *ptr = localtime(t);   // standard C localtime — always available
    if (ptr) result = *ptr;
    return result;
}
 
class Task
{
public:
    string name;
    bool completed;
    time_t deadline;
 
    Task(string n = "", bool c = false, time_t d = 0)
    {
        name = n;
        completed = c;
        deadline = d;
    }
};
 
class DailyTaskTracker
{
private:
    vector<Task> tasks;
 
    // ✅ Save raw data
    void saveRawData()
    {
        ofstream file("tasks_data.txt");
 
        for (const Task &t : tasks)
        {
            file << t.name << "\n";
            file << t.completed << "\n";
            file << t.deadline << "\n";
        }
 
        file.close();
    }
 
    // ✅ Load data
    void loadData()
    {
        ifstream file("tasks_data.txt");
 
        if (!file)
            return;
 
        string name;
        bool completed;
        time_t deadline;
 
        while (getline(file, name))
        {
            if (name.empty()) continue; // skip blank lines
 
            file >> completed;
            file >> deadline;
            file.ignore();
 
            tasks.push_back(Task(name, completed, deadline));
        }
 
        file.close();
    }
 
    // ✅ Save formatted table
    void saveFormattedTable()
    {
        ofstream file("tasks.txt");
 
        file << left << setw(5)  << "No."
             << setw(30) << "Task Name"
             << setw(20) << "Deadline"
             << "Status\n";
 
        file << "-----------------------------------------------------------------\n";
 
        for (int i = 0; i < (int)tasks.size(); i++)
        {
            string status = getStatus(tasks[i]);
 
            file << left << setw(5)  << i + 1
                 << setw(30) << tasks[i].name
                 << setw(20) << formatTime(tasks[i].deadline)
                 << status << "\n";
        }
 
        file.close();
    }
 
    // ✅ Format time safely
    string formatTime(time_t t)
    {
        char buffer[100];
        tm timeInfo = safeLocaltime(&t);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &timeInfo);
        return string(buffer);
    }
 
    // ✅ Take deadline input
    time_t getDeadline()
    {
        time_t now = time(0);
        tm t = safeLocaltime(&now);
 
        int hour, min;
        cout << "Enter deadline time (HH MM): ";
        cin >> hour >> min;
 
        if (hour < 0 || hour > 23 || min < 0 || min > 59)
        {
            cout << "Invalid time entered! Defaulting to 1 hour from now.\n";
            return now + 3600;
        }
 
        t.tm_hour = hour;
        t.tm_min  = min;
        t.tm_sec  = 0;
 
        time_t d = mktime(&t);
 
        if (d < now)
        {
            cout << "Warning: Time already passed today!\n";
        }
 
        return d;
    }
 
    // ✅ Get status string
    string getStatus(const Task &t) const
    {
        time_t now = time(0);
 
        if (t.completed)
            return "COMPLETED";
        else if (now > t.deadline)
            return "EXPIRED";
        else if (t.deadline - now <= 300) // 5 minutes = 300 sec
            return "ALMOST";
        else
            return "PENDING";
    }
 
public:
    DailyTaskTracker()
    {
        loadData();
        saveFormattedTable();
    }
 
    // ✅ Add task
    void addTask()
    {
        string name;
 
        cout << "\nEnter Task Name: ";
        getline(cin >> ws, name);
 
        if (name.empty())
        {
            cout << "Task Name Cannot Be Empty!\n";
            return;
        }
 
        for (const Task &t : tasks)
        {
            if (t.name == name && !t.completed)
            {
                cout << "Task Already Exists and Pending!\n";
                return;
            }
        }
 
        time_t d = getDeadline();
        tasks.push_back(Task(name, false, d));
 
        saveRawData();
        saveFormattedTable();
 
        cout << "Task Added Successfully!\n";
    }
 
    // ✅ View tasks
    void viewTasks()
    {
        if (tasks.empty())
        {
            cout << "\nNo Tasks Available.\n";
            return;
        }
 
        cout << "\n========== TASK LIST ==========\n";
        cout << left << setw(5)  << "No."
             << setw(30) << "Task Name"
             << setw(20) << "Deadline"
             << "Status\n";
        cout << "-----------------------------------------------------------------\n";
 
        for (int i = 0; i < (int)tasks.size(); i++)
        {
            string status = getStatus(tasks[i]);
 
            cout << left << setw(5)  << i + 1
                 << setw(30) << tasks[i].name
                 << setw(20) << formatTime(tasks[i].deadline);
 
            if (status == "EXPIRED")
            {
                cout << RED << "EXPIRED" << RESET;
            }
            else if (status == "ALMOST")
            {
                cout << YELLOW << "SOON (< 5 min)" << RESET;
            }
            else if (status == "PENDING")
            {
                cout << YELLOW << "PENDING" << RESET;
            }
            else
            {
                cout << GREEN << "COMPLETED" << RESET;
            }
 
            cout << "\n";
        }
    }
 
    // ✅ Mark completed
    void markCompleted()
    {
        if (tasks.empty())
        {
            cout << "\nNo Tasks Available.\n";
            return;
        }
 
        viewTasks();
 
        int index;
        cout << "\nEnter Task Number to Complete: ";
        cin >> index;
 
        if (index < 1 || index > (int)tasks.size())
        {
            cout << "Invalid Task Number!\n";
            return;
        }
 
        if (tasks[index - 1].completed)
        {
            cout << "Task Already Completed!\n";
            return;
        }
 
        if (time(0) > tasks[index - 1].deadline)
        {
            cout << "Invalid: Task already expired!\n";
            return;
        }
 
        tasks[index - 1].completed = true;
 
        saveRawData();
        saveFormattedTable();
 
        cout << "Task Marked Completed!\n";
    }
 
    // ✅ Delete task
    void deleteTask()
    {
        if (tasks.empty())
        {
            cout << "\nNo Tasks Available.\n";
            return;
        }
 
        viewTasks();
 
        int index;
        cout << "\nEnter Task Number to Delete: ";
        cin >> index;
 
        if (index < 1 || index > (int)tasks.size())
        {
            cout << "Invalid Task Number!\n";
            return;
        }
 
        tasks.erase(tasks.begin() + index - 1);
 
        saveRawData();
        saveFormattedTable();
 
        cout << "Task Deleted Successfully!\n";
    }
 
    // ✅ Productivity
    void showProductivity()
    {
        if (tasks.empty())
        {
            cout << "\nNo Tasks Available.\n";
            return;
        }
 
        int completedCount = 0;
 
        for (const Task &t : tasks)
        {
            if (t.completed)
                completedCount++;
        }
 
        double productivity = (double)completedCount / (double)tasks.size() * 100.0;
 
        cout << fixed << setprecision(2);
        cout << "\nToday's Productivity: " << productivity << "%\n";
    }
};
 
int main()
{
    enableANSI(); // ✅ Enable color output on Windows
 
    DailyTaskTracker tracker;
    int choice;
 
    do
    {
        cout << "\n=================================\n";
        cout << "      DAILY TASK TRACKER\n";
        cout << "=================================\n";
        cout << "1. Add Task\n";
        cout << "2. View Tasks\n";
        cout << "3. Mark Task Completed\n";
        cout << "4. Delete Task\n";
        cout << "5. Show Productivity Score\n";
        cout << "6. Exit\n";
        cout << "=================================\n";
        cout << "Enter Choice: ";
 
        if (!(cin >> choice))
        {
            cout << "\nInvalid input! Please enter a number.\n";
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }
 
        switch (choice)
        {
        case 1: tracker.addTask();        break;
        case 2: tracker.viewTasks();      break;
        case 3: tracker.markCompleted();  break;
        case 4: tracker.deleteTask();     break;
        case 5: tracker.showProductivity(); break;
        case 6: cout << "Goodbye!\n";    break;
        default: cout << "\nInvalid Choice!\n";
        }
 
    } while (choice != 6);
 
    return 0;
}