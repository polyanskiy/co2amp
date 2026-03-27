import re
from dataclasses import dataclass
from typing import Dict, Iterable, List, Optional, Tuple

import numpy as np
from scipy.optimize import least_squares

# ============================== INPUT =========================================
filename = "HITRAN_CO2.par"

# Isotopologue:
# 626: 1, 727: 9, 828: 7, 636: 2, 737: B, 838: 0
# 627: 4, 628: 3, 728: 8, 637: 6, 638: 5, 738: A
iso_code = "738"

Jmax_fit = 100
Amin = 1e-1
# ==============================================================================

CM_TO_HZ = 0.0299792458 * 1e12
MOLECULE_ID_CO2 = "2"

ISO_TO_HITRAN = {
    "626": "1",
    "636": "2",
    "628": "3",
    "627": "4",
    "638": "5",
    "637": "6",
    "828": "7",
    "728": "8",
    "727": "9",
    "738": "A",
    "737": "B",
    "838": "0",
}


@dataclass(frozen=True, order=True)
class VibNoEF:
    m1: int
    m2: int
    l2: int
    m3: int
    r: int


@dataclass(frozen=True, order=True)
class VibKey:
    m1: int
    m2: int
    l2: int
    m3: int
    r: int
    ef: str

    def noef(self) -> VibNoEF:
        return VibNoEF(self.m1, self.m2, self.l2, self.m3, self.r)

    def afgl(self) -> str:
        return f"{self.m1}{self.m2}{self.l2}{self.m3}{self.r}"


@dataclass
class Transition:
    upper_vib: VibKey
    lower_vib: VibKey
    J_up: int
    J_lo: int
    nu_hz: float
    line_no: int


@dataclass
class ParsedLine:
    upper_vib: Tuple[int, int, int, int, int]
    lower_vib: Tuple[int, int, int, int, int]
    upper_J: int
    lower_J: int
    upper_ef: str
    lower_ef: str
    wavenumber_cm: float
    A: float


def normalize_iso_code(code: str) -> str:
    code = code.strip()
    if code in ISO_TO_HITRAN.values():
        return code
    if code in ISO_TO_HITRAN:
        return ISO_TO_HITRAN[code]
    known = ", ".join(sorted(ISO_TO_HITRAN))
    raise ValueError(f"Unknown isotopologue code '{code}'. Use one of: {known}")


def parse_vib_field(field: str) -> Optional[Tuple[int, int, int, int, int]]:
    nums = re.findall(r"[-+]?\d", field)
    if len(nums) != 5:
        return None
    m1, m2, l2, m3, r = (int(x) for x in nums)
    return m1, m2, l2, m3, r


def parity_pair_from_transition(branch: str, lower_ef: str) -> Tuple[str, str]:
    ef = lower_ef.strip().lower()
    if ef not in {"e", "f"}:
        raise ValueError(f"Invalid e/f label: {lower_ef!r}")
    return ef, ef


def parse_hitran_line(line: str, iso_hitran: str) -> Optional[ParsedLine]:
    if len(line.rstrip("\r\n")) != 160:
        return None
    if line[0:2].strip() != MOLECULE_ID_CO2:
        return None
    if line[2] != iso_hitran:
        return None

    try:
        wavenumber_cm = float(line[3:15].strip())
        A = float(line[25:35].strip())
    except ValueError:
        return None

    upper_vib = parse_vib_field(line[73:82])
    lower_vib = parse_vib_field(line[88:97])
    if upper_vib is None or lower_vib is None:
        return None

    branch = line[117].strip().upper()
    if branch not in {"P", "R"}:
        return None

    try:
        lower_J = int(line[118:121].strip())
    except ValueError:
        return None

    if branch == "P":
        upper_J = lower_J - 1
    else:
        upper_J = lower_J + 1

    if upper_J < 0:
        return None

    lower_ef = line[121].strip().lower()
    if lower_ef not in {"e", "f"}:
        return None

    lower_ef, upper_ef = parity_pair_from_transition(branch, lower_ef)

    return ParsedLine(
        upper_vib=upper_vib,
        lower_vib=lower_vib,
        upper_J=upper_J,
        lower_J=lower_J,
        upper_ef=upper_ef,
        lower_ef=lower_ef,
        wavenumber_cm=wavenumber_cm,
        A=A,
    )


