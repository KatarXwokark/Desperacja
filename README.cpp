#include "Task.h" // dodalem nową zmienną do klasy Task - flag i struktury, które wyznaczają element z mniejszą flagą i submit_time


using namespace std;

long long Max_procs, Max_nodes, Max_jobs, Current_time, Total_flow_time = 0;
long long N, licz = 0;
int Pocz;
deque <Proc> Procs;
deque <Proc> Available_procs;

deque <Task> Tasks;
deque <Task> Current_tasks;
deque <Task> Waiting_tasks;
deque <Task> Finished_tasks;
deque <long long> Start_time_queue;
deque <Task> Best_tasks;
deque <Task> Previous_tasks;


deque <Task> get_dataset(string file_string, int flag) {
    string a,b,c,d,e;
    string dummy_str;
    deque <Task> tasks;
    ifstream input_file;
    input_file.open(file_string);
    if(!input_file) {
        cout << "Error, no file found";
    }
    else {
        if(input_file.good()) {
            cout << "Creating dataset...\n";
            if(flag == 0) {
            	cout << "TaskNo\tSubmitTime\tRunTime\tProcessors\n";	
            }
        }
        while(input_file >> a){
            if(a.compare("MaxProcs:") == 0) {
                input_file >> a;
                Max_procs = stoll(a);
                input_file.seekg(0, ios::beg);
                break;
            }
        }
        while(input_file >> a){
            if(a.compare("MaxNodes:") == 0) {
                input_file >> a;
                Max_nodes = stoll(a);
                input_file.seekg(0, ios::beg);
                break;
            }
        }
        if(Max_nodes > Max_procs) {
            Max_procs = Max_nodes;
        }
        while(input_file >> a){
            if(a.compare("MaxJobs:") == 0) {
                input_file >> a;
                Max_jobs = stoll(a);
                input_file.seekg(0, ios::beg);
                break;
            }
        }
        if(N > Max_jobs) {
            N = Max_jobs;
        }
        while(!input_file.eof() && N > 0) {
            if(input_file >> a && (a.compare(";") == 0 || a.at(0) == 59)) {
                getline(input_file, dummy_str);
            }
            else {
                input_file >> b >> c >> d >> e;
                getline(input_file, dummy_str);
                if(stoll(a) > 0 && stoll(b) >= 0 && stoll(d) > 0 && stoll(e) > 0 && N > 0) {
                    if(!tasks.empty() && stoll(a) == tasks.back().task_no) continue;
                    Task task(stoll(a), stoll(b), stoll(d), stoi(e));
                    tasks.push_back(task);
                    if(flag == 0) {
                    	cout << task.task_no << " " <<
                         	task.submit_time << " " <<
                         	task.run_time << " " <<
                         	task.proc_count <<  endl;	
                    }
                    N--;
                }
            }
        }
    }
    cout << "\nMaxjobs = " << Max_jobs << endl;
    cout << "Maxprocs = " << Max_procs << endl;
    cout << "Dataset created\n";
    input_file.close();
    return tasks;
}

void export_to_file(deque <Task> tasks) {
    sort(tasks.begin(), tasks.end(), min_task_no());
    ofstream output;
    output.open("output.txt");
    if(output.good()) {
        while(!tasks.empty()) {
            Task task = tasks.front();
            tasks.pop_front();

            output << task.task_no << " "
                   << task.put_time << " "
                   << task.end_time << " ";
            for(int i = 0; i < task.proc_nos.size(); i++) {
                output << task.proc_nos[i] << " ";
            }
            output << endl;
        }
    }
    output.close();
    cout << "File successfully created" << endl;

}

deque <Proc> create_processors() {
    int p = 0;
    deque <Proc> procs;
    while(p < Max_procs) {
        procs.push_back(Proc(p));
        p++;
    }
    return procs;
}

void print_tasks(deque <Task> tasks) {
    cout << "Printing tasks: " << endl;
    for(deque <Task>::iterator it = tasks.begin(); it != tasks.end(); ++it) {
        cout << "Task no " << it->task_no << " ,submit " << it->submit_time << ", proc_needed " << it->proc_count
                << " ,start " << it->put_time<< " ,stop: " << it->end_time
             << ". Processors: ";
        for(int i = 0; i < (*it).proc_nos.size(); i++) {
            cout << (*it).proc_nos[i] << " | ";
        }
        cout << endl;
    }
}

