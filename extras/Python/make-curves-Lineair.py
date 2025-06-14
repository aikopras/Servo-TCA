import matplotlib.pyplot as plt
import os

# Define curves from C++ code
curves = {
    "lin_A": [(0, 1), (2, 128), (4, 255)],
    "lin_B": [(0, 255), (2, 128), (4, 0)],
}

curve_indices = {
    name: idx for idx, name in enumerate([
        "lin_A", "lin_B"
    ])
}

os.makedirs("curves", exist_ok=True)

for name, points in curves.items():
    # Convert into milliseconds and remove {0,0} endpoint
    points = [p for p in points if p != (0, 0)]
    x_vals = [x * 20 for x, _ in points]
    y_vals = [y for _, y in points]

    plt.figure(figsize=(10.24, 4.8))  # 1024x480px
    plt.grid(True, which='both', linestyle=':', linewidth=0.5, color='gray')

    # Blue straignt line
    plt.plot(x_vals, y_vals, color='blue', linewidth=1, linestyle='-')


    plt.title(f"{name} ({curve_indices.get(name, '?')})", fontsize=14)
    plt.xlabel("time (ms)")
    plt.ylabel("position")
    plt.ylim(0, 256)
    plt.xticks(range(0, max(x_vals)+1, 20))
    plt.yticks(range(0, 257, 32))
    plt.tight_layout()

    plt.savefig(f"curves/{name}.png")
    plt.close()

print("✅ All pictures stored in 'curves'.")
