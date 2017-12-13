#include "Task.h"


using namespace std;

long long Max_procs, Max_nodes, Max_jobs, Current_time, Total_flow_time = 0;
long long N;
int Pocz, Kon;
string Heurs [] = {"FIFO", "SPT", "LPT", "LPU", "MPU", "RND"};
deque <Proc> Procs;
deque <Proc> Available_procs;

vector <Task> Tasks;
deque <Task> Current_tasks;
deque <Task> Waiting_tasks;
deque <Task> Finished_tasks;
deque <long long> Start_time_queue;
vector <vector <Task>> F_matrix;
Task empty_task = Task(-1, -1, -1, -1);



vector <Task> get_dataset(string file_string, int flag) {
    string a,b,c,d,e;
    string dummy_str;
    vector <Task> tasks;
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
        local_candidates.push_back(Waiting_tasks[0]);
        return local_candidates;
    }
    else {
        local_candidates = spt();
    }
    return local_candidates;

}

double calculate_avg_flow_time (deque <Task> tasks) {
    for(int i = 0; i < tasks.size() ; i++) {
        Total_flow_time += tasks[i].end_time;
    }
    return (Total_flow_time);
};

double calculate_opt_flow_time () {
    long long opt_flow = 0;
    for(int i = 0; i < Finished_tasks.size() ; i++) {
        opt_flow += (Finished_tasks[i].submit_time + Finished_tasks[i].run_time);
    }
    return (opt_flow);
};

double calculate_error(double avg_flow_time, double best_flow_time) {
    return ( (avg_flow_time - best_flow_time) / best_flow_time) * 100.00;
}

vector <vector <Task>> Swap_tasks(){ // to cała misterna machineria przezucania zadan na koniec i prawidłowego sciągania zadan powyżej dawnego miejsca zadania przezuconego
	int l = 0;
	vector <vector <Task>> temp = F_matrix;
	long long int i = rand() % Finished_tasks.size(), range = F_matrix[0].size(), counter = 0, j, block;
	vector <int> Remaining_procs_total = Finished_tasks[i].proc_nos, Remaining_procs_temp;
	for(j = range - 1; j >= 0; j--){
		for(int k = 0; k < Max_procs; k++){
			if(temp[j][k].task_no == -1) counter++; 
		}
		if(counter < Finished_tasks[i].proc_count) break;
		counter = 0;
	}
	block = ++j;
	if(block + Finished_tasks[i].run_time > temp[0].size()){
		for(int k = 0; k < Max_procs; k++){
			while(temp[k].size() < block + Finished_tasks[i].run_time) temp[k].push_back(empty_task);
		}
	}
	for(int k = 0; l < Finished_tasks[i].proc_count; k++){
		if(temp[k][block].task_no == -1){
			for(j = block; j <= block + Finished_tasks[i].run_time; j++){
				temp[k][j] = Finished_tasks[i];
			}
		}
		l++;
	}
	l = 0;
	for(int k = Finished_tasks[i].put_time; k <= Finished_tasks[i].end_time; k++){
		for(l = 0; l < Finished_tasks[i].proc_count; l++){
			temp[Finished_tasks[i].proc_nos[l]][k] = empty_task; 
		}
	}
	cout << "5" << endl;
	counter = 0;
	j = Finished_tasks[i].put_time;
	while(j < temp[0].size() && !Remaining_procs_total.empty()){
		Remaining_procs_temp = Remaining_procs_total;
		for(int k = 0; k < Remaining_procs_temp.size(); k++){
			if(temp[Remaining_procs_temp[k]][j].task_no == -1){
				while(temp[Remaining_procs_temp[k]][j + counter].task_no == -1) counter++;
				for(l = 0; l < temp[Remaining_procs_temp[k]][j + counter].proc_count; l++){
					if(find(Finished_tasks[i].proc_nos.begin(), Finished_tasks[i].proc_nos.end(), temp[Remaining_procs_temp[k]][j + counter].proc_nos[l]) == Finished_tasks[i].proc_nos.end()) break;
				}
				cout << "6" << endl;
				if(l != Remaining_procs_temp.size()){
					for(int o = 0; o < temp[Remaining_procs_temp[k]][j + counter].proc_count; o++){
						for(int p = 0; p < Finished_tasks[i].proc_count; p++){
							if(temp[Remaining_procs_temp[k]][j + counter].proc_nos[o] == Finished_tasks[i].proc_nos[p]){
								swap(Remaining_procs_total[k],Remaining_procs_total[Remaining_procs_total.size() - 1]);
								Remaining_procs_total.pop_back();
								swap(Remaining_procs_temp[k],Remaining_procs_temp[Remaining_procs_temp.size() - 1]);
								Remaining_procs_temp.pop_back();
								break;
							}
						}
					}
				}
				else{
					int o, p;
					for(o = j + counter - 1; o >= temp[Remaining_procs_temp[k]][j + counter].submit_time; o--){
						for(p = 0; p < temp[Remaining_procs_temp[k]][j + counter].proc_nos.size(); p++){
							if(temp[temp[Remaining_procs_temp[k]][j + counter].proc_nos[p]][o].task_no != -1) break;
						}
						if(p != temp[Remaining_procs_temp[k]][j + counter].proc_nos.size()) break;
					}
					o++;
					Task hold = temp[Remaining_procs_temp[k]][j + counter];
					for(p = 0; p < hold.proc_nos.size(); p++){
						for(int r = 0; r < hold.run_time; r++){
							swap(temp[hold.proc_nos[p]][j + counter + r], temp[hold.proc_nos[p]][o + r]);
						}
					}
					for(o = 0; o < hold.proc_count; o++){
						for(p = 0; p < Finished_tasks[i].proc_count; p++){
							if(temp[Remaining_procs_temp[k]][j + counter].proc_nos[o] == Finished_tasks[i].proc_nos[p]){
								swap(Remaining_procs_temp[k],Remaining_procs_temp[Remaining_procs_temp.size() - 1]);
								Remaining_procs_temp.pop_back();
								break;
							}
						}
					}
				}
			}
		}
	}
	return temp;
}

