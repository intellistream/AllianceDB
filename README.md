<p align="left">
  <a href="https://github.com/ShuhaoZhangTony/AllianceDB/actions">
    <img alt="GitHub Actions status" src="https://github.com/ShuhaoZhangTony/AllianceDB/workflows/Main%20workflow/badge.svg"></a>
</p>

# Stream Joins on Modern Multicore Processors


## Installation

apt-get install libnuma-dev


## Workload Distribution

### Join-Matrix (JM)

### Join-Biclique (JB)

### Handshaking (HS)
two data stream flow through in opposite direction.

## Partition Schemes

### Input data Partition

- Pass the tuple by reference (pointer) only.
- Pass the copy of tuples actually (involve tuple replication). This helps ensure later join processing only touches local datasets.

### Data Structure Partition

- whether we physically split data structure into sub-copies.

## Local Joiners

### Symmetric Hash Join (SHJ)

### Progressive Merge Join (PMJ)

### Ripple Join (RJ)
Mostly nested-loop-join

