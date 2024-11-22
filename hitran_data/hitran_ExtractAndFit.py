import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

# ================================== INPUT ====================================
filename = 'HITRAN_CO2.par'  # HITRAN data file
iso_id = 'A'  # Isotopologue id
# 626: 1, 727: 9, 828: 7, 636: 2, 737: B, 838: 0
# 627: 4, 628: 3, 728: 8, 637: 6, 638: 5, 738: A

V_up = ' 0 0 0 11'
#V_up = ' 0 0 0 21'
#V_up = ' 0 1 1 01'
#V_up = ' 0 1 1 11'
#V_up = ' 0 2 2 01'
#V_up = ' 0 2 2 11'
#V_up = ' 0 3 3 01'
#V_up = ' 1 0 0 11'
#V_up = ' 1 0 0 12'
#V_up = ' 1 1 1 01'
#V_up = ' 1 1 1 02'

V_lo = ' 0 0 0 01'
#V_lo = ' 0 0 0 11'
#V_lo = ' 0 1 1 01'
#V_lo = ' 0 2 2 01'
#V_lo = ' 1 0 0 01'
#V_lo = ' 1 0 0 02'
#V_lo = ' 1 1 1 01'
#V_lo = ' 1 1 1 02'

symmetry = 'e'


# Fixed fit parameters.
# "G B D H L"
# use 'fit' for not fixed parameters
# example: fixed_lo = "0 fit fit fit fit"

fixed_up = "fit fit fit fit fit"
#fixed_up = "3.841755e+13 fit fit fit fit"
#fixed_up = "1.039876e+14 1.062791e+10 3.922476e+03 5.014599e-03 -1.272138e-08"

#fixed_lo = "fit fit fit fit fit"
fixed_lo = "0 fit fit fit fit"
#fixed_lo = "6.763179e+13 1.061985e+10 3.328566e+03 1.629353e-04 5.193608e-09"

# =============================================================================


# Conversion factor from cm^-1 to Hz
cm_to_hz = 0.0299792458 * 1e12  # 1 cm^-1 = 0.0299792458 THz, converted to Hz

# Parse fixed parameter strings and convert from Hz to cm^-1
def parse_fixed_params(fixed_str):
    fixed_values = fixed_str.split()
    parsed_values = [float(v) / cm_to_hz if v != "fit" else None for v in fixed_values]
    return parsed_values

G_up_fixed, B_up_fixed, D_up_fixed, H_up_fixed, L_up_fixed = parse_fixed_params(fixed_up)
G_lo_fixed, B_lo_fixed, D_lo_fixed, H_lo_fixed, L_lo_fixed = parse_fixed_params(fixed_lo)

def parse_hitran_line(line, iso_id, V_up, V_lo, symmetry):
    if line[0:2].strip() != "2":  # Check for CO2 molecule ID
        return None
    if line[2] != iso_id:  # Isotopologue code
        return None
    if line[73:82].strip() != V_up.strip() or line[88:97].strip() != V_lo.strip():
        return None  # Vibrational levels V_up and V_lo match
    try:
        J_lo = int(line[118:121].strip())  # Lower rotational quantum number
    except ValueError:
        return None
    branch = line[117]
    if branch == 'P':
        J_up = J_lo - 1
    #elif branch == 'Q':
        #J_up = J_lo
    elif branch == 'R':
        J_up = J_lo + 1
    else:
        return None  # Invalid branch
    if line[121].strip().lower() != symmetry.lower():  # e/f match
        return None
    try:
        wavenumber = float(line[3:15].strip())  # Transition wavenumber
    except ValueError:
        return None
    return J_lo, J_up, wavenumber

def extract_data(filename, iso_id, V_up, V_lo, symmetry):
    extracted_data = []
    try:
        with open(filename, 'r') as file:
            for line in file:
                if len(line.rstrip('\r\n')) != 160:
                    continue  # Skip lines that are not 160 characters long
                parsed = parse_hitran_line(line, iso_id, V_up, V_lo, symmetry)
                if parsed:
                    extracted_data.append(parsed)
    except FileNotFoundError:
        print(f"Error: The file '{filename}' was not found.")
        return []
    except IOError:
        print(f"Error: The file '{filename}' could not be opened.")
        return []
    return extracted_data

def wavenumber_fit(J_lo, J_up, G_up, B_up, D_up, H_up, L_up, G_lo, B_lo, D_lo, H_lo, L_lo):
    """Extended fitting function with G_up, G_lo and additional terms."""
    JJ_up = J_up * (J_up + 1)
    JJ_lo = J_lo * (J_lo + 1)
    return ((G_up + B_up * JJ_up - D_up * JJ_up**2 + H_up * JJ_up**3 + L_up * JJ_up**4) -
            (G_lo + B_lo * JJ_lo - D_lo * JJ_lo**2 + H_lo * JJ_lo**3 + L_lo * JJ_lo**4))

