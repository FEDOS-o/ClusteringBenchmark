@echo off
echo Running S_max sweep tests...

set EXE_PATH=cmake-build-debug\clustering_benchmark.exe

if not exist %EXE_PATH% (
    echo ERROR: clustering_benchmark.exe not found at %EXE_PATH%
    pause
    exit /b 1
)

if not exist results\raw\smax mkdir results\raw\smax

set SMAX_VALUES=16 32 48 64 96 128 192 256
for %%s in (%SMAX_VALUES%) do (
    echo S_max = %%s
    %EXE_PATH% -g 100 -s %%s -a greedy,louvain,glb -o results\raw\smax\smax_%%s.csv
)

echo Done!
pause