vector <vector <Task>> Code(){ //to miało zmieniać vector Task'ów na vector vector'ów Task'ów, by mieć je zapisane w stylu "ten procesor w tym momencie ma takie zadanie (nie przepisuje samych identyfikatorów by przenosić wszystkie informacje)
	vector <vector <Task>> temp;
	temp.resize(Max_procs);
	cout << "1" << endl;
	for(int i = 0; i < Finished_tasks.size(); i++){
		for(int j = 0; j < Finished_tasks[i].proc_nos.size(); j++){
			long long int old = temp[Finished_tasks[i].proc_nos[j]].size();
			while(temp[Finished_tasks[i].proc_nos[j]].size() < Finished_tasks[i].end_time) temp[Finished_tasks[i].proc_nos[j]].push_back(empty_task);
			for(long long int k = old; k < temp[j].size(); k++){
				temp[Finished_tasks[i].proc_nos[j]][k] = empty_task;
			}
			for(long long int k = Finished_tasks[i].put_time; k <= Finished_tasks[i].end_time; k++){
				temp[Finished_tasks[i].proc_nos[j]][k] = Finished_tasks[i];
			}
		}
	}
	cout << "2" << endl;
	long long int maxi = 0;
	for(int i = 0; i < Max_procs; i++){
		if(maxi < temp[i].size()) maxi = temp[i].size();
	}
	for(int i = 0; i < Max_procs; i++){
		while(temp[i].size() < maxi) temp[i].push_back(empty_task);
	}
	cout << "3" << endl;
	return temp;
}

deque <Task> Decode (vector <vector <Task>> code){ //a to dekoduje w drugą stronę
	cout << "7" << endl;
	deque <Task> temp;
	for(int i = 0; i < Max_procs; i++){
		for(int j = 0; j < code[0].size(); j++){
			if(code[i][j].task_no != -1){
				Task Hold = Task(-1, -1, -1, -1);
				Hold.task_no = code[i][j].task_no;
				Hold.proc_count = code[i][j].proc_count;
				Hold.submit_time = code[i][j].submit_time;
				Hold.run_time = code[i][j].run_time;
				Hold.put_time = j;
				Hold.end_time = j + Hold.run_time - 1;
				for(int k = i; k < Max_procs; k++){
					if(code[k][j].task_no == Hold.task_no) Hold.proc_nos.push_back(k);
				}
				temp.push_back(Hold);
				for(int k = 0; k < Hold.proc_count; k++){
					for(int l = Hold.put_time; l <= Hold.end_time; l++){
						code[k][l] = empty_task;
					}
				}
			}
		}
	}
	return temp;
}

