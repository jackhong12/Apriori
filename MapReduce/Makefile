
CXX := g++
#CXX_FLAG := -g
CXX_FLAG := -g -O3
MAP_EXE := mapper
REDUCE_EXE := reducer
WRAPPER_EXE := wrapper

SRC := $(notdir $(wildcard *.cpp))
EXE := $(MAP_EXE) $(REDUCE_EXE) $(WRAPPER_EXE)

# variable for hadoop
HADOOP := $(HADOOP_HOME)/bin/hadoop
HDFS := $(HADOOP_HOME)/bin/hdfs
HADOOP_STREAM := $(HADOOP_HOME)/share/hadoop/tools/lib/hadoop-streaming-3.3.1.jar

all: $(EXE)

$(MAP_EXE): $(MAP_EXE).cpp
	$(CXX) $< -o $@ $(CXX_FLAG)

$(REDUCE_EXE): $(REDUCE_EXE).cpp
	$(CXX) $< -o $@ $(CXX_FLAG)

$(WRAPPER_EXE): $(WRAPPER_EXE).cpp
	$(CXX) $< -o $@ $(CXX_FLAG)

hadoop_init_fs:
	$(HDFS) dfs -mkdir -p /user
	$(HDFS) dfs -mkdir -p /user/$(USER)
	$(HDFS) dfs -mkdir -p /user/$(USER)/input
	$(HDFS) dfs -put ../transactional_T10I4D100K.csv input

run3:
	$(HADOOP) jar $(HADOOP_STREAM) -D mapred.job.name="apriori" \
		-input input -output output \
		--mapper "$(MAP_EXE) 3" \
		--reducer "$(REDUCE_EXE) 3 1 100000" \
		-file $(MAP_EXE) \
		-file $(REDUCE_EXE) \
		-file itemset2.txt

	rm -rf output
	$(HDFS) dfs -get output output
	$(HDFS) dfs -rm -r output

run2:
	$(HADOOP) jar $(HADOOP_STREAM) -D mapred.job.name="apriori" \
		-input input -output output \
		--mapper "$(MAP_EXE) 2" \
		--reducer "$(REDUCE_EXE) 2 1 100000" \
		-file $(MAP_EXE) \
		-file $(REDUCE_EXE) \
		-file itemset1.txt

	rm -rf output
	$(HDFS) dfs -get output output
	$(HDFS) dfs -rm -r output

run1:
	$(HADOOP) jar $(HADOOP_STREAM) -D mapred.job.name="apriori" \
		-input input -output output \
		--mapper "$(MAP_EXE) 1" \
		--reducer "$(REDUCE_EXE) 1 1 100000" \
		-file $(MAP_EXE) \
		-file $(REDUCE_EXE) \
		-file tmp.txt

	rm -rf output
	$(HDFS) dfs -get output output
	$(HDFS) dfs -rm -r output

clean:
	rm $(EXE)
