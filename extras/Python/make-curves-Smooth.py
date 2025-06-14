import matplotlib.pyplot as plt
import os

# Define curves from C++ code
curves = {
    "move_A": [(0, 0), (1, 5), (2, 17), (3, 37), (4, 64), (6, 128), (8, 192), (9, 218), (10, 238), (11, 250), (12, 255)],
    "move_B": [(0, 255), (1, 250), (2, 238), (3, 218), (4, 192), (6, 128), (8, 64), (9, 37), (10, 17), (11, 5), (12,0)],
    "sine_A": [(0, 128), (3, 186), (5, 218), (7, 241), (8, 249), (9, 253), (10, 255), (11, 253), (12, 249), (13, 241), (15, 218), (17, 186), (20, 128)],
    "sine_B": [(0, 128), (3, 70), (5, 38), (7, 15), (8, 7), (9, 3), (10, 1), (11, 3), (12, 7), (13, 15), (15, 38), (17, 70), (20, 128)],
    "whip_A": [(0, 128), (1, 135), (2, 145), (3, 157), (4, 172), (5, 189), (6, 208), (8, 255), (10, 208), (11, 189), (12, 172), (13, 157), (14, 145), (15, 135), (16, 128)],
    "whip_B": [(0, 128), (1, 121), (2, 111), (3, 99), (4, 84), (5, 67), (6, 48), (8, 1), (10, 48), (11, 67), (12, 84), (13, 99), (14, 111), (15, 121), (16, 128)],
    "sig_hp0": [(0,230), (1,224), (7,140), (9,89), (10,51), (11,26), (12,16), (13,15), (15,29), (16,33), (17,31), (19,21), (20,20), (22,27), (23,29), (25,26), (26,23), (27,23), (29,26), (30,27), (31,27), (33,24), (35,25)],
    "sig_hp1": [(0,26), (11,115), (13,122), (16,128), (28,122), (40,230), (41,239), (42,240), (44,226), (45,222), (46,224), (48,234), (49,235), (51,228), (52,226), (54,230), (55,232), (56,232), (58,229), (59,228), (60,228), (62,231), (64,230)],
    "hp1p": [(0,26), (11,115), (13,122), (16,128), (28,122), (40,230), (41,239), (42,240), (44,226), (45,222), (46,224), (48,234), (49,235), (51,228), (52,226), (54,230), (55,232), (56,232), (58,229), (59,228), (60,228), (62,231), (64,230)],
    "sine_AB": [(0,128), (3,186), (5,218), (7,241), (9,253), (10,255), (11,253), (13,241), (15,218), (17,186), (20,128), (23,70), (25,38), (27,15), (29,3), (30,1), (31,3), (33,15), (35,38), (37,70), (40,128)],
}

curve_indices = {
    name: idx for idx, name in enumerate([
        "move_A", "move_B",
        "sine_A", "sine_B", "whip_A", "whip_B",
        "sig_hp0", "sig_hp1", "hp1p", "sine_AB"
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

    # Black smooth line (interpolate using spline)
    try:
        from scipy.interpolate import make_interp_spline
        import numpy as np
        x_smooth = np.linspace(min(x_vals), max(x_vals), 300)
        spline = make_interp_spline(x_vals, y_vals)
        y_smooth = spline(x_smooth)
        plt.plot(x_smooth, y_smooth, color='black', linewidth=2)
    except ImportError:
        plt.plot(x_vals, y_vals, color='black', linewidth=2)

    plt.title(f"{name} ({curve_indices.get(name, '?') + 2})", fontsize=14)
    plt.xlabel("time (ms)")
    plt.ylabel("position")
    plt.ylim(0, 256)
    plt.xticks(range(0, max(x_vals)+1, 200))
    plt.yticks(range(0, 257, 32))
    plt.tight_layout()

    plt.savefig(f"curves/{name}.png")
    plt.close()

print("✅ All pictures stored in 'curves'.")