def fit_and_plot(data):
    J_lo_values = np.array([item[0] for item in data])
    J_up_values = np.array([item[1] for item in data])
    wavenumbers = np.array([item[2] for item in data])

    # Define fixed parameters
    fixed_params = {
        'G_up': G_up_fixed, 'B_up': B_up_fixed, 'D_up': D_up_fixed, 'H_up': H_up_fixed, 'L_up': L_up_fixed,
        'G_lo': G_lo_fixed, 'B_lo': B_lo_fixed, 'D_lo': D_lo_fixed, 'H_lo': H_lo_fixed, 'L_lo': L_lo_fixed
    }

    # Determine which parameters are free and set initial guesses
    initial_guess = []
    param_names = []

    for name, value in fixed_params.items():
        if value is None:
            # Add initial guesses based on parameter type
            if name in ['G_up', 'G_lo']:
                initial_guess.append(1000)
            elif name in ['B_up', 'B_lo']:
                initial_guess.append(0.4)
            elif name in ['D_up', 'D_lo']:
                initial_guess.append(0.01)
            elif name in ['H_up', 'H_lo']:
                initial_guess.append(1e-4)
            elif name in ['L_up', 'L_lo']:
                initial_guess.append(1e-6)
            param_names.append(name)

    # Define the fitting function with only the free parameters
    def fitting_function(J_lo, *params):
        param_dict = dict(zip(param_names, params))
        G_up = param_dict.get('G_up', fixed_params['G_up'])
        B_up = param_dict.get('B_up', fixed_params['B_up'])
        D_up = param_dict.get('D_up', fixed_params['D_up'])
        H_up = param_dict.get('H_up', fixed_params['H_up'])
        L_up = param_dict.get('L_up', fixed_params['L_up'])
        G_lo = param_dict.get('G_lo', fixed_params['G_lo'])
        B_lo = param_dict.get('B_lo', fixed_params['B_lo'])
        D_lo = param_dict.get('D_lo', fixed_params['D_lo'])
        H_lo = param_dict.get('H_lo', fixed_params['H_lo'])
        L_lo = param_dict.get('L_lo', fixed_params['L_lo'])
        return wavenumber_fit(J_lo, J_up_values, G_up, B_up, D_up, H_up, L_up, G_lo, B_lo, D_lo, H_lo, L_lo)

    # Perform the curve fitting
    params, _ = curve_fit(fitting_function, J_lo_values, wavenumbers, p0=initial_guess)

    # Map results back to all parameters
    fitted_params = {
        name: fixed_params[name] if fixed_params[name] is not None else params[param_names.index(name)]
        for name in ['G_up', 'B_up', 'D_up', 'H_up', 'L_up', 'G_lo', 'B_lo', 'D_lo', 'H_lo', 'L_lo']
    }

    # Format the output for UP and LO levels in scientific notation
    up_output = f"UP: {fitted_params['G_up']*cm_to_hz:.6e} {fitted_params['B_up']*cm_to_hz:.6e} " \
                f"{fitted_params['D_up']*cm_to_hz:.6e} {fitted_params['H_up']*cm_to_hz:.6e} " \
                f"{fitted_params['L_up']*cm_to_hz:.6e}"
    lo_output = f"LO: {fitted_params['G_lo']*cm_to_hz:.6e} {fitted_params['B_lo']*cm_to_hz:.6e} " \
                f"{fitted_params['D_lo']*cm_to_hz:.6e} {fitted_params['H_lo']*cm_to_hz:.6e} " \
                f"{fitted_params['L_lo']*cm_to_hz:.6e}"
    print(up_output)
    print(lo_output)

    # Generate fitted curves for each branch
    J_lo_range = np.arange(0, J_lo_values.max(), 1)
    J_up_p_branch = J_lo_range - 1
    #J_up_q_branch = J_lo_range
    J_up_r_branch = J_lo_range + 1

    # Calculate fitted wavenumbers for each branch
    G_up_fit = fitted_params['G_up']
    B_up_fit = fitted_params['B_up']
    D_up_fit = fitted_params['D_up']
    H_up_fit = fitted_params['H_up']
    L_up_fit = fitted_params['L_up']
    G_lo_fit = fitted_params['G_lo']
    B_lo_fit = fitted_params['B_lo']
    D_lo_fit = fitted_params['D_lo']
    H_lo_fit = fitted_params['H_lo']
    L_lo_fit = fitted_params['L_lo']
    fitted_wavenumbers_p = wavenumber_fit(J_lo_range, J_up_p_branch, G_up_fit, B_up_fit, D_up_fit, H_up_fit, L_up_fit, G_lo_fit, B_lo_fit, D_lo_fit, H_lo_fit, L_lo_fit)
    #fitted_wavenumbers_q = wavenumber_fit(J_lo_range, J_up_q_branch, G_up_fit, B_up_fit, D_up_fit, H_up_fit, L_up_fit, G_lo_fit, B_lo_fit, D_lo_fit, H_lo_fit, L_lo_fit)
    fitted_wavenumbers_r = wavenumber_fit(J_lo_range, J_up_r_branch, G_up_fit, B_up_fit, D_up_fit, H_up_fit, L_up_fit, G_lo_fit, B_lo_fit, D_lo_fit, H_lo_fit, L_lo_fit)

    # Plot data points and fitted functions for each branch
    plt.figure(figsize=(10, 6))
    plt.scatter(J_lo_values, wavenumbers, color='blue', marker='o', label='Data')
    plt.plot(J_lo_range, fitted_wavenumbers_p, color='green', linestyle='--', label='P-branch fit')
    #plt.plot(J_lo_range, fitted_wavenumbers_q, color='orange', linestyle='-.', label='Q-branch fit')
    plt.plot(J_lo_range, fitted_wavenumbers_r, color='red', label='R-branch fit')
    plt.xlabel("J_lo (Lower Rotational Quantum Number)")
    plt.ylabel("Transition Wavenumber (cm^-1)")
    plt.title("Transition Wavenumber as a Function of J_lo with Fitted Curves for Each Branch")
    plt.legend()
    plt.grid(True)
    plt.show()

# Run the data extraction and fitting
data = extract_data(filename, iso_id, V_up, V_lo, symmetry)
if data:
    fit_and_plot(data)
else:
    print("No matching data found.")
