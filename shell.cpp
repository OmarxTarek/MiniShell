#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <direct.h>
#include <windows.h>
#include <csignal>
#include <ctime>

using namespace std;

string previousDirectory = "";
vector<string> commandHistory;

void setColor(const string& color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (color == "red") SetConsoleTextAttribute(hConsole, 12);
    else if (color == "green") SetConsoleTextAttribute(hConsole, 10);
    else if (color == "yellow") SetConsoleTextAttribute(hConsole, 14);
    else if (color == "blue") SetConsoleTextAttribute(hConsole, 9);
    else if (color == "cyan") SetConsoleTextAttribute(hConsole, 11);
    else if (color == "magenta") SetConsoleTextAttribute(hConsole, 13);
    else SetConsoleTextAttribute(hConsole, 7);
}

void resetColor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7);
}

void signalHandler(int signal) {
    if (signal == SIGINT) {
        cout << "\n^C (Use 'exit' to quit the shell)\n";
        cout << "MiniShell> " << flush;
    }
}

void printBanner() {
    setColor("cyan");
    cout << R"(
        _       _ __ _          _ _ 
  /\/\ (_)_ __ (_) _\ |__   ___| | |
 /    \| | '_ \| \ \| '_ \ / _ \ | |
/ /\/\ \ | | | | |\ \ | | |  __/ | |
\/    \/_|_| |_|_\__/_| |_|\___|_|_|                                                                      
)" << endl;
    setColor("yellow");
    cout << "                by Omar Tarek" << endl;
    resetColor();
    cout << "    Type 'help' for commands.\n" << endl;
}

void showHelp() {
    setColor("green");
    cout << "\n=== Available Commands ===\n";
    resetColor();
    cout << "  ls [path]       - List directory contents\n";
    cout << "  pwd             - Print working directory\n";
    cout << "  cd <dir>        - Change directory (cd ~ for home, cd - for previous)\n";
    cout << "  mkdir <dir>     - Create a new directory\n";
    cout << "  rmdir <dir>     - Remove an empty directory\n";
    cout << "  rm <file>       - Delete a file\n";
    cout << "  touch <file>    - Create a new file\n";
    cout << "  cat <file>      - Display file contents (with line numbers)\n";
    cout << "  echo <text>     - Display a line of text\n";
    cout << "  date            - Display current date and time\n";
    cout << "  history         - Show command history\n";
    cout << "  env             - Show environment variables\n";
    cout << "  clear           - Clear the screen\n";
    cout << "  help            - Show this help message\n";
    cout << "  exit            - Exit the shell\n";
    
    setColor("yellow");
    cout << "\n=== Redirection ===\n";
    resetColor();
    cout << "  >   - Overwrite file (e.g., echo hello > file.txt)\n";
    cout << "  >>  - Append to file (e.g., echo world >> file.txt)\n" << endl;
}

void listDirectory(const string& path = "") {
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    
    char currentDir[MAX_PATH];
    if (path.empty()) {
        _getcwd(currentDir, MAX_PATH);
    } else {
        strcpy(currentDir, path.c_str());
    }
    
    setColor("cyan");
    cout << "\nDirectory: " << currentDir << "\n";
    cout << "----------------------------------------\n";
    resetColor();
    
    string searchPath = string(currentDir) + "\\*";
    hFind = FindFirstFile(searchPath.c_str(), &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        setColor("red");
        cerr << "Error: Cannot read directory\n";
        resetColor();
        return;
    }
    
    int fileCount = 0, dirCount = 0;
    
    do {
        string name = findData.cFileName;
        if (name != "." && name != "..") {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                setColor("blue");
                cout << "[DIR]  ";
                resetColor();
                cout << name << endl;
                dirCount++;
            } else {
                setColor("green");
                cout << "[FILE] ";
                resetColor();
                cout << name;

                LARGE_INTEGER fileSize;
                fileSize.LowPart = findData.nFileSizeLow;
                fileSize.HighPart = findData.nFileSizeHigh;
                double size = static_cast<double>(fileSize.QuadPart);
                
                cout << " (";
                if (size >= 1073741824) {
                    cout << fixed << setprecision(2) << (size / 1073741824) << " GB";
                } else if (size >= 1048576) {
                    cout << fixed << setprecision(2) << (size / 1048576) << " MB";
                } else if (size >= 1024) {
                    cout << fixed << setprecision(2) << (size / 1024) << " KB";
                } else {
                    cout << static_cast<long long>(size) << " bytes";
                }
                cout << ")" << endl;
                fileCount++;
            }
        }
    } while (FindNextFile(hFind, &findData) != 0);
    
    FindClose(hFind);
    
    setColor("yellow");
    cout << "\nTotal: " << dirCount << " directories, " << fileCount << " files\n";
    resetColor();
    cout << endl;
}

