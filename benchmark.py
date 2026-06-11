import subprocess
import time
import math
import sys

def benchmark(executable_path):
    times = []
    checksums = set()
    
    for i in range(100):
        t0 = time.perf_counter()
        # Eseguiamo il file passando la variabile 'executable_path' direttamente alla lista
        res = subprocess.run([executable_path, "--output", "img/mandelbrot.pgm"], capture_output=True, text=True)
        t1 = time.perf_counter()
        times.append(t1 - t0)
        
        for line in res.stdout.splitlines():
            if "iteration_checksum" in line:
                checksums.add(line.split()[1])
   
    mean_t = sum(times) / len(times)
    var_t = sum((x - mean_t)**2 for x in times) / len(times)
    std_t = math.sqrt(var_t)
   
    print(f"    time:    {mean_t:.6f} s")
    print(f"    std dev: {std_t:.6f} s")
   
    if len(checksums) == 0:
        print("    [ERROR] No checksum in the output!")
    elif len(checksums) > 1:
        print(f"    [ATTENTION] There are different checksums! Found: {checksums}")
    else:
        chk = list(checksums)[0]
        print(f"    Checksum:     {chk}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 benchmark.py <executable_path>")
        sys.exit(1)
        
    benchmark(sys.argv[1])