void print_procs(deque <Proc> proc) {
    cout << "Printing processors: " << endl;
    for(deque <Proc>::iterator it = proc.begin(); it != proc.end(); ++it) {
        cout << "Proc no " << it->proc_no << " ,will be empty at" << it->when_empty() << endl;
    }
}



void create_submit_queue() {
    for(int i = 0; i < Tasks.size(); i++) {
        if(find(Start_time_queue.begin(), Start_time_queue.end(), Tasks[i].submit_time) == Start_time_queue.end()) {
            Start_time_queue.push_back(Tasks[i].submit_time);
        }
    }
}

long long move_time_ahead() {
    long long next_submit, next_task_ending, min;
    int i = 0;
    if(!Start_time_queue.empty()) {
        while(Start_time_queue[i] <= Current_time && i < Start_time_queue.size() && Start_time_queue.size() != 1) {
            i++;
        }
        next_submit = Start_time_queue[i];
    }
    else {
        next_submit = -1;
    }

    if(Current_tasks.empty() && Waiting_tasks.empty()) {
        min = next_submit;
        if(!Start_time_queue.empty()) {
            while(i >= 0) {
                Start_time_queue.pop_front();
                i--;
            }
        }
        return min;
    }
    else {
        if(Current_tasks.size() == 1 && Start_time_queue.size() == 0) return Current_tasks[0].end_time;
        Task next_task = *(min_element(Current_tasks.begin(), Current_tasks.end(), min_ending_time()));
        next_task_ending = next_task.end_time;
        if(next_task_ending < next_submit || next_submit <= 0) {
            min = next_task_ending;
        }
        else {
            min = next_submit;
            if(!Start_time_queue.empty()) {
                while(i >= 0) {
                    Start_time_queue.pop_front();
                    i--;
                }
            }
        }
    }
    return min;
}

void remove_tasks () {
    if(!Current_tasks.empty()) {
        for(int i = 0; i < Current_tasks.size();) {
            if(Current_tasks[i].end_time <= Current_time) {
                Finished_tasks.push_back(Current_tasks[i]);
                Current_tasks.erase(Current_tasks.begin() + i);
            }
            else {
                i++;
            }
        }
    }
}

void put_candidates (vector <Task> tasks_to_put) {

    if(!tasks_to_put.empty()) {
        for(int i = 0; i < tasks_to_put.size(); i++) {
            Task task = tasks_to_put[i];
            task.put(Current_time);
            for(int j = 0; j < task.proc_count; j++) {
                if(Available_procs.empty()) throw PutError("No available procs in put_candidates");
                task.proc_nos.push_back(Available_procs.front().proc_no);
                Available_procs.pop_front();
            }
            Current_tasks.push_back(task);
            Waiting_tasks.erase(std::find_if(Waiting_tasks.begin(), Waiting_tasks.end(),
                                     [&] (Task const & t) { return t.task_no == task.task_no; }));
        }
    }

}

void get_processors() {
    if(Current_tasks.empty()) {
        Available_procs = Procs;
        return;
    }
    long long at = Current_time;
    deque <int> non_available_procs_nos;
    deque <Proc> local_available_processors;
    int * procs_non_available = new int [Procs.size()];

    for(int i = 0; i < Current_tasks.size(); i++) {
        Task task = Current_tasks[i];
        for(int j = 0; j < task.proc_nos.size(); j++) {
            procs_non_available[task.proc_nos[j]] = 1;
        }
    }
    for(int k = 0; k < Procs.size(); k++) {
        if(procs_non_available[k] != 1) {
            local_available_processors.push_back(Procs[k]);
        }
    }
    delete [] procs_non_available;
    Available_procs = local_available_processors;
}

void get_candidates() {
    long long at = Current_time;
    while(!Tasks.empty() && Tasks[0].submit_time <= at) {
        Waiting_tasks.push_back(Tasks[0]);
        Tasks.erase(Tasks.begin());
    }

}

vector <Task> spt () {
    int available_procs = Available_procs.size();
    long long at = Current_time;
    long long task_i = 0;
    vector <Task> local_candidates;
    sort(Waiting_tasks.begin(), Waiting_tasks.end(), min_run_time());
    while(at >= Waiting_tasks[task_i].submit_time && available_procs > 0 && task_i < Waiting_tasks.size()) {
        if(available_procs - Waiting_tasks[task_i].proc_count >= 0) {
        	Waiting_tasks[task_i].flag = licz;
        	licz++;
            local_candidates.push_back(Waiting_tasks[task_i]);
            available_procs -= Waiting_tasks[task_i].proc_count;
        }
        task_i++;
    }
    return local_candidates;
}



