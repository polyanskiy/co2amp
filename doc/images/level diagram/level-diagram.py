import matplotlib.pyplot as plt

# -----------------------------------------
# CO2 vibrational levels
# G values are in units of 1e14 Hz
# -----------------------------------------
levels = [
    # Ground state
    {"family": "GS",              "column": 1, "label": r"Ground state",              "state": "00001e", "G": 0.000},

    # nu3 only
    {"family": "nu3",             "column": 1, "label": r"$\nu_3$",                    "state": "00011e", "G": 0.704},
    {"family": "2nu3",            "column": 1, "label": r"$2\nu_3$",                   "state": "00021e", "G": 1.401},
    {"family": "3nu3",            "column": 1, "label": r"$3\nu_3$",                   "state": "00031e", "G": 2.090},

    # nu2 only / Fermi polyads
    {"family": "nu2",             "column": 2, "label": r"$\nu_2$",                    "state": "01101e", "G": 0.200},

    {"family": "2nu2",            "column": 2, "label": r"$2\nu_2$",                   "state": "10001e", "G": 0.416},
    {"family": "2nu2",            "column": 2, "label": r"$2\nu_2$",                   "state": "10002e", "G": 0.385},
    {"family": "2nu2",            "column": 2, "label": r"$2\nu_2$",                   "state": "02201e", "G": 0.400},

    {"family": "3nu2",            "column": 2, "label": r"$3\nu_2$",                   "state": "11101e", "G": 0.623},
    {"family": "3nu2",            "column": 2, "label": r"$3\nu_2$",                   "state": "11102e", "G": 0.579},
    {"family": "3nu2",            "column": 2, "label": r"$3\nu_2$",                   "state": "03301e", "G": 0.601},

    {"family": "4nu2",            "column": 2, "label": r"$4\nu_2$",                   "state": "20001e", "G": 0.839},
    {"family": "4nu2",            "column": 2, "label": r"$4\nu_2$",                   "state": "20002e", "G": 0.801},
    {"family": "4nu2",            "column": 2, "label": r"$4\nu_2$",                   "state": "20003e", "G": 0.764},
    {"family": "4nu2",            "column": 2, "label": r"$4\nu_2$",                   "state": "12201e", "G": 0.828},
    {"family": "4nu2",            "column": 2, "label": r"$4\nu_2$",                   "state": "12202e", "G": 0.775},
    {"family": "4nu2",            "column": 2, "label": r"$4\nu_2$",                   "state": "04401e", "G": 0.801},

    # mixed nu2 + nu3
    {"family": "nu2+nu3",         "column": 3, "label": r"$\nu_2+\nu_3$",              "state": "01111e", "G": 0.901},

    {"family": "2nu2+nu3",        "column": 3, "label": r"$2\nu_2+\nu_3$",             "state": "10011e", "G": 1.114},
    {"family": "2nu2+nu3",        "column": 3, "label": r"$2\nu_2+\nu_3$",             "state": "10012e", "G": 1.083},
    {"family": "2nu2+nu3",        "column": 3, "label": r"$2\nu_2+\nu_3$",             "state": "02211e", "G": 1.097},

    {"family": "3nu2+nu3",        "column": 3, "label": r"$3\nu_2+\nu_3$",             "state": "11111e", "G": 1.316},
    {"family": "3nu2+nu3",        "column": 3, "label": r"$3\nu_2+\nu_3$",             "state": "11112e", "G": 1.273},
    {"family": "3nu2+nu3",        "column": 3, "label": r"$3\nu_2+\nu_3$",             "state": "03311e", "G": 1.294},

    {"family": "4nu2+nu3",        "column": 3, "label": r"$4\nu_2+\nu_3$",             "state": "20011e", "G": 1.529},
    {"family": "4nu2+nu3",        "column": 3, "label": r"$4\nu_2+\nu_3$",             "state": "20012e", "G": 1.492},
    {"family": "4nu2+nu3",        "column": 3, "label": r"$4\nu_2+\nu_3$",             "state": "20013e", "G": 1.455},
    {"family": "4nu2+nu3",        "column": 3, "label": r"$4\nu_2+\nu_3$",             "state": "12211e", "G": 1.517},
    {"family": "4nu2+nu3",        "column": 3, "label": r"$4\nu_2+\nu_3$",             "state": "12212e", "G": 1.465},
    {"family": "4nu2+nu3",        "column": 3, "label": r"$4\nu_2+\nu_3$",             "state": "04411e", "G": 1.490},
]

