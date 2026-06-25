#!/usr/bin/python

from pathlib import Path
import re
import argparse
import subprocess
from multiprocessing import Pool
import difflib

TESTS_DIR = Path("./tests")
FCC = "./fcc"


def get_passes_from_comment(test: Path):
    with test.open() as f:
        first_line = f.readline().strip()

    match = re.match(r"^//\s*(.+)$", first_line)
    if not match:
        return []

    passes = match.group(1).split()
    return [p.upper() for p in passes]


def get_fcc_command(test: Path):
    cmd = [FCC, str(test)]

    passes = get_passes_from_comment(test)
    cmd.extend(passes)

    return cmd


def run_test(args):
    test, update = args
    expected_file = test.with_suffix(".expected")

    cmd = get_fcc_command(test)
    result = subprocess.run(cmd, capture_output=True, text=True)
    actual_output = result.stdout.strip()

    if update:
        expected_file.write_text(actual_output + "\n")
        return (3, test.name, expected_file.name)  # updated

    if not expected_file.exists():
        return (2, test.name, None)  # skip

    expected_output = expected_file.read_text().strip()

    if actual_output == expected_output:
        return (0, test.name, None)  # pass

    return (
        1,  # fail
        test.name,
        {
            "expected": expected_output,
            "actual": actual_output,
        },
    )


def print_diff(expected, actual):
    diff = difflib.unified_diff(
        expected.splitlines(),
        actual.splitlines(),
        fromfile="expected",
        tofile="actual",
        lineterm="",
    )

    for line in diff:
        print(line)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--update",
        action="store_true",
        help="Update .expected files with current output",
    )
    args = parser.parse_args()

    test_files = list(TESTS_DIR.glob("*.c"))

    with Pool(5) as pool:
        results = pool.map(run_test, [(test, args.update) for test in test_files])

    failed = []

    for status, test_name, data in results:
        if status == 0:
            print(f"[PASS] {test_name}")
        elif status == 1:
            failed.append((test_name, data))
        elif status == 2:
            print(f"[SKIP] Missing expected file for {test_name}")
        elif status == 3:
            print(f"[UPDATED] {data}")

    if failed:
        for test_name, data in failed:
            print(f"\n[FAIL] {test_name}")
            print_diff(data["expected"], data["actual"])


if __name__ == "__main__":
    main()