vector <Task> set_candidates() {
    int available_procs = Available_procs.size();
    vector <Task> local_candidates;
    if(Waiting_tasks.empty()) return local_candidates;
    if(Waiting_tasks.size() == 1 && Waiting_tasks[0].proc_count <= available_procs) {
    	Waiting_tasks[0].flag = licz;
    	licz++;
        local_candidates.push_back(Waiting_tasks[0]);
        return local_candidates;
    }
    else local_candidates = spt();
    return local_candidates;

}

double calculate_avg_flow_time (deque <Task> counted) {
	long long flow_time = 0;
    for(int i = 0; i < counted.size() ; i++) {
        flow_time += counted[i].end_time;
    }
    return (flow_time);
};

double calculate_opt_flow_time () {
    long long opt_flow = 0;
    for(int i = 0; i < Best_tasks.size() ; i++) {
        opt_flow += (Best_tasks[i].submit_time + Best_tasks[i].run_time);
    }
    return (opt_flow);
};

double calculate_error(double avg_flow_time, double best_flow_time) {
    return ( (avg_flow_time - best_flow_time) / best_flow_time) * 100.00;
}

//nowe funkcyjki

void rand_swap() {
	int u = 5;
	long long i = rand() % Tasks.size(), j = rand() % Tasks.size();
	while(Tasks[i].put_time < Tasks[j].submit_time || i == j){	
		j = rand() % Tasks.size();
		u--;
		if(u == 0){
			i = rand() % Tasks.size();
			u = 5;
		}
	}
	swap(Tasks[i].flag, Tasks[j].flag);
}

void Clean() {
	for(int i = 0; i < Tasks.size(); i++){
		Tasks[i].proc_nos = {};
	}
}

vector <Task> fifo () {
    int available_procs = Available_procs.size();
    long long at = Current_time;
    long long task_i = 0;
    vector <Task> local_candidates;
    sort(Waiting_tasks.begin(), Waiting_tasks.end(), min_flag());
    while(at >= Waiting_tasks[task_i].submit_time && available_procs > 0 && task_i < Waiting_tasks.size()) {
        if(available_procs - Waiting_tasks[task_i].proc_count >= 0) {
            local_candidates.push_back(Waiting_tasks[task_i]);
            available_procs -= Waiting_tasks[task_i].proc_count;
        }
        task_i++;
    }
    return local_candidates;
}

vector <Task> set_candidates_other() {
    int available_procs = Available_procs.size();
    vector <Task> local_candidates;
    if(Waiting_tasks.empty()) return local_candidates;
    if(Waiting_tasks.size() == 1 && Waiting_tasks[0].proc_count <= available_procs) {
        local_candidates.push_back(Waiting_tasks[0]);
        return local_candidates;
    }
    else local_candidates = fifo();
    return local_candidates;

}