def vib_allowed(vib: Tuple[int, int, int, int, int]) -> bool:
    m1, m2, l2, m3, _r = vib

    if not (0 <= m3 <= 3):
        return False
    if m2 + 2 * m1 > 4:
        return False
    if m3 >= 2 and (m1 != 0 or m2 != 0):
        return False

    return True


def ignore_transition_due_to_f_l2_zero(
    upper_vib: Tuple[int, int, int, int, int],
    lower_vib: Tuple[int, int, int, int, int],
    upper_ef: str,
    lower_ef: str,
) -> bool:
    if upper_ef == "f" or lower_ef == "f":
        if upper_vib[2] == 0 or lower_vib[2] == 0:
            return True
    return False


def iter_transitions(filename: str, iso_hitran: str) -> Iterable[Transition]:
    with open(filename, "r", encoding="ascii", errors="ignore") as fh:
        for line_no, line in enumerate(fh, start=1):
            parsed = parse_hitran_line(line, iso_hitran)
            if parsed is None:
                continue

            if parsed.A <= Amin:
                continue

            if parsed.lower_J > Jmax_fit or parsed.upper_J > Jmax_fit:
                continue

            if not (vib_allowed(parsed.upper_vib) and vib_allowed(parsed.lower_vib)):
                continue

            if ignore_transition_due_to_f_l2_zero(
                parsed.upper_vib, parsed.lower_vib, parsed.upper_ef, parsed.lower_ef
            ):
                continue

            upper_key = VibKey(*parsed.upper_vib, parsed.upper_ef)
            lower_key = VibKey(*parsed.lower_vib, parsed.lower_ef)

            yield Transition(
                upper_vib=upper_key,
                lower_vib=lower_key,
                J_up=parsed.upper_J,
                J_lo=parsed.lower_J,
                nu_hz=parsed.wavenumber_cm * CM_TO_HZ,
                line_no=line_no,
            )


def build_vib_index(transitions: List[Transition]) -> Dict[VibKey, int]:
    vib_keys = sorted({t.upper_vib for t in transitions} | {t.lower_vib for t in transitions})
    return {key: idx for idx, key in enumerate(vib_keys)}


def build_vib_noef_index(vib_keys: List[VibKey]) -> Dict[VibNoEF, int]:
    keys = sorted({v.noef() for v in vib_keys})
    return {key: idx for idx, key in enumerate(keys)}


def choose_reference_g(vib_noef_to_idx: Dict[VibNoEF, int]) -> Tuple[VibNoEF, int]:
    ref = VibNoEF(0, 0, 0, 0, 1)
    if ref in vib_noef_to_idx:
        return ref, vib_noef_to_idx[ref]
    first = min(vib_noef_to_idx, key=lambda x: (x.m1, x.m2, x.l2, x.m3, x.r))
    return first, vib_noef_to_idx[first]


def initial_guess_params(
    vib_keys: List[VibKey],
    vib_noef_keys: List[VibNoEF],
    ref_noef: VibNoEF,
) -> np.ndarray:
    p0 = []

    for v in vib_noef_keys:
        if v == ref_noef:
            continue
        G0 = 1.0e13
        if (v.m1, v.m2, v.m3) == (0, 0, 0):
            G0 = 1.0e11
        p0.append(np.log(G0))

    for _v in vib_keys:
        p0.append(np.log(1.0e10))  # B > 0
        p0.append(np.log(1.0e3))   # D > 0
        p0.append(0.0)             # H free
        p0.append(0.0)             # L free

    return np.array(p0, dtype=float)


