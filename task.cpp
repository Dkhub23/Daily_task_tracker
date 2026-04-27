#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
using namespace std;

class Task {
public:
    string name;
    bool completed;

    Task(string n = "", bool c = false) {
        name = n;
        completed = c;
    }
};

class DailyTaskTracker {
private:
    vector<Task> tasks;

    void saveRawData() {
        ofstream file("tasks_data.txt");

        for (Task t : tasks) {
            file << t.name << endl;
            file << t.completed << endl;
        }

        file.close();
    }

    void saveFormattedTable() {
        ofstream file("tasks.txt");

        file << left << setw(5) << "No."
             << setw(30) << "Task Name"
             << "Status" << endl;

        file << "---------------------------------------------" << endl;

        for (int i = 0; i < tasks.size(); i++) {
            file << left << setw(5) << i + 1
                 << setw(30) << tasks[i].name
                 << (tasks[i].completed ? "Completed" : "Pending") << endl;
        }

        file.close();
    }

public:
    DailyTaskTracker() {
      
        saveFormattedTable();
    }

    void addTask() {
        string name;

        cout << "\nEnter Task Name: ";
        getline(cin >> ws, name);

        if (name.empty()) {
            cout << "Task Name Cannot Be Empty!\n";
            return;
        }

        for (Task t : tasks) {
            if (t.name == name && !t.completed) {
                cout << "Task Already Added And still Pending!\n";
                return;
            }
        }

        tasks.push_back(Task(name));

        saveRawData();
        saveFormattedTable();

        cout << "Task Added Successfully!\n";
    }

    void viewTasks() {
        if (tasks.empty()) {
            cout << "\nNo Tasks Available.\n";
            return;
        }

        cout << "\n========== DAILY TASKS ==========\n";
        cout << left << setw(5) << "No."
             << setw(30) << "Task Name"
             << "Status\n";
        cout << "---------------------------------------------\n";

        for (int i = 0; i < tasks.size(); i++) {
            cout << left << setw(5) << i + 1
                 << setw(30) << tasks[i].name
                 << (tasks[i].completed ? "Completed" : "Pending") << endl;
        }
    }

    void markCompleted() {
        int index;

        viewTasks();

        if (tasks.empty()) return;

        cout << "\nEnter Task Number to Complete: ";
        cin >> index;

        if (index < 1 || index > tasks.size()) {
            cout << "Invalid Task Number!\n";
            return;
        }

        if (tasks[index - 1].completed) {
            cout << "Task Already Completed!\n";
            return;
        }

        tasks[index - 1].completed = true;

        saveRawData();
        saveFormattedTable();

        cout << "Task Marked Completed!\n";
    }

    void deleteTask() {
        int index;

        viewTasks();

        if (tasks.empty()) return;

        cout << "\nEnter Task Number to Delete: ";
        cin >> index;

        if (index < 1 || index > tasks.size()) {
            cout << "Invalid Task Number!\n";
            return;
        }

        tasks.erase(tasks.begin() + index - 1);

        saveRawData();
        saveFormattedTable();

        cout << "Task Deleted Successfully!\n";
    }

    void showProductivity() {
        if (tasks.empty()) {
            cout << "\nNo Tasks Available.\n";
            return;
        }

        int completedCount = 0;

        for (Task t : tasks) {
            if (t.completed)
                completedCount++;
        }

        double productivity = (double)completedCount / tasks.size() * 100;

        cout << fixed << setprecision(2);
        cout << "\nToday's Productivity: " << productivity << "%\n";
    }
};

int main() {
    DailyTaskTracker tracker;
    int choice;

    do {
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
        cin >> choice;

        switch (choice) {
            case 1: tracker.addTask(); break;
            case 2: tracker.viewTasks(); break;
            case 3: tracker.markCompleted(); break;
            case 4: tracker.deleteTask(); break;
            case 5: tracker.showProductivity(); break;
            case 6: cout << "\nThank You For Using Daily Task Tracker!\n"; break;
            default: cout << "\nInvalid Choice!\n";
        }

    } while (choice != 6);

    return 0;
} 