int main(int argv, char * argc []) {
    srand((unsigned int) time(NULL));

    string file_string = argc[1];
    N = stoi(argc[2]);
    Pocz = stoi(argc[3]);
    Tasks = get_dataset(file_string, stoi(argc[4]));
    Procs = create_processors();
    create_submit_queue();
    unsigned long tasks_size_at_entrance = Tasks.size();

    clock_t start, stop, b, e, b_1, e_1, b_2, e_2, b_3, e_3, b_4, e_4, b_5, e_5;
    start = clock();
    double duration_rm = 0;
    double duration_get_available = 0;
    double duration_get_candidates = 0;
    double duration_set_candidates = 0;
    double duration_pt_tasks = 0;
    double duration_move = 0;
    while((!Tasks.empty() || !Waiting_tasks.empty() || !Current_tasks.empty()) && Current_time >= 0) { // tu jest tworzone rozwiązanie z SPT

        b = clock();
        remove_tasks();
        e = clock();
        duration_rm += e - b;

        b_1 = clock();
        get_processors();
        e_1 = clock();
        duration_get_available += e_1 - b_1;

        b_2 = clock();
        get_candidates();
        e_2 = clock();
        duration_get_candidates += e_2 - b_2;

        b_3 = clock();
        vector <Task> tasks_to_put = set_candidates();
        e_3 = clock();
        duration_set_candidates += e_3 - b_3;

        try {
            b_4 = clock();
            put_candidates(tasks_to_put);
            e_4 = clock();
            duration_pt_tasks += e_4 - b_4;
        }
        catch (PutError & e) {
            cout << e.what() << endl;
        }

        b_5 = clock();
        Current_time = move_time_ahead();
        e_5 = clock();
        duration_move += e_5 - b_5;
        
    }
    double T = Pocz;
    Best_tasks = Finished_tasks;
    Previous_tasks = Finished_tasks;
    while(T > 1 && (clock() - start) / (double) CLOCKS_PER_SEC < 300){ //a tu zaczyna się SA
    	Tasks = Previous_tasks;
    	sort(Tasks.begin(), Tasks.end(), min_submit_time());
        Clean();
    	Finished_tasks = {};
    	rand_swap();
    	Current_time = 0;
    	Start_time_queue = {};
        create_submit_queue();
    	while((!Tasks.empty() || !Waiting_tasks.empty() || !Current_tasks.empty()) && Current_time >= 0) {
			
    	    b = clock();
        	remove_tasks();
        	e = clock();
        	duration_rm += e - b;
	
    	    b_1 = clock();
        	get_processors();
        	e_1 = clock();
        	duration_get_available += e_1 - b_1;

        	b_2 = clock();
        	get_candidates();
        	e_2 = clock();
        	duration_get_candidates += e_2 - b_2;
	
    	    b_3 = clock();
        	vector <Task> tasks_to_put = set_candidates_other();
        	e_3 = clock();
        	duration_set_candidates += e_3 - b_3;
	
    	    try {
        	    b_4 = clock();
            	put_candidates(tasks_to_put);
            	e_4 = clock();
            	duration_pt_tasks += e_4 - b_4;
        	}
        	catch (PutError & e) {
            	cout << e.what() << endl;
        	}
			
    	    b_5 = clock();
        	Current_time = move_time_ahead();
        	e_5 = clock();
        	duration_move += e_5 - b_5;
			
		    //cout << Tasks.empty() << " " << Waiting_tasks.empty() << " " << Current_tasks.empty() << " " << Start_time_queue.empty() << endl;
    	}
    	double Previous_resault = calculate_avg_flow_time (Previous_tasks);
		double This_resault = calculate_avg_flow_time (Finished_tasks);
		double Best_resault = calculate_avg_flow_time (Best_tasks);
		cout << Previous_resault << " " << This_resault << " " << Best_resault << endl;
		if(This_resault < Previous_resault) {
			cout << "TAK" << endl;
			Previous_tasks = Finished_tasks;
			if(This_resault < Best_resault) Best_tasks = Finished_tasks;
		}
		else if(This_resault > Previous_resault){
			cout << "NIE";
			double prop = exp((Previous_resault - This_resault)/1000000000*T) * 10000;
			double decider = rand() % 10000;
			if(prop > decider){
				T *= 0.9;
				cout << " TAK" << endl;				
				Previous_tasks = Finished_tasks;
			}
			else cout << " NIE" << endl;
		}
		stop = clock();
	}
    stop = clock();
    

    //printing finished tasks in console
    if(stoi(argc[4]) == 0) {
        //print_tasks(Best_tasks);
        double avg_flow_time = calculate_avg_flow_time(Best_tasks);
    	double best_avg_flow_time = calculate_opt_flow_time();
    	double error_perc = calculate_error(avg_flow_time, best_avg_flow_time);
    	if (tasks_size_at_entrance == Best_tasks.size() && Max_jobs > 0) {
	        cout << "------------------------------------------------------\n";
	        cout << "Calculated best flow: " << best_avg_flow_time << endl;
	        cout << "Average flow: " << avg_flow_time << endl;
	        cout << setprecision(4) << error_perc << "% away from lower bound with " << Pocz << endl;
	        cout << "Total program run time: " << (stop - start) / (double) CLOCKS_PER_SEC << endl;
	        cout << "Duration remove:  " << duration_rm/ (double) CLOCKS_PER_SEC << endl;
	        cout << "Duration get_procs: " << duration_get_available/ (double) CLOCKS_PER_SEC  << endl;
	        cout << "Duration get candidates: " << duration_get_candidates/ (double) CLOCKS_PER_SEC  << endl;
	        cout << "Duration set candidates: " << duration_set_candidates/ (double) CLOCKS_PER_SEC  << endl;
	        cout << "Duration put tasks " << duration_pt_tasks/ (double) CLOCKS_PER_SEC  << endl;
	        cout << "Duration move time " << duration_move/ (double) CLOCKS_PER_SEC  << endl;
    	}
    }

    if (tasks_size_at_entrance == Best_tasks.size() && Max_jobs > 0) {
    	cout << "Program finished successfully. Check output.txt for results" << endl;
    	export_to_file(Best_tasks);
    }

    return 0;

}