def unpack_params(
    param_vec: np.ndarray,
    vib_keys: List[VibKey],
    vib_noef_keys: List[VibNoEF],
    ref_noef: VibNoEF,
) -> Tuple[
    Dict[VibNoEF, float],
    Dict[VibKey, Tuple[float, float, float, float]],
]:
    shared_G: Dict[VibNoEF, float] = {}
    params: Dict[VibKey, Tuple[float, float, float, float]] = {}

    k = 0
    for v in vib_noef_keys:
        if v == ref_noef:
            shared_G[v] = 0.0
        else:
            shared_G[v] = np.exp(param_vec[k])
            k += 1

    for v in vib_keys:
        B = np.exp(param_vec[k])
        D = np.exp(param_vec[k + 1])
        H = param_vec[k + 2]
        L = param_vec[k + 3]
        params[v] = (B, D, H, L)
        k += 4

    return shared_G, params


def residuals_params(
    param_vec: np.ndarray,
    transitions: List[Transition],
    vib_keys: List[VibKey],
    vib_noef_keys: List[VibNoEF],
    ref_noef: VibNoEF,
) -> np.ndarray:
    shared_G, params = unpack_params(param_vec, vib_keys, vib_noef_keys, ref_noef)
    res = np.empty(len(transitions), dtype=float)

    for i, t in enumerate(transitions):
        Gu = shared_G[t.upper_vib.noef()]
        Gl = shared_G[t.lower_vib.noef()]

        Bu, Du, Hu, Lu = params[t.upper_vib]
        Bl, Dl, Hl, Ll = params[t.lower_vib]

        Xu = t.J_up * (t.J_up + 1)
        Xl = t.J_lo * (t.J_lo + 1)

        Eu = Gu + Bu * Xu - Du * Xu**2 + Hu * Xu**3 + Lu * Xu**4
        El = Gl + Bl * Xl - Dl * Xl**2 + Hl * Xl**3 + Ll * Xl**4
        res[i] = t.nu_hz - (Eu - El)

    return res


def fit_gbdhl_nonlinear(transitions: List[Transition], vib_to_idx: Dict[VibKey, int]):
    vib_keys = sorted(vib_to_idx, key=lambda x: (x.m1, x.m2, x.l2, x.m3, x.r, x.ef))
    vib_noef_to_idx = build_vib_noef_index(vib_keys)
    vib_noef_keys = sorted(vib_noef_to_idx, key=lambda x: (x.m1, x.m2, x.l2, x.m3, x.r))
    ref_noef, _ = choose_reference_g(vib_noef_to_idx)

    p0 = initial_guess_params(vib_keys, vib_noef_keys, ref_noef)

    result = least_squares(
        residuals_params,
        p0,
        args=(transitions, vib_keys, vib_noef_keys, ref_noef),
        method="trf",
        loss="linear",
        max_nfev=5000,
        xtol=1e-14,
        ftol=1e-14,
        gtol=1e-14,
    )

    if not result.success:
        print("Warning: nonlinear fit did not fully converge:", result.message)

    shared_G, params = unpack_params(result.x, vib_keys, vib_noef_keys, ref_noef)
    return vib_keys, ref_noef, shared_G, params


def print_constants_terminal(
    vib_keys: List[VibKey],
    shared_G: Dict[VibNoEF, float],
    params: Dict[VibKey, Tuple[float, float, float, float]],
) -> None:
    print()
    print(" AFGL   ef             G_Hz             B_Hz             D_Hz             H_Hz             L_Hz")
    print("-" * 96)
    for vib in vib_keys:
        G = shared_G[vib.noef()]
        B, D, H, L = params[vib]
        print(
            f"{vib.afgl():>6s} {vib.ef:>2s} "
            f"{G:16.6E} {B:16.6E} {D:16.6E} {H:16.6E} {L:16.6E}"
        )


