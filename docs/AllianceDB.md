### Test with TPCH

```
-a
SHJ_JM_NP
-r
10
-s
10
-R
/home/xtra/AllianceDB/dbgen/dataset/orders.tbl
-S
/home/xtra/AllianceDB/dbgen/dataset/customer.tbl
-J
1
-K
0
```

### 1. Stream - Stream

### Stock

````
-a
SHJ_JM_NP
-r
1000000
-s
1000000
-R
/data1/xtra/datasets/stock/cj_key32_sampled_partitioned.csv
-S
/data1/xtra/datasets/stock/sb_key32_sampled_partitioned.csv
-J
0
-K
0
-L
1
-M
1
````

### Rovio

```
-a
SHJ_JM_NP
-r
1000000
-s
1000000
-R
/data1/xtra/datasets/rovio/rovio_key32_partitioned.txt
-S
/data1/xtra/datasets/rovio/rovio_key32_partitioned.txt
-J
0
-K
0
-L
3
-M
3
```

### 2. Batch - Stream

#### Yahoo Streaming Benchmark

```
-a
SHJ_JM_NP
-r
1000
-s
1000000
-R
/data1/xtra/datasets/YSB/campaigns_key32_partitioned.txt
-S
/data1/xtra/datasets/YSB/ad_events_key32_partitioned.txt
-J
0
-K
0
-L
0
-M
1
```

### 3. Batch - Batch

#### DEBS

```
-a
SHJ_JM_NP
-r
1000000
-s
1000000
-R
/data1/xtra/datasets/DEBS/posts_key32_partitioned.csv
-S
/data1/xtra/datasets/DEBS/comments_key32_partitioned.csv
-J
0
-K
0
```

#### Google

```
-a
SHJ_JM_NP
-r
3747939
-s
11931801
-R
/data1/xtra/datasets/google/users_key32_partitioned.csv
-S
/data1/xtra/datasets/google/reviews_key32_partitioned.csv
-J
1
-K
1
```

#### Amazon

```
-a
SHJ_JM_NP
-r
10
-s
10
-R
/data1/xtra/datasets/amazon/amazon_question_partitioned.csv
-S
/data1/xtra/datasets/amazon/amazon_answer_partitioned.csv
-J
0
-K
0
```


#### Kim

Kim data generator parameter

1. `t`: either to generate tuples with timestamps, 0 - w/o ts, 1 - w/ ts.
2. `w`: window size (msecs) of join operation, $w > 0$.
3. `n`: number of threads, $n > 0$.
4. `e`:  step size,  generate number of tuples at each step, all tuples in the same step have the same timestamp, $e > 0$.
5. `l`: interval of each step (msecs),  [0, window_size], (0 means batch operation).
6. `d`: distribution of key-field, [0 - unique, 2 - zipf].
7. `z` : configure key-filed zipf distribution.
8. `D`: distribution of ts-field, [0 - unique,  2 - zipf].
7. `Z`: configure ts-field zipf distribution.  zipf factor of zipf distribution should be configured, range [0,1]

num_tuples = (window_size / interval) * step_size;
