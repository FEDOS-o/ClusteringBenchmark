import pandas as pd
import matplotlib.pyplot as plt
import os
import numpy as np

# Настройка стиля
plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams['figure.figsize'] = (10, 6)
plt.rcParams['font.size'] = 11
plt.rcParams['axes.titlesize'] = 12
plt.rcParams['axes.labelsize'] = 11


def load_all_results(raw_dir='results/raw'):
    """Загружает все CSV файлы из всех подпапок рекурсивно"""
    all_files = []

    for root, dirs, files in os.walk(raw_dir):
        for f in files:
            if f.endswith('.csv'):
                all_files.append(os.path.join(root, f))

    if not all_files:
        print(f"No CSV files found in {raw_dir}")
        return None

    dfs = []
    for f in all_files:
        try:
            df = pd.read_csv(f)
            filename = os.path.basename(f).replace('.csv', '')
            df['filename'] = filename
            df['filepath'] = f
            dfs.append(df)
        except Exception as e:
            print(f"Warning: Could not read {f}: {e}")

    if not dfs:
        return None

    combined = pd.concat(dfs, ignore_index=True)
    print(f"Loaded {len(combined)} rows from {len(all_files)} files")
    return combined


def extract_parameters(df):
    """Извлекает параметры экспериментов из имени файла"""

    # Для 2D сетки: grid_100x100
    df['grid_size'] = df['filename'].str.extract(r'grid_(\d+)x\1').astype(float)
    df['grid_size'] = df['grid_size'].fillna(0)

    # Для 3D сетки: grid3d_10x10x10
    df['grid3d_size'] = df['filename'].str.extract(r'grid3d_(\d+)x\1x\1').astype(float)
    df['grid3d_size'] = df['grid3d_size'].fillna(0)

    # Для S_max: smax_64
    df['smax'] = df['filename'].str.extract(r'smax_(\d+)').astype(float)
    df['smax'] = df['smax'].fillna(0)

    # Для GLB итераций: glb_iters_3
    df['glb_iters'] = df['filename'].str.extract(r'glb_iters_(\d+)').astype(float)
    df['glb_iters'] = df['glb_iters'].fillna(0)

    # Для случайных графов: random_1000_0.01
    df['random_n'] = df['filename'].str.extract(r'random_(\d+)_').astype(float)
    df['random_n'] = df['random_n'].fillna(0)
    df['random_p'] = df['filename'].str.extract(r'random_\d+_(\d+\.?\d*)').astype(float)
    df['random_p'] = df['random_p'].fillna(0)

    # Определяем тип эксперимента
    df['exp_type'] = 'unknown'
    df.loc[df['grid_size'] > 0, 'exp_type'] = 'grid2d'
    df.loc[df['grid3d_size'] > 0, 'exp_type'] = 'grid3d'
    df.loc[df['smax'] > 0, 'exp_type'] = 'smax'
    df.loc[df['random_n'] > 0, 'exp_type'] = 'random'
    df.loc[df['glb_iters'] > 0, 'exp_type'] = 'glb_iters'

    # По пути
    df.loc[df['filepath'].str.contains('grid2d', case=False), 'exp_type'] = 'grid2d'
    df.loc[df['filepath'].str.contains('grid3d', case=False), 'exp_type'] = 'grid3d'
    df.loc[df['filepath'].str.contains('smax', case=False), 'exp_type'] = 'smax'
    df.loc[df['filepath'].str.contains('random', case=False), 'exp_type'] = 'random'
    df.loc[df['filepath'].str.contains('glb_iters', case=False), 'exp_type'] = 'glb_iters'
    df.loc[df['filename'].str.contains('test', case=False), 'exp_type'] = 'test'

    return df


def safe_groupby_mean(df, group_cols):
    """Безопасная группировка: усредняет только числовые колонки"""
    numeric_cols = df.select_dtypes(include=[np.number]).columns.tolist()
    # Убираем из числовых колонок те, что уже есть в group_cols
    for col in group_cols:
        if col in numeric_cols:
            numeric_cols.remove(col)
    result = df.groupby(group_cols)[numeric_cols].mean().reset_index()
    return result