def print_selected_levels_summary(
    vib_keys: List[VibKey],
    shared_G: Dict[VibNoEF, float],
    params: Dict[VibKey, Tuple[float, float, float, float]],
) -> None:
    # Table level number -> (AFGL, ef)
    selected = [
        # Ground state
        (0,  "00001", "e"),
        # nu3, 2nu3, 3nu3
        (1,  "00011", "e"),
        (2,  "00021", "e"),
        (3,  "00031", "e"),
        # nu2
        (4,  "01101", "e"),
        (5,  "01101", "f"),
        # 2nu2
        (6,  "10001", "e"),
        (7,  "10002", "e"),
        (8,  "02201", "e"),
        (9,  "02201", "f"),
        # 3nu2
        (10, "11101", "e"),
        (11, "11101", "f"),
        (12, "11102", "e"),
        (13, "11102", "f"),
        (14, "03301", "e"),
        (15, "03301", "f"),
        # 4nu22
        (16, "20001", "e"),
        (17, "20002", "e"),
        (18, "20003", "e"),
        (19, "12201", "e"),
        (21, "12201", "f"),
        (20, "12202", "e"),
        (22, "12202", "f"),
        (23, "04401", "e"),
        (24, "04401", "f"),
        # nu2 + nu3
        (25, "01111", "e"),
        (26, "01111", "f"),
        # 2nu2 + nu3
        (27, "10011", "e"),
        (28, "10012", "e"),
        (29, "02211", "e"),
        (30, "02211", "f"),
        # 3nu2 + nu3        
        (31, "11111", "e"),
        (32, "11111", "f"),
        (33, "11112", "e"),
        (34, "11112", "f"),
        (35, "03311", "e"),
        (36, "03311", "f"),
        # 4nu2 + nu3
        (37, "20011", "e"),
        (38, "20012", "e"),
        (39, "20013", "e"),
        (40, "12211", "e"),
        (42, "12211", "f"),
        (41, "12212", "e"),
        (43, "12212", "f"),
        (44, "04411", "e"),
        (45, "04411", "f"),
    ]

    # Build lookup from fitted results
    lookup: Dict[Tuple[str, str], Tuple[float, float]] = {}
    for vib in vib_keys:
        G = shared_G[vib.noef()]
        B, D, H, L = params[vib]
        lookup[(vib.afgl(), vib.ef)] = (G, B)

    print()
    print("Selected levels: G in 1e14 Hz, B in 1e10 Hz")
    print(" level   AFGL  ef      G(1e14 Hz)   B(1e10 Hz)")
    print("------------------------------------------------")
    for idx, afgl, ef in selected:
        item = lookup.get((afgl, ef))
        if item is None:
            print(f"{idx:5d} {afgl:>6s} {ef:>2s} {'---':>14s} {'---':>14s}")
        else:
            G, B = item
            print(f"{idx:5d} {afgl:>6s} {ef:>2s} {G/1.0e14:14.3f} {B/1.0e10:14.3f}")


def main() -> None:
    iso_hitran = normalize_iso_code(iso_code)

    transitions = list(iter_transitions(filename, iso_hitran))
    if not transitions:
        print("No matching transitions found.")
        return

    vib_to_idx = build_vib_index(transitions)
    vib_keys, ref_noef, shared_G, params = fit_gbdhl_nonlinear(transitions, vib_to_idx)

    print(f"Isotopologue input: {iso_code} -> HITRAN code {iso_hitran}")
    print(f"Reference vibrational origin fixed to G=0: {ref_noef}")
    print("Constraint used: G(e) = G(f) for same (m1,m2,l2,m3,r)")
    print("Ignored: all f transitions where at least one involved level has l2 = 0")
    print(f"Used only P/R lines with J_up,J_lo <= {Jmax_fit} and A > {Amin}")
    print(f"Transitions used in fit: {len(transitions)}")

    print_constants_terminal(vib_keys, shared_G, params)
    print_selected_levels_summary(vib_keys, shared_G, params)


if __name__ == "__main__":
    main()