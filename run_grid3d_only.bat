@echo off
echo Running 3D grid scalability tests...

set EXE_PATH=cmake-build-debug\clustering_benchmark.exe

if not exist %EXE_PATH% (
    echo ERROR: clustering_benchmark.exe not found at %EXE_PATH%
    pause
    exit /b 1
)

if not exist results\raw\grid3d mkdir results\raw\grid3d

set SIZES_3D=5 10 15 20 25 30
for %%s in (%SIZES_3D%) do (
    echo Grid 3D %%sx%%sx%%s
    %EXE_PATH% -g %%s -3 --depth %%s -a greedy,louvain,glb -o results\raw\grid3d\grid3d_%%sx%%sx%%s.csv
)

echo Done!
pause