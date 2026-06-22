@echo off
echo Running random graph tests...

set EXE_PATH=cmake-build-debug\clustering_benchmark.exe

if not exist %EXE_PATH% (
    echo ERROR: clustering_benchmark.exe not found at %EXE_PATH%
    pause
    exit /b 1
)

if not exist results\raw\random mkdir results\raw\random

set RANDOM_SIZES=100 200 500 1000 2000 5000
set RANDOM_PROBS=0.001 0.005 0.01 0.02 0.05

for %%n in (%RANDOM_SIZES%) do (
    for %%p in (%RANDOM_PROBS%) do (
        echo Random N=%%n, p=%%p
        %EXE_PATH% -r %%n %%p -a greedy,louvain,glb -o results\raw\random\random_%%n_%%p.csv
    )
)

echo Done!
pause