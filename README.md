# High Performance Computing - Mandelbrot

This repository contains sequential, OpenMP, MPI, and hybrid implementations of the Mandelbrot set generation using brute-force and Mariani-Silver algorithms.

## Compilation and Local Execution

* **Compile all executables:**
  ```bash
  make all
  ```
  This compiles all binaries and places them in the `build/` directory.

* **Run diagnostic tests:**
  ```bash
  make smoke
  ```
  This runs a baseline validation suite on the generated binaries to check if they compute the expected pixel counts and checksums.

* **View generated images:**
  ```bash
  make show_image
  ```
  This opens all generated images located in the `img/` folder. Note that this command requires a Linux environment with `xdg-open` installed.

* **Clean build files:**
  ```bash
  make clean
  ```

---

### Running MPI and Hybrid Binaries

For MPI and hybrid implementations, run the binaries using the `mpirun` command specifying the number of processes:
```bash
mpirun -n <num_processes> ./build/<binary_name>
```

> **Note:** All compiled executables support a `--help` flag (e.g., `./build/mandel_mariani_seq --help`) to display the complete list of configurable command-line options and parameters.

---
## Slurm Benchmarking

### Basic Verification
Schedule analyses:
```bash
sbatch slurm/helloworld.sh
```

### Benchmarks and Profiling
Profiling and scaling tests:
```bash
sbatch slurm/mismatch.sh
sbatch slurm/parameter.sh
sbatch slurm/schedule.sh
sbatch slurm/strong_scaling.sh
```

### Partition-Dependent Launchers
The following launcher scripts must be run as shell scripts:
* `sh slurm/launcher_container.sh`
* `sh slurm/launcher_native.sh`
* `sh slurm/weak_scaling_launcher.sh`

**Note:** Before running these launchers, ensure that you compile the source code inside the targeted cluster partition.

## Container Image Build

To build the Singularity container image (`mandelbrot.sif`) from the provided definition file (`mandelbrot.def`), execute the following command:

```bash
singularity build --fakeroot mandelbrot.sif mandelbrot.def
```
