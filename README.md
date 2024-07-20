# FreeSam

FreeSam is a sampling technique achieving adaptive sampling for intra-window join, and can simultaneously optimize quantity and quality. For the design insights, please refer to our SIGMOD'25 paper. Here we provide its implementation based on AllianceDB.

## Prerequisite

1. cmake > 3.13.0.
2. Profiling only supports Intel CPUs.
3. You may need to reset pmu once at first run, some pcm profiling results may be incorrect at first time.
4. Prepare cpu-mapping in `cpu-mapping.txt`.
5. Specify `exp_dir` in `run_all.sh`.
6. Unzip real world datasets in `data1/xtra/datasets` and run preprocessing scripts if neccessary, and then move it to `exp_dir/datasets`.
7. Our program should be running as root with two params exp_dir and l3_cache_size: `sudo bash run_all.sh -d /data1/xtra -c 19922944`.
8. You can run a test experiment by running `run_all.sh`.

## Datasets

We have 3 real datasets that are compressed in `exp_dir/datasets`.

We extracted the useful columns of those datasets, the one is joined key and another is timestamp.

DEBS: 

​	comments_key32_partitioned.csv: user_id|comments_payload

​	posts_key32_partitioned.csv: user_id|posts_payload

Rovio:

​	1000ms_1t.txt: combined_id|payload|price|timestamp

EERC: 

​	SEP85L.txt: regionid|timestamp

​	SEP86L.txt: regionid|timestamp