void printWorkingDirectory() {
    char currentDir[MAX_PATH];
    if (_getcwd(currentDir, MAX_PATH) != NULL) {
        setColor("cyan");
        cout << currentDir << endl;
        resetColor();
    } else {
        setColor("red");
        cerr << "Error: Cannot get current directory\n";
        resetColor();
    }
}

void changeDirectory(const string& path) {
    if (path.empty()) {
        setColor("red");
        cerr << "Error: Please specify a directory\n";
        resetColor();
        return;
    }
    
    char currentDir[MAX_PATH];
    _getcwd(currentDir, MAX_PATH);
    string current = currentDir;
    
    string targetDir;
    
    if (path == "-") {
        if (previousDirectory.empty()) {
            setColor("red");
            cerr << "Error: No previous directory\n";
            resetColor();
            return;
        }
        targetDir = previousDirectory;
    } else if (path == "~") {
        char* homeDir = getenv("USERPROFILE");
        if (homeDir != nullptr) {
            targetDir = homeDir;
        } else {
            setColor("red");
            cerr << "Error: Cannot find home directory\n";
            resetColor();
            return;
        }
    } else {
        targetDir = path;
    }
    
    if (_chdir(targetDir.c_str()) == 0) {
        previousDirectory = current;
        char newDir[MAX_PATH];
        _getcwd(newDir, MAX_PATH);
        setColor("green");
        cout << "Changed to: ";
        setColor("cyan");
        cout << newDir << endl;
        resetColor();
    } else {
        setColor("red");
        cerr << "Error: Cannot change to directory '" << targetDir << "'\n";
        resetColor();
    }
}

void makeDirectory(const string& dirName) {
    if (dirName.empty()) {
        setColor("red");
        cerr << "Error: Please specify a directory name\n";
        resetColor();
        return;
    }
    
    if (_mkdir(dirName.c_str()) == 0) {
        setColor("green");
        cout << "Directory created: " << dirName << endl;
        resetColor();
    } else {
        setColor("red");
        cerr << "Error: Cannot create directory '" << dirName << "'\n";
        resetColor();
    }
}

void removeDirectory(const string& dirName) {
    if (dirName.empty()) {
        setColor("red");
        cerr << "Error: Please specify a directory name\n";
        resetColor();
        return;
    }
    
    if (_rmdir(dirName.c_str()) == 0) {
        setColor("green");
        cout << "Directory removed: " << dirName << endl;
        resetColor();
    } else {
        setColor("red");
        cerr << "Error: Cannot remove directory '" << dirName << "' (must be empty)\n";
        resetColor();
    }
}

void removeFile(const string& fileName) {
    if (fileName.empty()) {
        setColor("red");
        cerr << "Error: Please specify a file name\n";
        resetColor();
        return;
    }
    
    if (remove(fileName.c_str()) == 0) {
        setColor("green");
        cout << "File deleted: " << fileName << endl;
        resetColor();
    } else {
        setColor("red");
        cerr << "Error: Cannot delete file '" << fileName << "'\n";
        resetColor();
    }
}

void touchFile(const string& fileName) {
    if (fileName.empty()) {
        setColor("red");
        cerr << "Error: Please specify a file name\n";
        resetColor();
        return;
    }
    
    ofstream file(fileName);
    if (file.is_open()) {
        file.close();
        setColor("green");
        cout << "File created: " << fileName << endl;
        resetColor();
    } else {
        setColor("red");
        cerr << "Error: Could not create file '" << fileName << "'\n";
        resetColor();
    }
}