void cos(){  //tu się zaczyna przeróbka
	F_matrix = Code();
	deque <Task> Kopia;
	vector <vector <Task>> Copy = F_matrix;
	double T, prop_cal, prop_temp, flow_f, flow_k;
	for(T = Pocz; T > Kon; T *= 0.9){
		flow_f = calculate_avg_flow_time(Finished_tasks);
		cout << "4" << endl;
		Copy = Swap_tasks();
		Kopia = Decode(Copy);
		flow_k = calculate_avg_flow_time(Kopia);
		if(flow_f > flow_k){
			F_matrix = Copy;
			cout << "8" << endl;
			Finished_tasks = Decode(F_matrix);
		}
		else{
			prop_cal = exp((flow_f-flow_k)/T) * 100000000;
			prop_temp = rand() % 100000000;
			if(prop_cal > prop_temp)
				F_matrix = Copy;
				cout << "9" << endl;
				Finished_tasks = Decode(F_matrix);
		}
	}
}


int main(int argv, char * argc []) {
    srand((unsigned int) time(NULL));

    string file_string = argc[1];
    N = stoi(argc[2]);
    Pocz = stoi(argc[3]);
    Kon = stoi(argc[4]);
    Tasks = get_dataset(file_string, stoi(argc[5]));
    Procs = create_processors();
    create_submit_queue();
    unsigned long tasks_size_at_entrance = Tasks.size();

    clock_t start, stop, b, e, b_1, e_1, b_2, e_2, b_3, e_3, b_4, e_4, b_5, e_5, b_6, e_6;
    start = clock();
    double duration_rm = 0;
    double duration_get_available = 0;
    double duration_get_candidates = 0;
    double duration_set_candidates = 0;
    double duration_pt_tasks = 0;
    double duration_move = 0;
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

	b_6 = clock();
	cos();
    e_6 = clock();
    double duration_swap = e_6 - b_6;
    
    stop = clock();

    //printing finished tasks in console
    if(stoi(argc[5]) == 0) {
        //print_tasks(Finished_tasks);
        double avg_flow_time = calculate_avg_flow_time(Finished_tasks);
    	double best_avg_flow_time = calculate_opt_flow_time();
    	double error_perc = calculate_error(avg_flow_time, best_avg_flow_time);
    	if (tasks_size_at_entrance == Finished_tasks.size() && Max_jobs > 0) {
	        cout << "------------------------------------------------------\n";
	        cout << "Calculated best flow: " << best_avg_flow_time << endl;
	        cout << "Average flow: " << avg_flow_time << endl;
	        cout << setprecision(4) << error_perc << "% away from lower bound with " << Pocz << "/" << Kon << endl;
	        cout << "Total program run time: " << (stop - start) / (double) CLOCKS_PER_SEC << endl;
	        cout << "Duration remove:  " << duration_rm/ (double) CLOCKS_PER_SEC << endl;
	        cout << "Duration get_procs: " << duration_get_available/ (double) CLOCKS_PER_SEC  << endl;
	        cout << "Duration get candidates: " << duration_get_candidates/ (double) CLOCKS_PER_SEC  << endl;
	        cout << "Duration set candidates: " << duration_set_candidates/ (double) CLOCKS_PER_SEC  << endl;
	        cout << "Duration put tasks " << duration_pt_tasks/ (double) CLOCKS_PER_SEC  << endl;
	        cout << "Duration move time " << duration_move/ (double) CLOCKS_PER_SEC  << endl;
    	}
    }

    if (tasks_size_at_entrance == Finished_tasks.size() && Max_jobs > 0) {
    	cout << "Program finished successfully. Check output.txt for results" << endl;
    	export_to_file(Finished_tasks);
    }

    return 0;

}
