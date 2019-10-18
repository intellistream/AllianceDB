#include <stdio.h>  /* FILE, fopen */
#include <stdlib.h> /* exit, perror */
#include <unistd.h> /* sysconf */
#include <assert.h> /* assert() */
#include <numa.h>

#include "../config.h" /* HAVE_LIBNUMA */

#ifdef HAVE_NUMA
#include <numa.h>   /* for automatic NUMA-mappings */
#endif

#include "cpu_mapping.h"

/** \internal
 * @{
 */

#define MAX_THREADS 1024

static int inited = 0;
static int max_cpus;
static int max_threads;
static int cpumapping[MAX_THREADS];

#ifdef HAVE_NUMA
static int numamap1d[MAX_THREADS];
#endif

/*** NUMA-Topology related variables ***/
static int numthreads;
/* if there is no info., default is assuming machine has only-1 NUMA region */
static int numnodes = 1;
static int thrpernuma;
static int ** numa;
static char ** numaactive;
static int * numaactivecount;

/**
 * Initializes the cpu mapping from the file defined by CUSTOM_CPU_MAPPING.
 * NUMBER-OF-THREADS(NTHR) and mapping of PHYSICAL-THR-ID for each LOGICAL-THR-ID from
 * 0 to NTHR and optionally number of NUMA-nodes (overridden by libNUMA value if
 * exists).
 * The mapping used for our machine Intel E5-4640 is =
 * "64 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53  54 55 56 57 58 59 60 61 62 63 4".
 */
static int
init_mappings_from_file()
{
    FILE * cfg;
    int i;

    cfg = fopen(CUSTOM_CPU_MAPPING, "r");
    if (cfg!=NULL) {
        if(fscanf(cfg, "%d", &max_threads) <= 0) {
            perror("Could not parse input!\n");
        }

        for(i = 0; i < max_threads; i++){
            if(fscanf(cfg, "%d", &cpumapping[i]) <= 0) {
                perror("Could not parse input!\n");
            }
        }
        int numnodes_from_file = 1;
        if(fscanf(cfg, "%d", &numnodes_from_file) > 0) {
            /* number of NUMA nodes configured by the user (if libNUMA not
               exists) */
            numnodes = numnodes_from_file;
        }

        fclose(cfg);
        fprintf(stdout, "\n[INFO ] CPU mappings are intialized from %s.\n"\
                        "[WARN ] Make sure to use at most %d threads with -n on the commandline.\n",
                CUSTOM_CPU_MAPPING, max_threads);
        return 1;
    }


    /* perror("Custom cpu mapping file not found!\n"); */
    return 0;
}

/**
 * Initialize NUMA-topology with libnuma.
 */
static void
numa_default_init()
{
    /* numnodes   = 1; */
    numthreads = max_cpus;
    thrpernuma = max_cpus;
    numa = (int **) malloc(sizeof(int *));
    numa[0] = (int *) malloc(sizeof(int) * numthreads);
    numaactive = (char **) malloc(sizeof(char *));
    numaactive[0] = (char *) calloc(numthreads, sizeof(char));
    numaactivecount = (int *) calloc(numnodes, sizeof(int));
    for(int i = 0; i < max_cpus; i++){
        if(max_cpus == max_threads)
            numa[0][i] = cpumapping[i];
        else
            numa[0][i] = i;
    }
}

static void
numa_init()
{
#ifdef HAVE_NUMA
    int i, k, ncpus, j;
	struct bitmask *cpus;

    fprintf(stdout, "[INFO ] Getting the NUMA configuration automatically with libNUMA. \n");

	if (numa_available() < 0)  {
		//printf("no numa\n");
		numa_default_init();
		return;
	}

	numnodes   = numa_num_configured_nodes();
	cpus       = numa_allocate_cpumask();
	ncpus      = cpus->size;
	thrpernuma = sysconf(_SC_NPROCESSORS_ONLN) / numnodes; /* max #threads per NUMA-region */
	numa = (int **) malloc(sizeof(int *) * numnodes);
	numaactive = (char **) malloc(sizeof(char *) * numnodes);
	for (i = 0; i < numnodes ; i++) {
		numa[i] = (int *) malloc(sizeof(int) * thrpernuma);
		numaactive[i] = (char *) calloc(thrpernuma, sizeof(char));
	}
	numaactivecount = (int *) calloc(numnodes, sizeof(int));

    //printf("\n");
	int nm = 0;
	for (i = 0; i < numnodes ; i++) {
		if (numa_node_to_cpus(i, cpus) < 0) {
			printf("node %d failed to convert\n",i);
		}
		//printf("Node-%d: ", i);
		j = 0;
		for (k = 0; k < ncpus; k++){
			if (numa_bitmask_isbitset(cpus, k)){
				//printf(" %s%d", k>0?", ":"", k);
                numa[i][j] = k;
                j++;
                numamap1d[k] = nm++;
            }
		}
		//printf("\n");
	}
    numthreads = thrpernuma * numnodes;

    numa_free_cpumask(cpus);

#else
    fprintf(stdout, "[WARN ] NUMA is not available, using single NUMA-region as default.\n");
    numa_default_init();
#endif
}

