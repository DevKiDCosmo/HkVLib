import os

Import("env")


def _read_hosts_file(path: str) -> str:
    if not os.path.exists(path):
        return ""

    entries = []
    with open(path, "r", encoding="utf-8") as hosts_file:
        for raw_line in hosts_file:
            line = raw_line.strip()
            if not line or line.startswith("#"):
                continue
            entries.append(line)

    return " ".join(entries)


def _resolve_distcc_hosts(project_dir: str) -> str:
    hosts_from_env = os.environ.get("DISTCC_HOSTS", "").strip()
    if hosts_from_env:
        return hosts_from_env

    hosts_from_custom_env = os.environ.get("PIO_DISTCC_HOSTS", "").strip()
    if hosts_from_custom_env:
        return hosts_from_custom_env

    hosts_file = os.path.join(project_dir, ".distcc", "hosts")
    hosts_from_file = _read_hosts_file(hosts_file).strip()
    if hosts_from_file:
        return hosts_from_file

    return "localhost/24,cpp"


def _configure_distcc_for_idf() -> None:
    frameworks = env.get("PIOFRAMEWORK", [])
    if isinstance(frameworks, str):
        frameworks = [frameworks]

    has_espidf = "espidf" in frameworks

    print(f"PIO frameworks: {frameworks}")
    print(f"SCons bootstrap CC: {env.get('CC')}")
    print(f"SCons bootstrap CXX: {env.get('CXX')}")

    project_dir = env.subst("$PROJECT_DIR")
    distcc_hosts = _resolve_distcc_hosts(project_dir)

    # Ensure cross-toolchain is visible for wrappers and local fallback.
    toolchain_bin = os.path.expanduser("~/.platformio/packages/toolchain-xtensa-esp32/bin")
    env.AppendENVPath("PATH", toolchain_bin)

    # Use distcc directly as compiler launcher (no ccache wrapper).
    # This ensures all compilation jobs are distributed to worker machines,
    # maximizing utilization across the cluster.
    env["ENV"]["CMAKE_C_COMPILER_LAUNCHER"] = "distcc"
    env["ENV"]["CMAKE_CXX_COMPILER_LAUNCHER"] = "distcc"

    # Distcc behavior / diagnostics.
    env["ENV"]["DISTCC_FALLBACK"] = "1"
    env["ENV"]["DISTCC_VERBOSE"] = os.environ.get("DISTCC_VERBOSE", "1")
    env["ENV"]["DISTCC_LOG"] = os.environ.get(
        "DISTCC_LOG", os.path.join(project_dir, ".distcc", "distcc-client.log")
    )

    env["ENV"]["DISTCC_HOSTS"] = distcc_hosts

    # For pure Arduino builds, direct CC/CXX override can be useful.
    # For ESP-IDF builds this does NOT control actual compiler commands.
    if not has_espidf:
        env.Replace(CC="xtensa-esp32-elf-gcc", CXX="xtensa-esp32-elf-g++")

    print(f"[distcc] ESP-IDF mode: {has_espidf}")
    print(f"[distcc] CMAKE_C_COMPILER_LAUNCHER=distcc")
    print(f"[distcc] CMAKE_CXX_COMPILER_LAUNCHER=distcc")
    print(f"[distcc] DISTCC_HOSTS={env['ENV'].get('DISTCC_HOSTS', '')}")
    print(f"[distcc] DISTCC_LOG={env['ENV'].get('DISTCC_LOG')}")


_configure_distcc_for_idf()