def plot_grid2d_scalability(df, output_dir='results/plots'):
    """График масштабируемости 2D сетки"""
    data = df[df['exp_type'] == 'grid2d']
    if data.empty:
        print("No 2D grid data found")
        return

    os.makedirs(output_dir, exist_ok=True)
    grouped = safe_groupby_mean(data, ['algorithmName', 'grid_size'])

    fig, axes = plt.subplots(1, 3, figsize=(15, 5))

    metrics = [
        ('executionTimeMs', 'Time (ms)', axes[0]),
        ('greedyColorsEstimate', 'K (colors)', axes[1]),
        ('cut', 'Cut', axes[2])
    ]

    colors = {'greedy': '#2E86AB', 'louvain': '#A23B72', 'glb': '#F18F01'}

    for metric, ylabel, ax in metrics:
        for algo in ['greedy', 'louvain', 'glb']:
            algo_data = grouped[grouped['algorithmName'] == algo]
            if not algo_data.empty:
                ax.plot(algo_data['grid_size'], algo_data[metric],
                        marker='o', label=algo, color=colors.get(algo, 'black'))
        ax.set_xlabel('Grid size (N x N)')
        ax.set_ylabel(ylabel)
        ax.legend()
        ax.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'grid2d_scalability.png'), dpi=150)
    plt.close()
    print(f"Saved grid2d_scalability.png")


def plot_grid3d_scalability(df, output_dir='results/plots'):
    """График масштабируемости 3D сетки"""
    data = df[df['exp_type'] == 'grid3d']
    if data.empty:
        print("No 3D grid data found")
        return

    os.makedirs(output_dir, exist_ok=True)
    grouped = safe_groupby_mean(data, ['algorithmName', 'grid3d_size'])

    fig, axes = plt.subplots(1, 2, figsize=(12, 5))

    colors = {'greedy': '#2E86AB', 'louvain': '#A23B72', 'glb': '#F18F01'}

    for algo in ['greedy', 'louvain', 'glb']:
        algo_data = grouped[grouped['algorithmName'] == algo]
        if not algo_data.empty:
            axes[0].plot(algo_data['grid3d_size'], algo_data['executionTimeMs'],
                         marker='o', label=algo, color=colors.get(algo))
            axes[1].plot(algo_data['grid3d_size'], algo_data['greedyColorsEstimate'],
                         marker='s', label=algo, color=colors.get(algo))

    axes[0].set_xlabel('Grid size (N x N x N)')
    axes[0].set_ylabel('Time (ms)')
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)
    axes[0].set_title('Execution Time')

    axes[1].set_xlabel('Grid size (N x N x N)')
    axes[1].set_ylabel('K (colors)')
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)
    axes[1].set_title('Colors')

    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'grid3d_scalability.png'), dpi=150)
    plt.close()
    print(f"Saved grid3d_scalability.png")


def plot_smax_sweep(df, output_dir='results/plots'):
    """График влияния S_max"""
    data = df[df['exp_type'] == 'smax']
    if data.empty:
        print("No S_max sweep data found")
        return

    os.makedirs(output_dir, exist_ok=True)
    grouped = safe_groupby_mean(data, ['algorithmName', 'smax'])

    fig, axes = plt.subplots(1, 2, figsize=(12, 5))

    colors = {'greedy': '#2E86AB', 'louvain': '#A23B72', 'glb': '#F18F01'}

    for algo in ['greedy', 'louvain', 'glb']:
        algo_data = grouped[grouped['algorithmName'] == algo]
        if not algo_data.empty:
            axes[0].plot(algo_data['smax'], algo_data['greedyColorsEstimate'],
                         marker='o', label=algo, color=colors.get(algo))
            axes[1].plot(algo_data['smax'], algo_data['coefficientOfVariation'],
                         marker='s', label=algo, color=colors.get(algo))

    axes[0].set_xlabel('S_max (max cluster size)')
    axes[0].set_ylabel('K (colors)')
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)
    axes[0].set_title('Colors vs S_max')

    axes[1].set_xlabel('S_max (max cluster size)')
    axes[1].set_ylabel('CV (balance)')
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)
    axes[1].set_title('Balance vs S_max')

    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'smax_sweep.png'), dpi=150)
    plt.close()
    print(f"Saved smax_sweep.png")


