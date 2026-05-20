import asyncio
import time
import subprocess
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

TARGET_IP = '127.0.0.1'
TARGET_PORT = 8080
DELAY_BETWEEN_MSGS = 0.005

QUEUE_SIZES = np.linspace(10, 300, num=10).astype(int)
CLIENT_COUNTS = np.linspace(50, 400, num=10).astype(int)
MESSAGES_PER_CLIENT_ARRAY = np.linspace(10, 200, num=10).astype(int)

async def simulate_client(client_id, msg_count):
    try:
        reader, writer = await asyncio.open_connection(TARGET_IP, TARGET_PORT)

        msg = f"Client {client_id} payload transaction."
        header = f"{len(msg):04d}".encode()
        packet = header + msg.encode()

        await asyncio.sleep(0.02)

        for _ in range(msg_count):
            writer.write(packet)
            await writer.drain()
            await asyncio.sleep(DELAY_BETWEEN_MSGS)

        writer.close()
        await writer.wait_closed()

        return True

    except:
        return False

async def run_stress_blitz(num_clients, msg_count):
    tasks = [simulate_client(i, msg_count) for i in range(num_clients)]

    start = time.time()

    results = await asyncio.gather(*tasks)

    duration = time.time() - start
    success = sum(results)

    return success, duration

def main():
    print("==================================================")
    print(" STARTING DYNAMIC MATRIX BENCHMARK (NUMPY MODE)  ")
    print("==================================================\n")

    benchmark_data = []

    for q_size in QUEUE_SIZES:
        for c_count in CLIENT_COUNTS:
            for m_per_client in MESSAGES_PER_CLIENT_ARRAY:

                print(
                    f"--> Grid Slot: Backlog={q_size}, "
                    f"Clients={c_count}, Msgs/Client={m_per_client}..."
                )

                server_proc = subprocess.Popen(
                    ["./server", str(q_size)],
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL
                )

                time.sleep(2.0)

                success, duration = asyncio.run(
                    run_stress_blitz(c_count, int(m_per_client))
                )

                server_proc.terminate()
                server_proc.wait()

                time.sleep(0.4)

                success_rate = (success / c_count) * 100
                total_msgs = success * m_per_client
                throughput = total_msgs / duration if duration > 0 else 0

                benchmark_data.append({
                    "Queue_Size": q_size,
                    "Attempted_Clients": c_count,
                    "Msgs_Per_Client": m_per_client,
                    "Successful_Clients": success,
                    "Success_Rate_Pct": success_rate,
                    "Total_Messages": total_msgs,
                    "Duration_Sec": duration,
                    "Throughput_Msg_Sec": throughput
                })

    df = pd.DataFrame(benchmark_data)

    df.to_csv("tests/benchmark_results.csv", index=False)

    print(
        "\n[System] Evaluation complete. "
        "DataFrame written to 'tests/benchmark_results.csv'"
    )

    plt.style.use(
        'seaborn-v0_8-whitegrid'
        if 'seaborn-v0_8-whitegrid' in plt.style.available
        else 'default'
    )

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))

    summary_df = (
        df.groupby(["Queue_Size", "Attempted_Clients"])
        .mean()
        .reset_index()
    )

    for q_size in QUEUE_SIZES:
        sub_df = summary_df[
            summary_df["Queue_Size"] == q_size
        ]

        ax1.plot(
            sub_df["Attempted_Clients"],
            sub_df["Throughput_Msg_Sec"],
            marker='o',
            linewidth=2,
            label=f"Backlog Queue: {q_size}"
        )

        ax2.plot(
            sub_df["Attempted_Clients"],
            sub_df["Success_Rate_Pct"],
            marker='s',
            linestyle='--',
            linewidth=2,
            label=f"Backlog Queue: {q_size}"
        )

    ax1.set_title(
        "System Throughput Scaling Spectrum",
        fontsize=12,
        fontweight='bold'
    )

    ax1.set_xlabel(
        "Concurrent Client Connections (linspace range)",
        fontsize=10
    )

    ax1.set_ylabel(
        "Throughput (Mean Messages / Sec)",
        fontsize=10
    )

    ax1.legend()

    ax2.set_title(
        "Connection Success Rate Decay Spectrum",
        fontsize=12,
        fontweight='bold'
    )

    ax2.set_xlabel(
        "Concurrent Client Connections (linspace range)",
        fontsize=10
    )

    ax2.set_ylabel(
        "Success Rate (%)",
        fontsize=10
    )

    ax2.set_ylim(-5, 105)

    ax2.legend()

    plt.tight_layout()

    plt.savefig(
        "tests/charts_raw_sockets.png",
        dpi=300
    )

    print(
        "[System] Visualizations saved completely to "
        "'tests/charts_raw_sockets.png'"
    )

if __name__ == "__main__":
    main()