for lvl in levels:
    lvl["THz"] = 100.0 * lvl["G"]

def get_l_value(state):
    return int(state[:5][2])

style_map = {
    0: {"color": "black",      "lw": 3.0, "alpha": 1.00, "zorder": 5},
    1: {"color": "dimgray",    "lw": 2.4, "alpha": 0.95, "zorder": 4},
    2: {"color": "tab:blue",   "lw": 2.0, "alpha": 0.90, "zorder": 3},
    3: {"color": "tab:orange", "lw": 1.7, "alpha": 0.85, "zorder": 2},
    4: {"color": "tab:green",  "lw": 1.4, "alpha": 0.80, "zorder": 1},
}

families = {}
for lvl in levels:
    fam = lvl["family"]
    lvl["l"] = get_l_value(lvl["state"])
    if fam not in families:
        families[fam] = {
            "column": lvl["column"],
            "label": lvl["label"],
            "levels": []
        }
    families[fam]["levels"].append(lvl)

for fam in families.values():
    fam["levels"].sort(key=lambda x: x["THz"])

x_centers = {
    1: 1.0,
    2: 2.7,
    3: 4.4,
}

fig, ax = plt.subplots(figsize=(8.8, 8.2))

line_half_width = 0.16
family_spread = 0.34

# Draw vibrational manifolds
for fam_name, fam in families.items():
    x0 = x_centers[fam["column"]]
    fam_levels = fam["levels"]
    ys = [lvl["THz"] for lvl in fam_levels]
    n = len(fam_levels)

    if n == 1:
        x_offsets = [0.0]
    else:
        x_offsets = [
            -family_spread / 2 + i * family_spread / (n - 1)
            for i in range(n)
        ]

    for dx, lvl in zip(x_offsets, fam_levels):
        x = x0 + dx
        y = lvl["THz"]
        st = style_map.get(lvl["l"], {"color": "0.5", "lw": 1.5, "alpha": 0.8, "zorder": 1})

        ax.hlines(
            y,
            x - line_half_width,
            x + line_half_width,
            color=st["color"],
            linewidth=st["lw"],
            alpha=st["alpha"],
            zorder=st["zorder"],
        )
        
    y_label = sum(ys) / len(ys)

    if fam_name == "GS":
        y_label += 3   # adjust this number to taste
        x0 += 2.9
    
    ax.text(
        x0 + family_spread / 2 + 0.18,
        y_label,
        fam["label"],
        va="center",
        ha="left",
        fontsize=12,
    )


# Full-width GS baseline
ax.hlines(
    0.0,
    0.45,
    5.00,
    color="black",
    linewidth=3,
    zorder=0,
)

# Rotational sub-levels for GS and nu3
B_GS_THz  = 1.17e10 / 1e12
B_nu3_THz = 1.16e10 / 1e12

G_GS_THz  = 0.0
G_nu3_THz = 70.4

Js = list(range(0, 61, 2))

x_rot = x_centers[1]
rot_half_width = line_half_width

