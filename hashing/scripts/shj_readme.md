Instuctions for running SHJ experiments

### performance metrics
1. throughput
2. latency
3. progressiveness

### Benchmark workloads
1. *Stock*: join the traded stream (R) and the quotes stream (S) over the same stock id within the same window (i.e., same period of time). Low arrival rates.
2. *Rovio*: join the advertisements stream (R) and the purchase stream (S) (same source here?) over the user id and advertisement id within the same window. Low arrival rates.
3. *YSB*: joins the campaigns table (R) and the advertisements stream (S) over the campaign id. High arrival rates.
4. *DEBS*: joins the comments (S) and the post (R) over the same user id. Both inputs stored at rest: wind_len = 0, arrv_rate = infinity.
5. *Micro*: sythetic workload ("kim"). "AR" "RAR" "AD" "KD" "WS" "DD".

### Parameters (param_t)
{"verbose",          no_argument,       &verbose_flag,   1},
{"brief",            no_argument,       &verbose_flag,   0},
{"non-unique",       no_argument,       &nonunique_flag, 1},
{"full-range",       no_argument,       &fullrange_flag, 1},
{"basic-numa",       no_argument,       &basic_numa,     1},
{"help",             no_argument,       0,               'h'},
{"version",          no_argument,       0,               'v'},
/* These options don't set a flag.
    We distinguish them by their indices. */
{"algo",             required_argument, 0,               'a'}, // SHJ
{"nthreads",         required_argument, 0,               'n'}, //
{"prfconfe",         required_argument, 0,               'p'}, // Intel PCM config file with upto 4 counters [none]
{"r-size",           required_argument, 0,               'r'}, //
{"s-size",           required_argument, 0,               's'}, // ssize >= rsize
{"perfout",          required_argument, 0,               'o'}, // Output file to print performance counters [stdout]
{"r-seed",           required_argument, 0,               'x'}, //
{"s-seed",           required_argument, 0,               'y'}, //
{"skew",             required_argument, 0,               'z'}, // Zipf skew parameter for probe relation S
{"r-file",           required_argument, 0,               'R'}, //
{"s-file",           required_argument, 0,               'S'}, //
{"r-key",            required_argument, 0,               'J'}, //
{"s-key",            required_argument, 0,               'K'}, //
{"r-ts",             required_argument, 0,               'L'}, // timestamp col no.
{"s-ts",             required_argument, 0,               'M'}, //
{"partfanout",       required_argument, 0,               'f'}, // partitioning fanout, e.g., 2^rdxbits
{"numastrategy",     required_argument, 0,               'N'},
{"mwaybufsize",      required_argument, 0,               'm'},

{"gen_with_ts",      required_argument, 0,               't'}, // generate with timestamp?
{"real_data",        required_argument, 0,               'B'}, //
{"window-size",      required_argument, 0,               'w'}, //
{"step-size",        required_argument, 0,               'e'}, // step_sizeR
{"step-sizeS",        required_argument, 0,              'p'}, // step_sizeS
{"interval",         required_argument, 0,               'l'}, // distribution interval size
{"key_distribution", required_argument, 0,               'd'}, // 0 uniform, 2 zipfian
{"zipf_param",       required_argument, 0,               'Z'}, // zipfian distribution factor
{"exp_id",           required_argument, 0,               'I'}, //
{"ts_distribution",  required_argument, 0,               'D'}, // 0 uniform, 2 zipfian
{"duplicate_num",    required_argument, 0,               'P'}, // represent num of Partitions
{"progress_step",    required_argument, 0,               '['},
{"merge_step",       required_argument, 0,               ']'}, // not in use


### Experiment sections
APP_BENCH       
MICRO_BENCH
SCALE_STUDY
PROFILE_MICRO
PROFILE
PROFILE_MEMORY_CONSUMPTION
PROFILE_PMU_COUNTERS
PROFILE_TOPDOWN
