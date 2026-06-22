@echo off
echo Running 2D grid scalability tests...

set EXE_PATH=cmake-build-debug\clustering_benchmark.exe

if not exist %EXE_PATH% (
    echo ERROR: clustering_benchmark.exe not found at %EXE_PATH%
    pause
    exit /b 1
)

if not exist results\raw\grid2d mkdir results\raw\grid2d

set SIZES=20 40 60 80 100 120 140 160 180 200
for %%s in (%SIZES%) do (
    echo Grid %%sx%%s
    %EXE_PATH% -g %%s -a greedy,louvain,glb -o results\raw\grid2d\grid_%%sx%%s.csv
)

echo Done!
pause