def plot_random_graphs(df, output_dir='results/plots'):
    """График для случайных графов"""
    data = df[df['exp_type'] == 'random']
    if data.empty:
        print("No random graph data found")
        return

    os.makedirs(output_dir, exist_ok=True)
    grouped = safe_groupby_mean(data, ['algorithmName', 'random_n', 'random_p'])

    fig, axes = plt.subplots(1, 2, figsize=(12, 5))

    algo_colors = {'greedy': '#2E86AB', 'louvain': '#A23B72', 'glb': '#F18F01'}
    markers = {0.001: 'o', 0.005: 's', 0.01: '^', 0.02: 'D', 0.05: '*'}

    for p, marker in markers.items():
        p_data = grouped[abs(grouped['random_p'] - p) < 0.0001]
        if p_data.empty:
            continue
        for algo in ['greedy', 'louvain', 'glb']:
            algo_data = p_data[p_data['algorithmName'] == algo]
            if not algo_data.empty:
                label = f'{algo} p={p}'
                axes[0].plot(algo_data['random_n'], algo_data['executionTimeMs'],
                             marker=marker, label=label, color=algo_colors.get(algo, 'black'))
                axes[1].plot(algo_data['random_n'], algo_data['greedyColorsEstimate'],
                             marker=marker, label=label, color=algo_colors.get(algo, 'black'))

    axes[0].set_xlabel('Number of vertices')
    axes[0].set_ylabel('Time (ms)')
    axes[0].legend(loc='upper left', fontsize=8)
    axes[0].grid(True, alpha=0.3)
    axes[0].set_title('Execution Time')

    axes[1].set_xlabel('Number of vertices')
    axes[1].set_ylabel('K (colors)')
    axes[1].legend(loc='upper left', fontsize=8)
    axes[1].grid(True, alpha=0.3)
    axes[1].set_title('Colors')

    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'random_graphs.png'), dpi=150)
    plt.close()
    print(f"Saved random_graphs.png")


def plot_glb_iters(df, output_dir='results/plots'):
    """График влияния итераций GLB"""
    data = df[df['exp_type'] == 'glb_iters']
    if data.empty:
        print("No GLB iterations data found")
        return

    os.makedirs(output_dir, exist_ok=True)
    grouped = safe_groupby_mean(data, ['glb_iters'])

    fig, axes = plt.subplots(1, 3, figsize=(15, 5))

    axes[0].plot(grouped['glb_iters'], grouped['greedyColorsEstimate'],
                 marker='o', linestyle='-', color='#2E86AB', linewidth=2)
    axes[0].set_xlabel('Local optimization iterations')
    axes[0].set_ylabel('K (colors)')
    axes[0].grid(True, alpha=0.3)
    axes[0].set_title('Colors vs Iterations')

    axes[1].plot(grouped['glb_iters'], grouped['executionTimeMs'],
                 marker='s', linestyle='-', color='#F18F01', linewidth=2)
    axes[1].set_xlabel('Local optimization iterations')
    axes[1].set_ylabel('Time (ms)')
    axes[1].grid(True, alpha=0.3)
    axes[1].set_title('Time vs Iterations')

    axes[2].plot(grouped['glb_iters'], grouped['coefficientOfVariation'],
                 marker='^', linestyle='-', color='#A23B72', linewidth=2)
    axes[2].set_xlabel('Local optimization iterations')
    axes[2].set_ylabel('CV (balance)')
    axes[2].grid(True, alpha=0.3)
    axes[2].set_title('Balance vs Iterations')

    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'glb_iters_sweep.png'), dpi=150)
    plt.close()
    print(f"Saved glb_iters_sweep.png")


def plot_algorithms_comparison(df, output_dir='results/plots'):
    """Сравнение алгоритмов по всем метрикам"""
    os.makedirs(output_dir, exist_ok=True)

    df_filtered = df[df['exp_type'] != 'test']
    numeric_cols = df_filtered.select_dtypes(include=[np.number]).columns
    algo_means = df_filtered.groupby('algorithmName')[numeric_cols].mean().reset_index()

    if algo_means.empty:
        print("No data for algorithms comparison")
        return

    metrics = [
        ('executionTimeMs', 'Time (ms)'),
        ('greedyColorsEstimate', 'K (colors)'),
        ('cut', 'Cut'),
        ('coefficientOfVariation', 'CV (balance)'),
        ('warpEfficiency', 'Warp Efficiency'),
        ('loadBalanceScore', 'Load Balance')
    ]

    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    axes = axes.flatten()

    colors = {'greedy': '#2E86AB', 'louvain': '#A23B72', 'glb': '#F18F01'}

    for idx, (metric, label) in enumerate(metrics):
        if metric not in algo_means.columns:
            continue
        bars = axes[idx].bar(
            algo_means['algorithmName'],
            algo_means[metric],
            color=[colors.get(a, 'gray') for a in algo_means['algorithmName']]
        )
        axes[idx].set_title(label)
        axes[idx].grid(True, alpha=0.3)
        for bar in bars:
            height = bar.get_height()
            axes[idx].text(bar.get_x() + bar.get_width()/2., height,
                           f'{height:.2f}', ha='center', va='bottom', fontsize=9)

    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'algorithms_comparison.png'), dpi=150)
    plt.close()
    print(f"Saved algorithms_comparison.png")