void catFile(const string& fileName) {
    if (fileName.empty()) {
        setColor("red");
        cerr << "Error: Please specify a file name\n";
        resetColor();
        return;
    }
    
    ifstream file(fileName);
    if (file.is_open()) {
        if (file.peek() == ifstream::traits_type::eof()) {
            setColor("yellow");
            cout << "\n(File is empty)\n" << endl;
            resetColor();
        } else {
            setColor("cyan");
            cout << "\n--- " << fileName << " ---\n";
            resetColor();
            
            string line;
            int lineNum = 1;
            while (getline(file, line)) {
                setColor("yellow");
                cout << lineNum << ": ";
                resetColor();
                cout << line << endl;
                lineNum++;
            }
            
            setColor("cyan");
            cout << "--- End of file ---\n" << endl;
            resetColor();
        }
        file.close();
    } else {
        setColor("red");
        cerr << "Error: Could not open file '" << fileName << "'\n";
        resetColor();
    }
}

void echoText(const vector<string>& tokens) {
    if (tokens.size() < 2) {
        cout << endl;
        return;
    }
    
    for (size_t i = 1; i < tokens.size(); i++) {
        cout << tokens[i];
        if (i < tokens.size() - 1) {
            cout << " ";
        }
    }
    cout << endl;
}

void showDateTime() {
    time_t now = time(0);
    tm* localTime = localtime(&now);
    
    char buffer[100];
    const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    
    int hour = localTime->tm_hour;
    const char* period = (hour >= 12) ? "PM" : "AM";
    if (hour == 0) hour = 12;
    else if (hour > 12) hour -= 12;
    
    sprintf(buffer, "%s %s %02d %02d:%02d:%02d %s %d",
            days[localTime->tm_wday],
            months[localTime->tm_mon],
            localTime->tm_mday,
            hour,
            localTime->tm_min,
            localTime->tm_sec,
            period,
            localTime->tm_year + 1900);
    
    setColor("cyan");
    cout << "Current date/time: ";
    resetColor();
    cout << buffer << endl;
}

void showHistory() {
    if (commandHistory.empty()) {
        setColor("yellow");
        cout << "No command history yet.\n";
        resetColor();
        return;
    }
    
    setColor("cyan");
    cout << "\n=== Command History ===\n";
    resetColor();
    
    for (size_t i = 0; i < commandHistory.size(); i++) {
        setColor("yellow");
        cout << i + 1 << ": ";
        resetColor();
        cout << commandHistory[i] << endl;
    }
    cout << endl;
}

void showEnvironment() {
    setColor("cyan");
    cout << "\n=== Environment Variables ===\n";
    resetColor();
    
    const char* envVars[] = {"PATH", "USERPROFILE", "USERNAME", "COMPUTERNAME", "OS", "PROCESSOR_ARCHITECTURE"};
    
    for (const char* var : envVars) {
        char* value = getenv(var);
        if (value != nullptr) {
            setColor("yellow");
            cout << var << ": ";
            resetColor();
            
            if (string(var) == "PATH") {
                cout << "\n";
                string pathStr(value);
                stringstream ss(pathStr);
                string path;
                int count = 1;
                
                while (getline(ss, path, ';')) {
                    if (!path.empty()) {
                        setColor("green");
                        cout << "  [" << count++ << "] ";
                        resetColor();
                        cout << path << "\n";
                    }
                }
            } else {
                cout << value << endl;
            }
        }
    }
    cout << endl;
}

void clearScreen() {
    system("cls");
}

vector<string> parseCommand(const string& input) {
    vector<string> tokens;
    stringstream ss(input);
    string token;
    
    while (ss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

bool handleRedirection(vector<string>& tokens, string& outputFile, bool& appendMode) {
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i] == ">>") {
            if (i + 1 < tokens.size()) {
                outputFile = tokens[i + 1];
                appendMode = true;
                tokens.erase(tokens.begin() + i, tokens.end());
                return true;
            } else {
                setColor("red");
                cerr << "Error: No filename specified after >>\n";
                resetColor();
                return false;
            }
        } else if (tokens[i] == ">") {
            if (i + 1 < tokens.size()) {
                outputFile = tokens[i + 1];
                appendMode = false;
                tokens.erase(tokens.begin() + i, tokens.end());
                return true;
            } else {
                setColor("red");
                cerr << "Error: No filename specified after >\n";
                resetColor();
                return false;
            }
        }
    }
    return false;
}

