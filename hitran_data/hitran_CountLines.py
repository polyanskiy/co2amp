import numpy as np

# ================================== INPUT ====================================
filename = 'HITRAN_CO2.par'  # HITRAN data file

# Isotopologue mapping: HITRAN ID to standard isotopologue code
isotopologue_map = {
    '1': '626', '9': '727', '7': '828', '2': '636', 'B': '737', '0': '838',
    '4': '627', '3': '628', '8': '728', '6': '637', '5': '638', 'A': '738'
}

# List of transitions (vibrational levels and descriptions)
transitions = [
    {"V_up": " 0 0 0 11", "V_lo": " 1 0 0 01", "symmetry": ['e'], "desc": "reg 10 um"},
    {"V_up": " 0 0 0 11", "V_lo": " 1 0 0 02", "symmetry": ['e'], "desc": "reg 9 um"},
    {"V_up": " 0 1 1 11", "V_lo": " 1 1 1 01", "symmetry": ['e', 'f'], "desc": "hot 10 um"},
    {"V_up": " 0 1 1 11", "V_lo": " 1 1 1 02", "symmetry": ['e', 'f'], "desc": "hot 9 um"},
    {"V_up": " 0 0 0 21", "V_lo": " 1 0 0 11", "symmetry": ['e'], "desc": "seq 10 um"},
    {"V_up": " 0 0 0 21", "V_lo": " 1 0 0 12", "symmetry": ['e'], "desc": "seq 9 um"},
    {"V_up": " 1 0 0 11", "V_lo": " 1 0 0 01", "symmetry": ['e'], "desc": "4um-1"},
    {"V_up": " 1 0 0 12", "V_lo": " 1 0 0 02", "symmetry": ['e'], "desc": "4um-2"},
    {"V_up": " 0 2 2 11", "V_lo": " 0 2 2 01", "symmetry": ['e', 'f'], "desc": "4um-3"}
]

latex_filename = 'hitran_tables.tex'  # LaTeX output file
# =============================================================================

def parse_hitran_line(line, iso_id, V_up, V_lo, symmetry):
    """
    Parses a HITRAN line to check if it matches specified criteria.
    Extracts the Einstein A coefficient, J_lo, and branch.
    """
    global PR_lines

    # Check molecule code and isotopologue ID
    if line[0:2].strip() != "2" or line[2] != iso_id:
        return None

    # Check vibrational levels
    if line[73:82].strip() != V_up.strip() or line[88:97].strip() != V_lo.strip():
        return None

    # Check symmetry (e or f)
    if line[121].strip().lower() != symmetry.lower():
        return None

    try:
        J_lo = int(line[118:121].strip())  # Lower rotational quantum number
        A = float(line[25:35].strip())    # Einstein A coefficient
        branch = line[117]               # Branch (P, Q, R)
    except ValueError:
        return None

    # Count P and R lines, ignoring Q lines
    if branch in ['P', 'R']:
        PR_lines += 1
        return {'branch': branch, 'J_lo': J_lo, 'A': A}
    return None

def extract_data(filename, iso_id, V_up, V_lo, symmetry):
    """
    Extracts data from the HITRAN file and counts transitions by branch.
    """
    global PR_lines
    PR_lines = 0
    A_coefficients = []

    try:
        with open(filename, 'r') as file:
            for line in file:
                if len(line.rstrip('\r\n')) != 160:
                    continue  # Skip lines that are not 160 characters long
                result = parse_hitran_line(line, iso_id, V_up, V_lo, symmetry)
                if result:
                    A_coefficients.append(result)
    except FileNotFoundError:
        print(f"Error: The file '{filename}' was not found.")
        return []
    except IOError:
        print(f"Error: The file '{filename}' could not be opened.")
        return []

    return A_coefficients

def calculate_average_A(data):
    """
    Calculates the average Einstein A value for the given data.
    """
    if not data:
        return None  # No lines found
    A_values = [item['A'] for item in data]
    return np.mean(A_values)

# Prepare LaTeX tables
pr_line_table = []  # Stores number of P+R lines for each transition (rows) and isotopologue (columns)
avg_a_table = []    # Stores average A for each transition (rows) and isotopologue (columns)

# Process all isotopologues and their transitions
for iso_id, iso_code in isotopologue_map.items():
    print(f"\n=== Isotopologue: {iso_id} ({iso_code}) ===")
    for transition in transitions:
        for sym in transition["symmetry"]:  # Separate e and f symmetries if both exist
            # Extract data for this transition, isotopologue, and symmetry
            total_pr_lines = 0
            avg_a = None
            data = extract_data(filename, iso_id, transition["V_up"], transition["V_lo"], sym)

            # Count P+R lines and calculate average A
            total_pr_lines += PR_lines
            if data:
                avg_a = calculate_average_A(data)

            # Print terminal output
            print(f"Transition: {transition['V_up']} -> {transition['V_lo']} [{sym}] ({transition['desc']})")
            print(f"P+R lines: {total_pr_lines}")
            if avg_a is not None:
                print(f"Average A: {avg_a:.3e}")
            else:
                print("Average A: N/A")
            print()

            # Store results for this transition, isotopologue, and symmetry
            pr_line_table.append((transition["desc"], sym, iso_code, total_pr_lines))
            avg_a_table.append((transition["desc"], sym, iso_code, avg_a if avg_a is not None else "N/A"))

# Consolidate LaTeX rows per transition
consolidated_pr_lines = {}
consolidated_avg_a = {}

for desc, sym, iso_code, value in pr_line_table:
    key = f"{desc} [{sym}]"
    if key not in consolidated_pr_lines:
        consolidated_pr_lines[key] = [0] * len(isotopologue_map)
    consolidated_pr_lines[key][list(isotopologue_map.values()).index(iso_code)] = value

for desc, sym, iso_code, value in avg_a_table:
    key = f"{desc} [{sym}]"
    if key not in consolidated_avg_a:
        consolidated_avg_a[key] = ["N/A"] * len(isotopologue_map)
    consolidated_avg_a[key][list(isotopologue_map.values()).index(iso_code)] = (
        f"{value:.3e}" if value != "N/A" else "N/A"
    )

# Write LaTeX file
with open(latex_filename, 'w') as f:
    f.write("\\documentclass{article}\n")
    f.write("\\usepackage{booktabs}\n")
    f.write("\\begin{document}\n")

    # Write table for P+R line counts
    f.write("\\section*{Table 1: Number of P+R Lines}\n")
    f.write("\\begin{tabular}{l" + "r" * len(isotopologue_map) + "}\n")
    f.write("\\toprule\n")
    f.write("Transition & " + " & ".join(isotopologue_map.values()) + " \\\\\n")
    f.write("\\midrule\n")
    for key, row in consolidated_pr_lines.items():
        row_str = " & ".join(map(str, row))
        f.write(f"{key} & {row_str} \\\\\n")
    f.write("\\bottomrule\n")
    f.write("\\end{tabular}\n")

    # Write table for average A values
    f.write("\\section*{Table 2: Average A (P+R Lines)}\n")
    f.write("\\begin{tabular}{l" + "r" * len(isotopologue_map) + "}\n")
    f.write("\\toprule\n")
    f.write("Transition & " + " & ".join(isotopologue_map.values()) + " \\\\\n")
    f.write("\\midrule\n")
    for key, row in consolidated_avg_a.items():
        row_str = " & ".join(row)
        f.write(f"{key} & {row_str} \\\\\n")
    f.write("\\bottomrule\n")
    f.write("\\end{tabular}\n")

    f.write("\\end{document}\n")

print(f"\nLaTeX tables written to {latex_filename}")

