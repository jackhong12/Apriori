#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

#define DEBUG

#define XTERM

#ifdef XTERM
#define cred(msg) ("\033[31m" + (msg) + "\033[39m")
#define cgreen(msg) ("\033[32m" + (msg) + "\033[39m")
#else
#define cred(msg) msg
#define cgreen(msg) msg
#endif

using namespace std;

const string map_exe = "mapper";
const string reduce_exe = "reducer";
const string hadoop_hdfs_sub = "/bin/hdfs";
const string hadoop_stream_sub = "/share/hadoop/tools/lib/hadoop-streaming-3.3.1.jar";
const string hadoop_sub = "/bin/hadoop";
const string input_dir = "/user/input";
const string output_dir = "/user/output";
const string tmp_file = "itemsets";
string hadoop_home, hadoop_stream, hadoop_hdfs, hadoop;


void panic (string msg) {
    cerr << cred("Error: " + msg) << endl;
    exit(1);
}

void lsystem (string command) {
#ifdef DEBUG
    cout << cgreen("+ system: " + command) << endl;
#endif
    if (system(command.c_str()))
        panic("exe > " + command);
}

void lsystem_nonblock (string command) {
    size_t pid;
    if ((pid = fork()) < 0)
        panic("fork fail");
    // child
    if (!pid) {
        lsystem(command);
        exit(0);
    }
}

void init_env () {
    char *ptr;
    if (!(ptr = getenv("HADOOP_HOME")))
        panic("No HADOOP_HOME enviroment variable");
    hadoop_home = string(ptr);
    hadoop_stream = hadoop_home + hadoop_stream_sub;
    hadoop_hdfs = hadoop_home + hadoop_hdfs_sub;
    hadoop = hadoop_home + hadoop_sub;
}

void init_input (string test) {
    // delete input folder
    lsystem(hadoop_hdfs + " dfs -rm -r -f " + input_dir);
    // create input folder
    lsystem(hadoop_hdfs + " dfs -mkdir -p " + input_dir);
    // upload test case
    lsystem(hadoop_hdfs + " dfs -put -f " + test + " " + input_dir);
}

void start_map_reduce (string support, int size, int run) {

    string map_command = map_exe + " " + to_string(run);
    string reduce_command = reduce_exe +
        " " + to_string(run) +
        " " + support +
        " " + to_string(size);
    string out_dir = output_dir + to_string(run);
    string tmpf_input = tmp_file + to_string(run - 1) + ".txt";
    string tmpf_output = tmp_file + to_string(run) + ".txt";
    string command = hadoop + " jar " + hadoop_stream +
        " -D mapred.job.name=\"apriori\"" +
        " -input " + input_dir +
        " -output " + out_dir +
        " --mapper \"" + map_command + "\"" +
        " --reducer \"" + reduce_command + "\"" +
        " -file " + map_exe +
        " -file " + reduce_exe;

    if (run > 1)
        command += " -file " + tmpf_input;

    // run i-th iteration Apriori
    lsystem(command);

    // download result
    lsystem(hadoop_hdfs + " dfs -get -f " + out_dir + "/part-00000 " +
            tmpf_output);

    // delete output folder
    lsystem_nonblock(hadoop_hdfs + " dfs -rm -r " + out_dir);
}

int get_line_num (string file) {
    ifstream fin;
    fin.open(file);
    string tmp;
    int result = 0;
    while (getline(fin, tmp))
        result++;

    fin.close();
    return result;
}

int main (int argc, char *argv[]) {
    if (argc != 4) {
        cerr << "wrapper support input output\n";
        exit(1);
    }

    int transaction_size = get_line_num(argv[2]);

    // initialize enviroment variables
    init_env();

    // move test case to hadoop filesystem
    init_input(argv[2]);

    for (int i = 1; ; i++) {
        start_map_reduce(argv[1], transaction_size, i);
        string output = tmp_file + to_string(i) + ".txt";
        if (!get_line_num(output))
            break;
    }

}