bool processCommand(const string& input) {
    if (input.empty()) {
        return true;
    }
    
    // Add to history
    commandHistory.push_back(input);
    if (commandHistory.size() > 100) {
        commandHistory.erase(commandHistory.begin());
    }
    
    vector<string> tokens = parseCommand(input);
    
    if (tokens.empty()) {
        return true;
    }
    
    string outputFile;
    bool appendMode = false;
    bool redirect = handleRedirection(tokens, outputFile, appendMode);
    
    // Check if tokens is empty after redirection handling
    if (tokens.empty()) {
        return true;
    }

    streambuf* originalCoutBuffer = nullptr;
    ofstream fileStream;
    
    if (redirect && !outputFile.empty()) {
        if (appendMode) {
            fileStream.open(outputFile, ios::app);
        } else {
            fileStream.open(outputFile);
        }
        
        if (fileStream.is_open()) {
            originalCoutBuffer = cout.rdbuf();
            cout.rdbuf(fileStream.rdbuf());
        } else {
            setColor("red");
            cerr << "Error: Cannot open file '" << outputFile << "' for writing\n";
            resetColor();
            return true;
        }
    }
    
    string command = tokens[0];
    
    if (command == "exit" || command == "quit") {
        return false;
    } else if (command == "help") {
        showHelp();
    } else if (command == "ls") {
        if (tokens.size() > 1) {
            listDirectory(tokens[1]);
        } else {
            listDirectory();
        }
    } else if (command == "pwd") {
        printWorkingDirectory();
    } else if (command == "cd") {
        if (tokens.size() > 1) {
            changeDirectory(tokens[1]);
        } else {
            setColor("red");
            cerr << "Error: cd requires a directory argument\n";
            resetColor();
        }
    } else if (command == "mkdir") {
        if (tokens.size() > 1) {
            makeDirectory(tokens[1]);
        } else {
            setColor("red");
            cerr << "Error: mkdir requires a directory name\n";
            resetColor();
        }
    } else if (command == "rmdir") {
        if (tokens.size() > 1) {
            removeDirectory(tokens[1]);
        } else {
            setColor("red");
            cerr << "Error: rmdir requires a directory name\n";
            resetColor();
        }
    } else if (command == "rm") {
        if (tokens.size() > 1) {
            removeFile(tokens[1]);
        } else {
            setColor("red");
            cerr << "Error: rm requires a file name\n";
            resetColor();
        }
    } else if (command == "touch") {
        if (tokens.size() > 1) {
            touchFile(tokens[1]);
        } else {
            setColor("red");
            cerr << "Error: touch requires a file name\n";
            resetColor();
        }
    } else if (command == "cat") {
        if (tokens.size() > 1) {
            catFile(tokens[1]);
        } else {
            setColor("red");
            cerr << "Error: cat requires a file name\n";
            resetColor();
        }
    } else if (command == "echo") {
        echoText(tokens);
    } else if (command == "date") {
        showDateTime();
    } else if (command == "history") {
        showHistory();
    } else if (command == "env") {
        showEnvironment();
    } else if (command == "clear") {
        clearScreen();
        printBanner();
    } else {
        setColor("red");
        cout << "Unknown command: " << command << ". Type 'help' for available commands.\n";
        resetColor();
    }
    
    if (redirect && originalCoutBuffer != nullptr) {
        cout.rdbuf(originalCoutBuffer);
        fileStream.close();
        if (command == "echo") {
            setColor("green");
            cout << "\u221A ";
            resetColor();
            if (appendMode) {
                cout << "Text appended to: ";
                setColor("cyan");
                cout << outputFile;
                resetColor();
                cout << endl;
            } else {
                cout << "Text written to: ";
                setColor("cyan");
                cout << outputFile;
                resetColor();
                cout << endl;
            }
        }
    }
    
    return true;
}

int main() {
    string input;
    
    signal(SIGINT, signalHandler);
    
    clearScreen();
    printBanner();
    
    while (true) {
        setColor("green");
        cout << "MiniShell";
        setColor("yellow");
        cout << "> ";
        resetColor();
        getline(cin, input);
        
        if (!processCommand(input)) {
            setColor("green");
            cout << "\n\u221A ";
            resetColor();
            cout << "Goodbye! Thanks for using MiniShell.\n";
            break;
        }
    }
    
    return 0;
}
