import argparse
import platform
from pathlib import Path
import shutil
import subprocess
import sys
import os
import re
import threading
import itertools
import time
import getpass

# Data to delete during clean
TO_CLEAN_FOLDERS = [".venv", "build", ".vs", "out"]
TO_CLEAN_FILES = ["CMakeUserPresets.json"]
LUDUS_CONAN_SERVER_URL = "https://conan.matthieudelaere.com/artifactory/api/conan/conan-local"
LUDUS_CONAN_REMOTE_NAME = "ludus-server"
LUDUS_CONAN_DOWNLOAD_CONF = {
    "core.download:parallel": "4",
    "core.download:retry": "8",
    "core.download:retry_wait": "5",
}

# Color codes for outputs
RESET       = "\033[0m"
RED         = "\033[91m"
GREEN       = "\033[92m"
YELLOW      = "\033[93m"
BLUE        = "\033[94m"
MAGENTA     = "\033[35m"
CYAN        = "\033[36m"
BG_BLACK    = "\033[40m"
BG_RED      = "\033[41m"
BG_GREEN    = "\033[42m"
BG_YELLOW   = "\033[43m"
BG_BLUE     = "\033[44m"
BG_MAGENTA  = "\033[45m"
BG_CYAN     = "\033[46m"
BG_WHITE    = "\033[47m"
BOLD        = "\033[1m"
DIM         = "\033[2m"
ITALIC      = "\033[3m"
UNDERLINE   = "\033[4m"
STRIKE      = "\033[9m"

SPINNER_FRAMES = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"]

def run_with_spinner(cmd, label, verbose, **args):
    if verbose:
        return subprocess.run(cmd, text=True, **args)
    
    stop_event = threading.Event()

    def spin():
        for frame in itertools.cycle(SPINNER_FRAMES):
            if stop_event.is_set():
                break
            print(f"\r  {CYAN}{frame}{RESET} {label}", end="", flush=True)
            time.sleep(0.08)

    spinner_thread = threading.Thread(target=spin, daemon=True)
    spinner_thread.start()
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, **args)
    finally:
        stop_event.set()
        spinner_thread.join()
        # Clear the spinner line
        print(f"\r{' ' * (len(label) + 6)}\r", end="", flush=True)
    return result

def _read_profile_compiler_version(profile_path):
    """Read compiler.version from a Conan profile, following include() directives."""
    path = Path(profile_path)
    if not path.exists():
        return None
    content = path.read_text()
    # Check for include directive and follow it
    include_match = re.search(r'^include\((.+)\)$', content, re.MULTILINE)
    if include_match:
        included = path.parent / include_match.group(1)
        if included.exists():
            content = included.read_text() + "\n" + content
    version_match = re.search(r'^compiler\.version\s*=\s*(.+)$', content, re.MULTILINE)
    if version_match:
        return version_match.group(1).strip()
    return None


