//
// Created by Shuhao Zhang on 8/9/19.
//

#include <stdio.h>  /* FILE, fopen */
#include <stdlib.h> /* exit, perror */
#include <unistd.h> /* sysconf */
#include <assert.h> /* assert() */

#include "cpu_mapping.h"/* HAVE_LIBNUMA */

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
static void numa_init() {
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

/**
 *  Try custom cpu mapping file first, if does not exist then round-robin
 *  initialization among available CPUs reported by the system.
 */
void
cpu_mapping_init() {
    max_cpus = sysconf(_SC_NPROCESSORS_ONLN);
    if (init_mappings_from_file() == 0) {
        int i;

        max_threads = max_cpus;
        for (i = 0; i < max_cpus; i++) {
            cpumapping[i] = i;
        }
    }

    numa_init();
    inited = 1;
}