for J in Js:
    y_gs  = G_GS_THz  + B_GS_THz  * J * (J + 1)
    y_nu3 = G_nu3_THz + B_nu3_THz * J * (J + 1)

    ax.hlines(
        y_gs,
        x_rot - rot_half_width,
        x_rot + rot_half_width,
        color="black",
        linewidth=0.6,
        zorder=10,
    )

    ax.hlines(
        y_nu3,
        x_rot - rot_half_width,
        x_rot + rot_half_width,
        color="black",
        linewidth=0.6,
        zorder=10,
    )






# --- Arrow-related part using arrays + loops ---

arrowprops = dict(
    arrowstyle="->",
    lw=0.6,
    color="black",
    shrinkA=0,
    shrinkB=0,
)

# Level centers used by arrows
pts = {
    "00011e": (x_centers[1], 70.4),
    "00021e": (x_centers[1], 140.1),
    "01111e": (x_centers[3], 90.1),

    # 2nu2
    "10001e": (x_centers[2] + family_spread / 2, 41.6),
    "10002e": (x_centers[2] - family_spread / 2, 38.5),
    "02201e": (x_centers[2], 40.0),

    # 3nu2  (sorted: 11102e, 03301e, 11101e)
    "11101e": (x_centers[2] + family_spread / 2, 62.3),
    "11102e": (x_centers[2] - family_spread / 2, 57.9),

    # 2nu2+nu3 (sorted: 10012e, 02211e, 10011e)
    "10011e": (x_centers[3] + family_spread / 2, 111.4),
    "10012e": (x_centers[3] - family_spread / 2, 108.3),
    "02211e": (x_centers[3], 109.7),
}

transitions = [
    ("00011e", ["10001e", "10002e"]),
    ("00021e", ["10011e", "10012e"]),
    ("01111e", ["11101e", "11102e"]),
    ("10011e", ["10001e"]),
    ("10012e", ["10002e"]),
    ("02211e", ["02201e"]),
]

for start, ends in transitions:
    x0, y0 = pts[start]
    for end in ends:
        x1, y1 = pts[end]
        ax.annotate(
            "",
            xy=(x1, y1),
            xytext=(x0, y0),
            arrowprops=arrowprops,
            zorder=20,
        )


# --- Add continuation markers above the nu2 and nu2+nu3 ladders ---

def add_ladder_continuation(ax, x, y_top, dy=25, n=5, fontsize=16):
    for i in range(n):
        ax.text(
            x,
            y_top + (i + 1) * dy,
            r"$\cdots$",
            ha="center",
            va="center",
            fontsize=fontsize,
            color="0.35",
        )

# top of nu2 ladder
y_top_nu2 = max(lvl["THz"] for lvl in families["4nu2"]["levels"])
add_ladder_continuation(ax, x_centers[2], y_top_nu2, dy=25, n=5, fontsize=16)

# top of nu2+nu3 ladder
y_top_nu2_nu3 = max(lvl["THz"] for lvl in families["4nu2+nu3"]["levels"])
add_ladder_continuation(ax, x_centers[3], y_top_nu2_nu3, dy=25, n=2, fontsize=16)



#--- Add labels to arrows -->

ax.text(1.5, 50, "Regular bands", fontsize=11)
ax.text(1.5, 125, "Sequence bands", fontsize=11)
ax.text(4, 80, "Hot bands", fontsize=11)
ax.text(3.5, 95, "4 μm bands", fontsize=11)







all_y = [lvl["THz"] for lvl in levels]
ymax_rot = max(
    G_GS_THz + B_GS_THz * 60 * 61,
    G_nu3_THz + B_nu3_THz * 60 * 61,
)

ax.set_xlim(0.35, 5.15)
ax.set_ylim(-2, max(max(all_y), ymax_rot) + 2)

ax.set_ylabel("Frequency (THz)", fontsize=13)

ax.set_xticks([])
ax.spines["bottom"].set_visible(False)
ax.spines["top"].set_visible(False)
ax.spines["right"].set_visible(False)

ax.grid(False)

plt.tight_layout()
plt.savefig("level-diagram.pdf", bbox_inches="tight")
plt.show()