def plot_k_vs_cut(df, output_dir='results/plots'):
    """Scatter plot: K vs Cut"""
    os.makedirs(output_dir, exist_ok=True)

    colors = {'greedy': '#2E86AB', 'louvain': '#A23B72', 'glb': '#F18F01'}

    fig, ax = plt.subplots(figsize=(10, 6))

    for algo in ['greedy', 'louvain', 'glb']:
        algo_data = df[df['algorithmName'] == algo]
        if not algo_data.empty:
            ax.scatter(
                algo_data['greedyColorsEstimate'],
                algo_data['cut'],
                label=algo,
                color=colors.get(algo, 'gray'),
                alpha=0.6,
                s=50
            )

    ax.set_xlabel('K (colors)')
    ax.set_ylabel('Cut')
    ax.legend()
    ax.grid(True, alpha=0.3)
    ax.set_title('K vs Cut: Trade-off Analysis')

    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'k_vs_cut.png'), dpi=150)
    plt.close()
    print(f"Saved k_vs_cut.png")


def plot_time_vs_quality(df, output_dir='results/plots'):
    """Scatter plot: Time vs Quality (1/(1+CV))"""
    os.makedirs(output_dir, exist_ok=True)

    colors = {'greedy': '#2E86AB', 'louvain': '#A23B72', 'glb': '#F18F01'}

    fig, ax = plt.subplots(figsize=(10, 6))

    for algo in ['greedy', 'louvain', 'glb']:
        algo_data = df[df['algorithmName'] == algo]
        if not algo_data.empty:
            quality = 1.0 / (1.0 + algo_data['coefficientOfVariation'])
            ax.scatter(
                quality,
                algo_data['executionTimeMs'],
                label=algo,
                color=colors.get(algo, 'gray'),
                alpha=0.6,
                s=50
            )

    ax.set_xlabel('Quality (1/(1+CV))')
    ax.set_ylabel('Time (ms)')
    ax.legend()
    ax.grid(True, alpha=0.3)
    ax.set_title('Time vs Quality Trade-off')

    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, 'time_vs_quality.png'), dpi=150)
    plt.close()
    print(f"Saved time_vs_quality.png")


def generate_summary_table(df, output_dir='results/summary'):
    """Генерирует сводную таблицу"""
    os.makedirs(output_dir, exist_ok=True)

    numeric_cols = df.select_dtypes(include=[np.number]).columns
    summary = df.groupby('algorithmName')[numeric_cols].agg(['mean', 'std', 'min', 'max']).round(3)

    # Упрощаем названия колонок
    summary.columns = ['_'.join(col).strip() for col in summary.columns.values]

    summary.to_csv(os.path.join(output_dir, 'summary_table.csv'))
    print(f"Saved summary_table.csv to {output_dir}")

    print("\n" + "="*80)
    print("SUMMARY TABLE")
    print("="*80)
    print(summary.to_string())
    print("="*80)

    return summary


def main():
    print("="*80)
    print("CLUSTERING BENCHMARK - RESULTS ANALYZER")
    print("="*80)
    print()

    df = load_all_results('results/raw')

    if df is None or df.empty:
        print("\nNo data found. Please run experiments first:")
        print("  - run_quick_test.bat")
        print("  - run_all_experiments.bat")
        return

    print(f"Total rows: {len(df)}")

    df = extract_parameters(df)

    print("\n" + "="*80)
    print("EXPERIMENT STATISTICS")
    print("="*80)
    print(f"\nExperiment types:")
    for exp_type, count in df['exp_type'].value_counts().items():
        print(f"  {exp_type}: {count} rows")
    print(f"\nAlgorithms:")
    for algo, count in df['algorithmName'].value_counts().items():
        print(f"  {algo}: {count} rows")

    os.makedirs('results/plots', exist_ok=True)
    os.makedirs('results/summary', exist_ok=True)

    print("\n" + "="*80)
    print("GENERATING PLOTS")
    print("="*80)

    plot_grid2d_scalability(df)
    plot_grid3d_scalability(df)
    plot_smax_sweep(df)
    plot_random_graphs(df)
    plot_glb_iters(df)
    plot_algorithms_comparison(df)
    plot_k_vs_cut(df)
    plot_time_vs_quality(df)

    print("\n" + "="*80)
    print("GENERATING SUMMARY")
    print("="*80)
    summary = generate_summary_table(df)

    print("\n" + "="*80)
    print("ANALYSIS COMPLETE!")
    print("="*80)
    print("\nResults saved to:")
    print("  - results/plots/     (all graphs)")
    print("    * grid2d_scalability.png")
    print("    * grid3d_scalability.png")
    print("    * smax_sweep.png")
    print("    * random_graphs.png")
    print("    * glb_iters_sweep.png")
    print("    * algorithms_comparison.png")
    print("    * k_vs_cut.png")
    print("    * time_vs_quality.png")
    print("  - results/summary/   (summary tables)")
    print("    * summary_table.csv")


if __name__ == '__main__':
    main()