void
cpu_mapping_cleanup()
{
    for (int i = 0; i < numnodes ; i++) {
        free(numa[i]);
        free(numaactive[i]);
    }
    free(numa);
    free(numaactive);
    free(numaactivecount);
}


/** @} */

/**
 *  Try custom cpu mapping file first, if does not exist then round-robin
 *  initialization among available CPUs reported by the system.
 */
void
cpu_mapping_init()
{
    max_cpus  = sysconf(_SC_NPROCESSORS_ONLN);
    if( init_mappings_from_file() == 0 ) {
        int i;

        max_threads = max_cpus;
        for(i = 0; i < max_cpus; i++){
            cpumapping[i] = i;
        }
    }

    numa_init();
    inited = 1;
}

void
numa_thread_mark_active(int phytid)
{
    int numaregionid = -1;
    for(int i = 0; i < numnodes; i++){
        for(int j = 0; j < thrpernuma; j++){
            if(numa[i][j] == phytid){
                numaregionid = i;
                break;
            }
        }
        if(numaregionid != -1)
            break;
    }

    int thridx = -1;
    for(int i = 0; i < numnodes; i++){
        for(int j = 0; j < thrpernuma; j++){
            if(numa[i][j] == phytid){
                thridx = j;
                break;
            }
        }
        if(thridx != -1)
            break;
    }

    if(numaactive[numaregionid][thridx] == 0){
        numaactive[numaregionid][thridx] = 1;
        numaactivecount[numaregionid] ++;
    }
}

/**
 * Returns SMT aware logical to physical CPU mapping for a given logical thr-id.
 */
int
get_cpu_id(int thread_id)
{
    if(!inited){
        cpu_mapping_init();
        //inited = 1;
#if 0
        printf("\n------ Thread mappings -----\n");
        for(int t = 0; t < max_threads; t++){
            printf("Thread-%d --> CPU[%d] \n", t, get_cpu_id(t));
        }


        for(int n = 0; n < numnodes; n++){
            printf("NUMA-%d : ", n);
            for(int t = 0; t < thrpernuma; t++){
                printf("numa[%d][%d]=%d, ", n, t, numa[n][t]);
            }
            printf("\n");
        }

#ifdef HAVE_NUMA
        printf("------ NUMA-1D-mapping of logical threads -----\n");
        for(int n = 0; n < max_threads; n++){
            printf("numa1d[%d]=%d, ", n, numamap1d[n]);
        }
        printf("\n");
#endif

#endif
    }

    return cpumapping[thread_id % max_threads];
}

/**
 * Topology of Intel E5-4640 used in our experiments.
 node 0 cpus: 0 4 8 12 16 20 24 28 32 36 40 44 48 52 56 60
 node 1 cpus: 1 5 9 13 17 21 25 29 33 37 41 45 49 53 57 61
 node 2 cpus: 2 6 10 14 18 22 26 30 34 38 42 46 50 54 58 62
 node 3 cpus: 3 7 11 15 19 23 27 31 35 39 43 47 51 55 59 63
*/
/*
static int numa[][16] = {
    {0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60},
    {1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61},
    {2, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62},
    {3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63} };
*/

int
is_first_thread_in_numa_region(int logicaltid)
{
    int phytid = get_cpu_id(logicaltid);
    int ret = 0;
    for(int i = 0; i < numnodes; i++)
    {
        int j = 0;
        while(j < thrpernuma && !numaactive[i][j])
            j++;
        if(j < thrpernuma)
            ret = ret || (phytid == numa[i][j]);
    }

    return ret;
}

int
get_thread_index_in_numa(int logicaltid)
{
    int ret = -1;
    int phytid = get_cpu_id(logicaltid);

    for(int i = 0; i < numnodes; i++){
        int active_idx = 0;
        for(int j = 0; j < thrpernuma; j++){
            if(numa[i][j] == phytid){
                assert(numaactive[i][j]);
                ret = active_idx;
                break;
            }

            if(numaactive[i][j])
                active_idx ++;
        }
        if(ret != -1)
            break;
    }


    return ret;
}

int
get_numa_region_id(int logicaltid)
{
    int ret = -1;
    int phytid = get_cpu_id(logicaltid);

    for(int i = 0; i < numnodes; i++){
        for(int j = 0; j < thrpernuma; j++){
            if(numa[i][j] == phytid){
                ret = i;
                break;
            }
        }
        if(ret != -1)
            break;
    }

    return ret;
}

int
get_num_numa_regions(void)
{
    return numnodes;
}

int
get_num_active_threads_in_numa(int numaregionid)
{
    return numaactivecount[numaregionid];
}

int
get_numa_index_of_logical_thread(int logicaltid)
{
#ifdef HAVE_NUMA
    return numamap1d[logicaltid];
#else
    return cpumapping[logicaltid];
#endif
}

int
get_logical_thread_at_numa_index(int numaidx)
{
#ifdef HAVE_NUMA
    return numa[numaidx/thrpernuma][numaidx%thrpernuma];
#else
    return cpumapping[numaidx];
#endif
}
