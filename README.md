# AllianceDB

AllianceDB is an open-source benchmark suite for evaluating stream join
algorithms on multicore hardware. It collects reproducibility artifacts for
several related join studies; it is a DataSys benchmark suite, not a single
production database runtime.

## Project status

The root scripts coordinate multiple historical experiment branches and
hardware-sensitive measurements. Start with the
[artifact documentation](docs/README.md) and review its CPU, NUMA, and
privilege requirements before running experiments.

The repository is licensed under the MIT License. Bundled third-party source
files retain their upstream notices.

## Publications

If you use AllianceDB in your work, please cite our work.

- **[SIGMOD 2021]** Shuhao Zhang, Yancan Mao, Jiong He, Philipp M.
  Grulich, Steffen Zeuch, Bingsheng He, Richard T. B. Ma, and Volker Markl.
  [Parallelizing Intra-Window Join on Multicores: An Experimental
  Study](https://doi.org/10.1145/3448016.3452793).
  [Artifact release](https://github.com/DataSysResearch/AllianceDB/releases/tag/v1).
- **[ICDE 2023]** Hao Zhang, Xianzhi Zeng, Shuhao Zhang, Xinyi Liu, Mian
  Lu, and Zhao Zheng. [Scalable Online Interval Join on Modern Multicore
  Processors in OpenMLDB](https://doi.org/10.1109/ICDE55515.2023.00232).
- **[SIGMOD 2024]** Xianzhi Zeng, Shuhao Zhang, Hongbin Zhong, Hao Zhang,
  Mian Lu, Zhao Zheng, and Yuqiang Chen. [PECJ: Stream Window Join on
  Disorder Data Streams with Proactive Error
  Compensation](https://doi.org/10.1145/3639268).
- **[PACMMOD 2024 / SIGMOD 2025]** Xilin Tang, Feng Zhang, Shuhao Zhang,
  Yani Liu, Bingsheng He, and Xiaoyong Du. [Enabling Adaptive Sampling for
  Intra-Window Join: Simultaneously Optimizing Quantity and
  Quality](https://doi.org/10.1145/3677134).