def main():
    # Start with parsing arguments
    parser = argparse.ArgumentParser(description="Ludus Bootstraper")
    parser.add_argument(
        "--clean", 
        action="store_true",
        default=False, 
        help="Delete the generated files and the virtual environment")
    parser.add_argument(
        "--clean-only", 
        action="store_true", 
        default=False,
        help="Only clean and do not continue with the actual setup")
    parser.add_argument(
        "--clear-conan-cache",
        action="store_true",
        default=False,
        help="Also clear the global Conan cache (used with --clean or --clean-only)")
    parser.add_argument(
        "--verbose",
        action="store_true", 
        default=False,
        help="Does verbose logging")
    parser.add_argument(
        "--compilers", 
        nargs="+", 
        default=[], 
        help="Compilers to use with profiles (msvc, gcc, clang)")
    parser.add_argument(
        "--build_types", 
        nargs="+", 
        default=[], 
        help="Conan build_types to use (e.g. debug, release) [optional]")
    parser.add_argument(
        "--ci", 
        action="store_true",
        default=False, 
        help="Use CI overrides (avoid local profile mismatch)")
    parser.add_argument(
        "--build-tests",
        action="store_true",
        default=False,
        help="Enable building and installing unit tests (GTest)")
    parser.add_argument(
        "--no-conan-remote",
        action="store_true",
        default=False,
        help="Skip the private Conan server entirely, use conan-center only")
    args = parser.parse_args()

    # Acquire the arguments in an easy to use setup
    clean = args.clean
    clean_only = args.clean_only
    clear_conan_cache = args.clear_conan_cache
    verbose_logging = args.verbose
    compilers = args.compilers
    build_types = args.build_types
    ci_mode = args.ci
    build_tests = args.build_tests
    no_conan_remote = args.no_conan_remote

    # Set autodetected variables for fallback
    os_name = platform.system().lower()
    if not compilers:
        compilers = ["msvc"]
    if not build_types:
        build_types = ["debug", "release"]

    # Create full profiles here
    profiles = [
        f"{os_name}-{compiler}-{built_type}"
        for compiler in compilers
        for built_type in build_types] 

    #---------- START PROCESSING ---------
    print(f"{BOLD}{BLUE}------ GENERATING STARTED ------{RESET}")
    print(f"{UNDERLINE}SETTINGS: {RESET}")
    print(f"Clean flag: {clean}")
    print(f"Built Types: {build_types}")
    print(f"OS System: {os_name}")
    print(f"{BOLD}{MAGENTA}Profiles: {profiles}{RESET}")
    print(f"Build Tests: {build_tests}")

    #------------------------------------------
    # Cleanup of data if requested
    if clean or clean_only:
        print(f"{BOLD}{BLUE}------ CLEAN SETUP ------{RESET}")
        for folder in TO_CLEAN_FOLDERS:
            path = Path(folder)
            if path.exists() and path.is_dir():
                print(f"Removing folder: {path}")
                shutil.rmtree(path)
        for file in TO_CLEAN_FILES:
            path = Path(file)
            if path.exists() and path.is_file():
                print(f"Removing file: {path}")
                path.unlink()
        if clear_conan_cache:
            conan_home = Path(os.environ.get("CONAN_HOME", Path.home() / ".conan2"))
            if conan_home.exists() and conan_home.is_dir():
                print(f"{YELLOW}Removing global Conan cache: {conan_home}{RESET}")
                shutil.rmtree(conan_home)
                print(f"{GREEN}Global Conan cache removed{RESET}")
            else:
                print(f"{MAGENTA}Global Conan cache not found at {conan_home}, skipping{RESET}")
    if clean_only:
        print(f"{BOLD}{GREEN}Done cleaning up all data. Exit early as request to only clean up.{RESET}")
        return
    if clear_conan_cache and not (clean or clean_only):
        print(f"{YELLOW}Warning: --clear-conan-cache has no effect without --clean or --clean-only{RESET}")

    #------------------------------------------
    # Generated virtual environment
    print(f"{BOLD}{BLUE}------ GENERATING VENV ------{RESET}")
    if not Path(".venv").exists():
        run_with_spinner(
                [sys.executable, "-m", "venv", ".venv"],
                f"Creating python virtual environment...",
                verbose_logging,
                check=False)
        print(f"{GREEN}Virtual environment created{RESET}")
    else:
        print(f"{BOLD}{MAGENTA}Virtual environment already exists{RESET}")

    # Acquire the virtual environment, but don't activate it, else one does:
    # ".venv\Scripts\activate.bat" on Windows. 
    # Instead when running subprocess use this path! Also acquire the path for
    # conan installation already for later use.
    if os_name == "windows":
        python_path = r".venv\Scripts\python.exe"
        conan_cmd = r".venv\Scripts\conan.exe"
    else:
        python_path = ".venv/bin/python"
        conan_cmd = ".venv/bin/conan"
    print(f"{BOLD}{MAGENTA}Virtual environment path acquired:{python_path}{RESET}")

    # Prepare environment injection now (used for Conan remote login below, and for Conan installs later)
    venv_dir = Path(".venv").resolve()
    if os_name == "windows":
        venv_bin_dir = venv_dir / "Scripts"
        site_packages = venv_dir / "Lib" / "site-packages"
    else:
        venv_bin_dir = venv_dir / "bin"
        sp_list = list(venv_dir.glob("lib/python*/site-packages"))
        site_packages = sp_list[0] if sp_list else venv_dir

    conan_env = {
        **os.environ,
        "PATH": str(venv_bin_dir) + os.pathsep + os.environ.get("PATH", ""),
        "PYTHONPATH": str(site_packages) + os.pathsep + os.environ.get("PYTHONPATH", ""),
        "Python3_ROOT_DIR": str(venv_dir),
        "Python_ROOT_DIR": str(venv_dir)}
    print(f"{BOLD}{MAGENTA}Using Python for builds: {Path(python_path).resolve()}{RESET}")

    #------------------------------------------
    # Upgrade pip
    print(f"{BOLD}{BLUE}------ UPGRADING PIP ------{RESET}")

    result = run_with_spinner(
        [python_path, "-m", "pip", "install", "--upgrade", "pip"],
        "Upgrading pip...",
        verbose_logging,
        check=False)

    if result.returncode != 0:
        print(f"{RED}Failed to upgrade pip{RESET}")
        raise SystemExit(result.returncode)
    print(f"{GREEN}pip upgraded successfully{RESET}")

    #------------------------------------------
    # Check if conan installation already exists
    print(f"{BOLD}{BLUE}------ INSTALLING CONAN ------{RESET}")
    try:
        result_version = subprocess.run(
            [conan_cmd, "--version"],
            capture_output=True,
            text=True,
            check=True)
        version_text = (result_version.stdout or result_version.stderr).strip()
        version_only = version_text.split()[-1]
        print(f"{GREEN}Conan already available: {version_only}{RESET}")

    except FileNotFoundError:
        print(f"{YELLOW}Conan not found in virtual environment, installing...{RESET}")

        # Not found, try and install conan for virtual environment
        result = run_with_spinner(
            [python_path, "-m", "pip", "install", "conan"],
            "Installing Conan via pip...",
            verbose_logging,
            check=False)
        if result.returncode != 0:
            print(f"{BOLD}{RED}Failed to install Conan (exit {result.returncode}){RESET}")
            print((result.stdout or "") + (result.stderr or ""))
            raise SystemExit(result.returncode)

        # After installation, verify version as well
        result_version = subprocess.run(
            [conan_cmd, "--version"],
            capture_output=True,
            text=True,
            check=True)
        version_text = (result_version.stdout or result_version.stderr).strip()
        version_only = version_text.split()[-1]
        print(f"{GREEN}Conan installed correctly: {version_only}{RESET}")
    
    except subprocess.CalledProcessError as e:
        print(f"{BOLD}{RED}Conan exists but failed to run (exit {e.returncode}){RESET}")
        print((e.stdout or "") + (e.stderr or ""))
        raise SystemExit(e.returncode)
    
    #------------------------------------------
    # Create default conan profile if it does not exist
    print(f"{BOLD}{BLUE}------ GENERATING CONAN DEFAULT PROFILE ------{RESET}")
    conan_home = Path(os.environ.get("CONAN_HOME", Path.home() / ".conan2"))
    default_profile = conan_home / "profiles" / "default"
    detected_msvc_version = None
    try:
        if not default_profile.exists():
            print("Default Conan profile not found, create/detect it.")
            # FIX 5: Respect verbose_logging so output is suppressed unless --verbose is set
            subprocess.run(
                [conan_cmd, "profile", "detect"],
                capture_output=not verbose_logging,
                text=True,
                check=True)
        else:
            print("Default Conan profile already exists")

        result_default_profile = subprocess.run(
            [conan_cmd, "profile", "show", "--profile", "default"],
            capture_output=True,
            text=True,
            check=True)
        profile_text = (result_default_profile.stdout or result_default_profile.stderr).strip()
        if verbose_logging:
            print(f"Default Conan profile ready:\n{profile_text}")
        # Extract MSVC compiler version
        match = re.search(r'^compiler\.version\s*=\s*(\d+)', profile_text, re.MULTILINE)
        if match:
            detected_msvc_version = match.group(1)
            if verbose_logging:
                print(f"Detected MSVC compiler.version: {detected_msvc_version}")

    except subprocess.CalledProcessError as e:
        print(f"{BOLD}{RED}Problem handling Conan profile:{RESET}")
        print(e.stderr or e.stdout)

    # Detect Clang compiler version from the clang binary on PATH
    detected_clang_version = None
    if shutil.which("clang"):
        try:
            clang_ver_result = subprocess.run(
                ["clang", "--version"],
                capture_output=True, text=True, check=True)
            clang_match = re.search(r'version\s+(\d+)', clang_ver_result.stdout)
            if clang_match:
                detected_clang_version = clang_match.group(1)
                if verbose_logging:
                    print(f"Detected Clang compiler.version: {detected_clang_version}")
        except (subprocess.CalledProcessError, FileNotFoundError):
            pass

    # Detect GCC compiler version from the gcc binary on PATH
    detected_gcc_version = None
    if shutil.which("gcc"):
        try:
            gcc_ver_result = subprocess.run(
                ["gcc", "--version"],
                capture_output=True, text=True, check=True)
            gcc_match = re.search(r'gcc.*?\s+(\d+)', gcc_ver_result.stdout, re.IGNORECASE)
            if gcc_match:
                detected_gcc_version = gcc_match.group(1)
                if verbose_logging:
                    print(f"Detected GCC compiler.version: {detected_gcc_version}")
        except (subprocess.CalledProcessError, FileNotFoundError):
            pass

    #------------------------------------------
    # Set Conan's download behavior
    print(f"{BOLD}{BLUE}------ TUNING CONAN DOWNLOAD BEHAVIOR ------{RESET}")
    global_conf_path = conan_home / "global.conf"
    existing_lines = global_conf_path.read_text().splitlines() if global_conf_path.exists() else []
    kept_lines = [
        line for line in existing_lines
        if line.split("=", 1)[0].strip() not in LUDUS_CONAN_DOWNLOAD_CONF]
    new_lines = kept_lines + [f"{key}={value}" for key, value in LUDUS_CONAN_DOWNLOAD_CONF.items()]
    global_conf_path.parent.mkdir(parents=True, exist_ok=True)
    global_conf_path.write_text("\n".join(new_lines) + "\n")
    print(f"{GREEN}Set {', '.join(LUDUS_CONAN_DOWNLOAD_CONF)} in {global_conf_path}{RESET}")

    #------------------------------------------
    # Configure the private Conan server remote
    print(f"{BOLD}{BLUE}------ CONFIGURING CONAN REMOTE ------{RESET}")

    if no_conan_remote:
        print(f"{MAGENTA}Skipping private Conan server (--no-conan-remote){RESET}")
    else:
        subprocess.run(
            [conan_cmd, "remote", "add", LUDUS_CONAN_REMOTE_NAME, LUDUS_CONAN_SERVER_URL, "--index", "0", "--force"],
            check=True, capture_output=not verbose_logging, text=True, env=conan_env)

        list_users_result = subprocess.run(
            [conan_cmd, "remote", "list-users"],
            capture_output=True, text=True, env=conan_env)
        list_users_lines = (list_users_result.stdout or "").splitlines()
        already_logged_in = False
        for i, line in enumerate(list_users_lines):
            if line.strip() == f"{LUDUS_CONAN_REMOTE_NAME}:" and i + 1 < len(list_users_lines):
                next_line = list_users_lines[i + 1].strip()
                already_logged_in = next_line not in ("No user", "Username: None")
                break

        if already_logged_in:
            subprocess.run([conan_cmd, "remote", "enable", LUDUS_CONAN_REMOTE_NAME], env=conan_env)
            print(f"{GREEN}Already authenticated with {LUDUS_CONAN_REMOTE_NAME}{RESET}")
        else:
            remote_user = os.environ.get("CONAN_LOGIN_USERNAME", "")
            remote_password = os.environ.get("CONAN_PASSWORD", "")

            if not remote_user and not remote_password and not ci_mode and sys.stdin.isatty():
                answer = input(f"{CYAN}Configure credentials for '{LUDUS_CONAN_REMOTE_NAME}' now? [y/n]: {RESET}").strip().lower()
                if answer == "y":
                    remote_user = input("Username: ").strip()
                    remote_password = getpass.getpass("Password / Token: ")

            if remote_user and remote_password:
                login_result = subprocess.run(
                    [conan_cmd, "remote", "login", LUDUS_CONAN_REMOTE_NAME, remote_user, "-p", remote_password],
                    capture_output=True, text=True, env=conan_env)
                if login_result.returncode != 0:
                    print(f"{RED}Login to {LUDUS_CONAN_REMOTE_NAME} failed, disabling remote (conancenter fallback){RESET}")
                    if verbose_logging:
                        print((login_result.stdout or "") + (login_result.stderr or ""))
                    subprocess.run([conan_cmd, "remote", "disable", LUDUS_CONAN_REMOTE_NAME], env=conan_env)
                else:
                    subprocess.run([conan_cmd, "remote", "enable", LUDUS_CONAN_REMOTE_NAME], env=conan_env)
                    print(f"{GREEN}Logged into {LUDUS_CONAN_REMOTE_NAME} as {remote_user}{RESET}")
            else:
                print(f"{YELLOW}No credentials for {LUDUS_CONAN_REMOTE_NAME}, disabling it (conancenter fallback){RESET}")
                subprocess.run([conan_cmd, "remote", "disable", LUDUS_CONAN_REMOTE_NAME], env=conan_env)

    #------------------------------------------
    # Install Jinja2 for python code generator (e.g. used by GLAD)
    print(f"{BOLD}{BLUE}------ INSTALLING PYTHON BUILD DEPENDENCIES ------{RESET}")
    deps = ["jinja2"]

    result = run_with_spinner(
        [python_path, "-m", "pip", "install", *deps],
        "Installing Python build dependencies...",
        verbose_logging)

    if result.returncode != 0:
        print(f"{RED}Failed installing Python dependencies{RESET}")
        raise SystemExit(result.returncode)
    print(f"{GREEN}Python dependencies installed{RESET}")

    #------------------------------------------
    # Install Ninja for multi-config setup, if required
    print(f"{BOLD}{BLUE}------ INSTALLING NINJA ------{RESET}")
    ninja = shutil.which("ninja", path=str(venv_bin_dir) + os.pathsep + os.environ.get("PATH", ""))

    if not ninja:
        print(f"{YELLOW}Ninja not found, installing...{RESET}")
        try:
            if os_name == "windows":
                subprocess.run(
                    [python_path, "-m", "pip", "install", "ninja"],
                    check=True)
            elif os_name == "linux":
                subprocess.run(
                    ["sudo", "apt", "install", "-y", "ninja-build"],
                    check=True)
            else:
                raise RuntimeError("Unsupported OS")
        except subprocess.CalledProcessError as e:
            print(f"{BOLD}{RED}Failed to install Ninja{RESET}")
            raise SystemExit(e.returncode)

        # Re-query including venv Scripts dir where pip installs binaries.
        ninja = shutil.which("ninja", path=str(venv_bin_dir) + os.pathsep + os.environ.get("PATH", ""))
        if not ninja:
            print(f"{BOLD}{RED}Ninja was installed but is not yet on PATH. Please restart your terminal and re-run.{RESET}")
            raise SystemExit(1)

    # Check version (after install or if already present)
    result = subprocess.run(
        [ninja, "--version"],
        capture_output=True,
        text=True,
        check=True)
    version = (result.stdout or result.stderr).strip()
    print(f"{GREEN}Ninja available with version: {version}{RESET}")

    #------------------------------------------
    # Install all profiles acquired earlier
    print(f"{BOLD}{BLUE}------ CONAN INSTALL ------{RESET}")

    for profile in profiles:
        print(f"{CYAN}Installing profile:{RESET} {profile}")
        conan_install_cmd = [
            conan_cmd,
            "install",
            ".",
            "--build=missing",
            "-pr",
            f".profiles/{profile}"]

        # Adaptive compiler version override
        profile_version = _read_profile_compiler_version(f".profiles/{profile}")
        detected_version = None
        compiler_name = None

        if "msvc" in profile:
            compiler_name = "MSVC"
            detected_version = detected_msvc_version
        elif "clang" in profile:
            compiler_name = "Clang"
            detected_version = detected_clang_version
        elif "gcc" in profile:
            compiler_name = "GCC"
            detected_version = detected_gcc_version

        if compiler_name and detected_version:
            if profile_version and detected_version != profile_version:
                print(f"{BOLD}{YELLOW}  WARNING: Profile declares {compiler_name} version={profile_version}, "
                      f"but detected {compiler_name} {detected_version} on PATH. Overriding to {detected_version}.{RESET}")
            else:
                print(f"{GREEN}  OK:{compiler_name} version matches: {detected_version}{RESET}")
            conan_install_cmd += ["-s", f"compiler.version={detected_version}"]
        elif compiler_name and not detected_version:
            print(f"{YELLOW}  WARNING: {compiler_name} not detected on PATH, using profile version ({profile_version or 'unknown'}){RESET}")

        # On Windows with GCC, explicitly set the build profile to GCC as well,
        # preventing Conan from auto-detecting MSVC as the build compiler when VS is installed.
        if os_name == "windows" and "gcc" in profile:
            conan_install_cmd += ["--profile:build", f".profiles/{profile}"]
            if detected_version:
                conan_install_cmd += ["-s:b", f"compiler.version={detected_version}"]

        # Always set build_tests for Conan based on incoming argument we captured
        if build_tests:
            conan_install_cmd += ["-o", "build_tests=True"]
        else:
            conan_install_cmd += ["-o", "build_tests=False"]

        # Install now
        conan_install_result = run_with_spinner(
            conan_install_cmd,
            f"Conan installing {profile}...",
            verbose_logging,
            check=False,
            env=conan_env)

        if conan_install_result.returncode != 0:
            print(f"{BOLD}{RED}Conan install failed for {profile} (exit {conan_install_result.returncode}){RESET}")
            if not verbose_logging:
                print((conan_install_result.stdout or "") + (conan_install_result.stderr or ""))
            raise SystemExit(conan_install_result.returncode)

        if verbose_logging:
            print(f"{GREEN}Success:{RESET} {profile}")

    print(f"{GREEN}------ SETUP COMPLETE ------{RESET}")

if __name__ == "__main__":
    main()