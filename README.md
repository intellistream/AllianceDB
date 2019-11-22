<p align="left">
  <a href="https://github.com/ShuhaoZhangTony/AllianceDB"><img alt="GitHub Actions status" src="https://github.com/ShuhaoZhangTony/AllianceDB/workflows/.github/workflows/ccpp.yml/badge.svg?branch=eth"></a>
</p>

# Stream Joins on Modern Multicore Processors

## Workload Distribution

### Join-Matrix (JM)

### Join-Biclique (JB)

### Handshaking (HS)
two data stream flow through in opposite direction.

## Data Partitions
NP or P.

### Non-partition (NP)

Pass the tuple by reference (pointer) only.

### Partitioned (P) 
Pass the copy of tuples actually (involve tuple replication). This helps ensure later join processing only touches local datasets.

## Local Joiners

### Symmetric Hash Join (SHJ)

### Progressive Merge Join (PMJ)

### Ripple Join (RJ)
Mostly nested-loop-join

## Current status

Done:
  SHJ Single thread
  SHJ + NP + JM/JB/HS
