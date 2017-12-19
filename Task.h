#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <vector>
#include <list>
#include <deque>
#include <fstream>
#include <algorithm>
#include <time.h>
#include <math.h>
#include <random>
#include <limits>

using namespace std;

class PutError {
public:
    string wh;
    PutError (string wh) {
        this->wh = wh;
    }
    string what () {
        return wh;
    }
};

class Task {
public:
    Task(long long , long long , long long, int);
    long long task_no, proc_count;
    long long submit_time, run_time;
    long long put_time, end_time;
    vector <int> proc_nos;
    void put(long long);
    long long flag;
};

Task::Task(long long task_no, long long submit_time, long long run_time, int proc_count) {
    this->task_no = task_no;
    this->submit_time = submit_time;
    this->run_time = run_time;
    this->proc_count = proc_count;
}

void Task::put(long long at) {
    this->put_time = at;
    this->end_time = at + this->run_time;
}


class Proc {
    friend class Task;
public:
    Proc(int);
    int proc_no;
    vector <Task> local_tasks;
    void print_local_tasks();
    long long when_empty();

};

Proc::Proc(int proc_no) {
    this->proc_no = proc_no;
}


long long Proc::when_empty() {
    if(this->local_tasks.empty()) return 0;
    Task last_task = this->local_tasks.back();
    return last_task.run_time + last_task.put_time;
}


void Proc::print_local_tasks() {
    cout << "Printing local tasks for proc no " << this->proc_no << endl;
    for(vector <Task>::iterator it = this->local_tasks.begin(); it != this->local_tasks.end(); ++it) {
        cout << "Task no " << it->task_no << ", start: " << it->put_time << ", end: " << it->run_time + it->put_time << endl;
    }
    cout << endl;
}


struct min_ending_time {
    inline bool operator () (const Task & task1, const Task & task2) {
        return task1.end_time < task2.end_time;
    }
};

struct min_run_time {
    inline bool operator () (const Task & task1, const Task & task2) {
        return task1.run_time < task2.run_time;
    }
};

struct max_run_time {
    inline bool operator () (const Task & task1, const Task & task2) {
        return task1.run_time > task2.run_time;
    }
};

struct max_proc_count {
    inline bool operator () (const Task & task1, const Task & task2) {
        return task1.proc_count > task2.proc_count;
    }
};

struct min_proc_count {
    inline bool operator () (const Task & task1, const Task & task2) {
        return task1.proc_count < task2.proc_count;
    }
};

struct min_task_no {
    inline bool operator () (const Task & task1, const Task & task2) {
        return task1.task_no < task2.task_no;
    }
};

struct min_flag {
    inline bool operator () (const Task & task1, const Task & task2) {
        return task1.flag < task2.flag;
    }
};

struct min_submit_time {
    inline bool operator () (const Task & task1, const Task & task2) {
        return task1.submit_time < task2.submit_time;
    }
};


