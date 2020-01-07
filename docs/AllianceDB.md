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
15010279
-s
15280728
-R
/home/xtra/data/datasets/stock/cj_key32_partitioned.csv
-S
/home/xtra/data/datasets/stock/sb_key32_partitioned.csv
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
999997
-s
999997
-R
/home/xtra/data/datasets/rovio/rovio_key32_partitioned.txt
-S
/home/xtra/data/datasets/rovio/rovio_key32_partitioned.txt
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
1001
-s
749900
-R
/home/xtra/data/datasets/YSB/campaigns_key32_partitioned.txt
-S
/home/xtra/data/datasets/YSB/ad_events_key32_partitioned.txt
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
1003605
-s
2052169
-R
/home/xtra/data/datasets/DEBS/comments_key32_partitioned.csv
-S
/home/xtra/data/datasets/DEBS/posts_key32_partitioned.csv
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
/home/xtra/data/datasets/google/users_key32_partitioned.csv
-S
/home/xtra/data/datasets/google/reviews_key32_partitioned.csv
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
/home/xtra/data/datasets/amazon/amazon_answer_partitioned.csv
-S
/home/xtra/data/datasets/amazon/amazon_question_partitioned.csv
-J
0
-K
0
```



