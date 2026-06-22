@echo off
setlocal enabledelayedexpansion

echo ============================================================
echo Clustering Benchmark - Full Experiment Suite
echo ============================================================
echo.

REM Путь к исполняемому файлу
set EXE_PATH=cmake-build-debug\clustering_benchmark.exe

REM Проверяем существование exe
if not exist %EXE_PATH% (
    echo ERROR: clustering_benchmark.exe not found at %EXE_PATH%
    echo Please build the project first.
    pause
    exit /b 1
)

REM Создаём папки для результатов
if not exist results mkdir results
if not exist results\raw mkdir results\raw
if not exist results\raw\grid2d mkdir results\raw\grid2d
if not exist results\raw\grid3d mkdir results\raw\grid3d
if not exist results\raw\smax mkdir results\raw\smax
if not exist results\raw\random mkdir results\raw\random
if not exist results\raw\glb_iters mkdir results\raw\glb_iters

REM ============================================================
REM ЭКСПЕРИМЕНТ 1: 2D Grid Scalability (разные размеры)
REM ============================================================
echo [1/5] Running 2D grid scalability tests...
echo ----------------------------------------

set SIZES=20 40 60 80 100 120 140 160 180 200
for %%s in (%SIZES%) do (
    echo   Grid %%sx%%s
    %EXE_PATH% -g %%s -a greedy,louvain,glb -o results\raw\grid2d\grid_%%sx%%s.csv
)

echo.
echo [2/5] Running 3D grid scalability tests...
echo ----------------------------------------

REM ============================================================
REM ЭКСПЕРИМЕНТ 2: 3D Grid Scalability
REM ============================================================
set SIZES_3D=5 10 15 20 25 30
for %%s in (%SIZES_3D%) do (
    echo   Grid 3D %%sx%%sx%%s
    %EXE_PATH% -g %%s -3 --depth %%s -a greedy,louvain,glb -o results\raw\grid3d\grid3d_%%sx%%sx%%s.csv
)

echo.
echo [3/5] Running S_max sweep tests...
echo ----------------------------------------

REM ============================================================
REM ЭКСПЕРИМЕНТ 3: S_max Sweep (влияние размера кластера)
REM ============================================================
set SMAX_VALUES=16 32 48 64 96 128 192 256
for %%s in (%SMAX_VALUES%) do (
    echo   S_max = %%s
    %EXE_PATH% -g 100 -s %%s -a greedy,louvain,glb -o results\raw\smax\smax_%%s.csv
)

echo.
echo [4/5] Running random graph tests...
echo ----------------------------------------

REM ============================================================
REM ЭКСПЕРИМЕНТ 4: Random Graphs (разные размеры и плотности)
REM ============================================================
set RANDOM_SIZES=100 200 500 1000 2000 5000
set RANDOM_PROBS=0.001 0.005 0.01 0.02 0.05

for %%n in (%RANDOM_SIZES%) do (
    for %%p in (%RANDOM_PROBS%) do (
        echo   Random N=%%n, p=%%p
        %EXE_PATH% -r %%n %%p -a greedy,louvain,glb -o results\raw\random\random_%%n_%%p.csv
    )
)

echo.
echo [5/5] Running GLB parameter sweep...
echo ----------------------------------------

REM ============================================================
REM ЭКСПЕРИМЕНТ 5: GLB Local Iterations Sweep
REM ============================================================
set LOCAL_ITERS=0 1 2 3 5 10 20
for %%l in (%LOCAL_ITERS%) do (
    echo   GLB local_iters = %%l
    %EXE_PATH% -g 100 -l %%l -a glb -o results\raw\glb_iters\glb_iters_%%l.csv
)

echo.
echo ============================================================
echo ALL EXPERIMENTS COMPLETED!
echo ============================================================
echo.
echo Results saved to:
echo   results/raw/grid2d/     - 2D grid scalability
echo   results/raw/grid3d/     - 3D grid scalability
echo   results/raw/smax/       - S_max sweep
echo   results/raw/random/     - Random graph tests
echo   results/raw/glb_iters/  - GLB iterations sweep
echo.
echo To generate plots, run: python analyze_results.py
echo ============================================================
pause