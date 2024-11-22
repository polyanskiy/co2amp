
# ================================== INPUT ====================================

output_filename = "estimate_828_band_2f.par"

iso_id = '7'  # Isotopologue id
# 626: 1, 727: 9, 828: 7, 636: 2, 737: B, 838: 0
# 627: 4, 628: 3, 728: 8, 637: 6, 638: 5, 738: A


A_value = "1.180E-01"  # Einstein A coefficient


# Vibrational levels and symmetry

#reg. (bands 0 & 1 - depending on last digit in V_lo)
#V_up = ' 0 0 0 11'
#V_lo = ' 1 0 0 02'
#symmetry = 'e'

#hot. (bands 2 & 3 - depending on last digits in V_lo_up and V_lo)
V_up = ' 0 1 1 11'
V_lo = ' 1 1 1 01'
symmetry = 'f'

#seq. (bands 4 & 5 - depending on last digit in V_lo)
#V_up = ' 0 0 0 21'
#V_lo = ' 1 0 0 11'
#symmetry = 'e'




#Molecular constants of vibrational levels

mc_up = "8.871241e+13 1.034305e+10 3.211021e+03 -4.142739e-04 5.457863e-08"

mc_lo = "6.069384e+13 1.044155e+10 2.915684e+03 -8.769903e-04 1.246696e-08"

# =============================================================================



mol_id = " 2"  # Molecule code for CO2



# Convert fit parameters from strings to cm^-1 for up and lo levels
cm_to_hz = 0.0299792458 * 1e12


def parse_mc(mc_str):
    mc_hz = [float(value) for value in mc_str.split()]
    mc_cm = [mc / cm_to_hz for mc in mc_hz]
    return mc_cm

G_up, B_up, D_up, H_up, L_up = parse_mc(mc_up)
G_lo, B_lo, D_lo, H_lo, L_lo = parse_mc(mc_lo)

def wavenumber(J_up, J_lo, G_up, B_up, D_up, H_up, L_up, G_lo, B_lo, D_lo, H_lo, L_lo):
    JJ_up = J_up * (J_up + 1)
    JJ_lo = J_lo * (J_lo + 1)
    return (G_up + B_up * JJ_up - D_up * JJ_up**2 + H_up * JJ_up**3 + L_up * JJ_up**4 -
            (G_lo + B_lo * JJ_lo - D_lo * JJ_lo**2 + H_lo * JJ_lo**3 + L_lo * JJ_lo**4))

# Define parity check
def get_parity(V):
    v2, v3 = int(V[3]), int(V[7])  # Corrected positions
    return 'g' if (v2 + v3) % 2 == 0 else 'u'

# Define filtering rules
def is_allowed_transition(J_up, J_lo, l_up, l_lo, parity_up, parity_lo):
    if J_up < l_up or J_lo < l_lo:
        return False
    if iso_id in {'4', '3', '8', '6', '5', 'A'}: #asymmetric molecules - all J's allowed
        return True
    if parity_up == 'g' and symmetry == 'e' and J_up % 2 == 1:
        return False
    if parity_up == 'g' and symmetry == 'f' and J_up % 2 == 0:
        return False
    if parity_up == 'u' and symmetry == 'e' and J_up % 2 == 0:
        return False
    if parity_up == 'u' and symmetry == 'f' and J_up % 2 == 1:
        return False
    return True

# Create and write .par file
with open(output_filename, 'w') as f:
    for branch in ['P', 'R']:
        for J in range(0, 101):
            if branch == 'P':
                J_lo = J
                J_up = J_lo - 1
            else:  # R branch
                J_lo = J
                J_up = J_lo + 1
            if J_up < 0 or J_lo < 0 or J_up > 100 or J_lo > 100:
                continue

            # Parity and lower level restrictions
            parity_up = get_parity(V_up)
            parity_lo = get_parity(V_lo)
            l_up, l_lo = int(V_up[5]), int(V_lo[5])  # Corrected positions
            
            if not is_allowed_transition(J_up, J_lo, l_up, l_lo, parity_up, parity_lo):
                continue

            # Calculate wavenumber
            wn = wavenumber(J_up, J_lo, G_up, B_up, D_up, H_up, L_up, G_lo, B_lo, D_lo, H_lo, L_lo)
            wn_str = f"{wn:.6f}".rjust(12)

            # Create branch and J_lo notation with symmetry letter (right-justified J)
            branch_J = f"{branch} {J_lo:>2}{symmetry}"

            # Create the 160-character line
            line = (
                f"{mol_id:<2}" +              # Molecule code (2 chars)
                f"{iso_id:<1}" +              # Isotopologue code (1 char)
                f"{wn_str:<12}" +               # Wavenumber (12 chars)
                " " * 11 +                      # Placeholder for other fields
                f"{A_value:<10}" +              # Einstein A coefficient (10 chars)
                " " * 37 +                      # Placeholder for remaining fields
                f"{V_up:<9}" +                  # Upper vibrational level (9 chars)
                " " * 6 +                       # Placeholder for intermediate fields
                f"{V_lo:<9}" +                  # Lower vibrational level (9 chars)
                " " * 20 +                      # Placeholder for remaining fields
                f"{branch_J:<5}"                # Branch and J_lo notation with symmetry (5 chars)
            ).ljust(160)  # Pad to 160 characters
            f.write(line + "\n")

print(f"Filtered .par file '{output_filename}' created.")
