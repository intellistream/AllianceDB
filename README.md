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

--

  * **[SIGMOD]** Shuhao Zhang, Yancan Mao, Jiong He, Philipp M. Grulich, Steffen Zeuch, Bingsheng He, Richard T. B. Ma, and Volker Markl. Parallelizing Intra-Window Join on Multicores: An Experimental Study, SIGMOD, 2021
    - Intra-Window Join *[Release at https://github.com/DataSysResearch/AllianceDB/releases/tag/v1]*
  * **[ICDE]** H. Zhang, X. Zeng, S. Zhang, X. Liu, M. Lu and Z. Zheng, "Scalable Online Interval Join on Modern Multicore Processors in OpenMLDB," 2023 IEEE 39th International Conference on Data Engineering (ICDE), Anaheim, CA, USA, 2023, pp. 3031-3042, doi: 10.1109/ICDE55515.2023.00232.
  * **[SIGMOD]** Zeng X, Zhang S, Zhong H, Zhang H, Lu M, Zheng Z, Chen Y. PECJ: Stream Window Join on Disorder Data Streams with Proactive Error Compensation, SIGMOD, 2024
  * **[SIGMOD]** Xilin Tang, Feng Zhang, Shuhao Zhang, Yani Liu, Bingsheng He, Xiaoyong Du. Enabling Adaptive Sampling for Intra-Window Join: Simultaneously Optimizing Quantity and Quality. SIGMOD, 2025
