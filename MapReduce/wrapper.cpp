#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

const string map = "mapper";
const string reduce = "reducer";

int main (int argc, char *argv[]) {
    if (argc < 3) {
        cerr << "wrapper support output\n";
        exit(1);
    }

    int transaction_size = 100000;
    const string hadoop_home = getenv("HADOOP_HOME");
    const string hadoop = hadoop_home + "/bin/hadoop";
    const string hadoop_stream = hadoop_home + "/share/hadoop/tools/lib/hadoop-streaming-3.3.1.jar";
    const string hadoop_hdfs = hadoop_home + "/bin/hdfs";
    //const string rm_output = hadoop_hdfs + " dfs -rm -r output";

    int step = 0;
    while (true) {
        step++;
        if (step == 4) break;

        const string mapper_command = "\"" + map + " " + to_string(step) + "\"";
        const string reducer_command = "\"" + reduce + " " + to_string(step) +
                                       " " + argv[1] + " " +
                                       to_string(transaction_size) + "\"";
        string command = hadoop + " jar " + hadoop_stream + " -input input" +
                         " -output output" + to_string(step) + " -mapper "
                         + mapper_command + " -reducer " + reducer_command +
                         " -file " + map + " -file " + reduce;
        if (step > 1)
            command += " -file itemset" + to_string(step - 1) + ".txt";

        const string ofolder = " output" + to_string(step);
        const string cp_output = hadoop_hdfs + " dfs -get" + ofolder + ofolder;

        const string mv_output = "cp" + ofolder + "/part-00000 itemset" +
                                  to_string(step) + ".txt";

        cout << command << endl << endl;
        system(command.c_str());
        cout << cp_output << endl << endl;
        system(cp_output.c_str());
        cout << mv_output << endl << endl;
        system(mv_output.c_str());
    }

}
