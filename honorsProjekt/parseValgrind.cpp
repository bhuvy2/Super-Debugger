#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;

// Compile with: g++ -std=c++0x -w parseValgrind.cpp -o parseValgrind

//map<string, pair<string, int> > errors;
//pair<string, int> lineNum; 

struct errors {
	string errorMessage;
	vector< tuple<string, string, int> > description;
};

vector<string> memory;
vector<errors> totalErrors;
tuple<string, string, int> desLines;
vector<errors> dividedErrors[9];

string listOfErrors[] = {"Segfault", "Invalid Read", "Invalid Write", "Invalid Free", "Mismatched Free ~ Always use malloc with free and new with delete", "Source and Destination Overwrite Each Other", "Conditional Jump", "A System Call Parameter is Uninitialised", "Your Function is Uninitialized, Value Might Be Too Big or Negative"}; 

/*
0 - SIGSEGV
1 - Invalid read
2 - Invalid write
3 - Invalid free
4 - Mismatched free
5 - Source and destination		--- 	can handle right away
6 - Conditional jump
7 - Syscall						---		can handle right away	
8 - Argument fishy 				---		can handle right away
*/

void printMem() {
	for (int i = 0; i < memory.size(); ++i)
		cout << memory[i] << "\n";
}

void printDes(tuple<string, string, int> t) { 
	cout << "    Function: " << get<0>(t) << "    File: " << get<1>(t) << "    Line#: " << get<2>(t) << "\n"; 
}

void printErrors(errors t) {
	cout << t.errorMessage << "\n";
	for (auto it = (t.description).begin(); it != (t.description).end(); ++it)
		printDes(*it);
	cout << "\n";
}

void printCategory() {
	for (int i = 0; i < 9; ++i) {
		if (dividedErrors[i].size()) {
			cout << "--------  " << listOfErrors[i] << "  -------- \n";
			for (int j = 0; j < dividedErrors[i].size(); ++j)
				printErrors(dividedErrors[i][j]);
				
			cout << "\n";
		}
	}
}

void parse() {
	vector<string> errorLines;
	string empty = "";
	string funct;
	string file;
	int lineNum;
	errorLines.push_back(empty);
	int end = 0;
	int place;

	ifstream parseMe("1234321.txt");		// got from online - cppreference
	if (parseMe.is_open()) {
		string line;
		
		for (int i = 0; i < 6; ++i)
			getline(parseMe, line);
		
		while (getline(parseMe, line)) {
			line.erase(0, line.find(" ") + 1);

			if (!line.find("HEAP SUMMARY")) 
				place = end;
			
			errorLines.push_back(line);
			++end;
		}
		
		parseMe.close();
	}

	int i = 0;
	while (i != place) {
		if (errorLines[i].empty()) {
			errors temp;
			while (errorLines[i].empty())
				++i;
			
			if (i >= place)
				break;
				
			temp.errorMessage = errorLines[i];
			++i;
			
			while (!errorLines[i].empty()) {
				int k = errorLines[i].find(":");
				if (k != -1) {
					errorLines[i].erase(0, k + 2);
					if (errorLines[i][0] != '_') {
						int w = errorLines[i].find(" ");
						funct = errorLines[i].substr(0, w);			
						errorLines[i].erase(0, w + 2);
						
						int c = errorLines[i].find(":");
						file = errorLines[i].substr(0, c);
						errorLines[i].erase(0, c + 1);
						//cout << funct << " " << file << "\n";
						if (errorLines[i][0] >= '0' && errorLines[i][0] <= '9') {
							int lineNum = stoi(errorLines[i].substr(0, errorLines[i].length() - 1));
					
							//cout << funct << " " << file << " " << lineNum << "\n";
							desLines = make_tuple(funct, file, lineNum);
							(temp.description).push_back(desLines);
						}
						else {
							desLines = make_tuple(funct, "C Lib Call", -1);
							(temp.description).push_back(desLines);
						}
					}
				}
				
				++i;
			}
			
			totalErrors.push_back(temp);
		}
	}
	
	for (; i < errorLines.size(); ++i) {
		if (errorLines[i].find("LEAK SUMMARY") != -1) {
			while (i < errorLines.size()) {
				if (errorLines[i].find("Rerun with") != -1) 
					i = errorLines.size();
				else memory.push_back(errorLines[i]);
				
				++i;
			}
		}
		else {
			memory.push_back(errorLines[i]);
			
			if (errorLines[i].find("All heap blocks") != -1) 
				i = errorLines.size();
		}
	}	

	// print stuff
/*	
	for (int i = 0; i < totalErrors.size(); ++i) 
		printErrors(totalErrors[i]);	
		
	printMem();
*/	 
}

void examine() {
	for (int i = 0; i < totalErrors.size(); ++i) {
		if (totalErrors[i].errorMessage.find("(SIGSEGV)") != -1)
			dividedErrors[0].push_back(totalErrors[i]);
		else if (!totalErrors[i].errorMessage.find("Invalid read"))
			dividedErrors[1].push_back(totalErrors[i]);
		else if (!totalErrors[i].errorMessage.find("Invalid write"))
			dividedErrors[2].push_back(totalErrors[i]);
		else if (!totalErrors[i].errorMessage.find("Invalid free"))
			dividedErrors[3].push_back(totalErrors[i]);
		else if (!totalErrors[i].errorMessage.find("Mismatched free"))
			dividedErrors[4].push_back(totalErrors[i]);
		else if (!totalErrors[i].errorMessage.find("Source and destination"))
			dividedErrors[5].push_back(totalErrors[i]);
		else if (totalErrors[i].errorMessage.find("has a fishy") != -1)
			dividedErrors[8].push_back(totalErrors[i]);
		else if (!totalErrors[i].errorMessage.find("Conditional jump"))
			dividedErrors[6].push_back(totalErrors[i]);
		else if (!totalErrors[i].errorMessage.find("Syscall param"))
			dividedErrors[7].push_back(totalErrors[i]);
	}
	
	// add print stuff here
	//printCategory();
}

int main(int argc, char* argv[]) {
	pid_t parent = fork();
	
	if (parent == -1)
		exit(420);
	
	if (parent) {
		int status;
		int k = waitpid(parent, &status, 0);
		
		if (k == -1)
			exit(666);
			
		if (WIFEXITED(status) && (WEXITSTATUS(status) == 420 || WEXITSTATUS(status) == 1337)) 
			exit(666);	
			
		// now parse the text file
		
		parse();
		
		// now do stuff  ---  vector should be filled by now
		
		// break up errors (if any) to different categories
		examine();
		
	}
	else {
		char* valgrind_args[argc + 2];
		valgrind_args[0] = "valgrind";
		valgrind_args[1] = "--log-file=1234321.txt";
		
		for (int i = 1; i < argc; ++i)
			valgrind_args[i + 1] = argv[i];
		valgrind_args[argc + 1] = NULL;
		
		int k = execvp(valgrind_args[0], valgrind_args);

		if (k == -1)
			exit(1337);
	}
	
	return 0;
}
