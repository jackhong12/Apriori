# MapReduce

## Usage
```bash
make
./apriori-map-reduce 1 ../testcases/transactional_T10I4D100K.csv output.txt
```

## Install
- Install accroding to manual
    - Follow steps in [Hadoop Single Cluster](https://hadoop.apache.org/docs/stable/hadoop-project-dist/hadoop-common/SingleCluster.html) and [Hadoop Cluster Setup](https://hadoop.apache.org/docs/stable/hadoop-project-dist/hadoop-common/ClusterSetup.html) to install Hadoop.

- Install with scripts
    - 1. Install dependencies
        ```bash
        ./scripts/install.sh
        ```

    - 2. Activate nodes
        ```bash
        ./scripts/start.sh
        ```

## Reference
- [Hadoop Streaming](https://hadoop.apache.org/docs/stable/hadoop-streaming/HadoopStreaming.html?fbclid=IwAR0V4E8Xek2jv5PnSMyq-PBGFuVralYpIDRoUrYtJgIOU3yAMc1Xfjp_--Q)
- [How To Set Up a Hadoop 3.2.1 Multi-Node Cluster on Ubuntu 18.04 (2 Nodes)](https://medium.com/@jootorres_11979/how-to-set-up-a-hadoop-3-2-1-multi-node-cluster-on-ubuntu-18-04-2-nodes-567ca44a3b12)
