#!/usr/bin/python

import subprocess
import argparse
from pathlib import Path

TESTS_DIR = Path("./tests")
FCC = "./fcc"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--update",
        action="store_true",
        help="Update .expected files with current output",
    )
    args = parser.parse_args()

    test_files = list(TESTS_DIR.glob("*.c"))

    for test in test_files:
        expected_file = test.with_suffix(".expected")

        result = subprocess.run([FCC, str(test)], capture_output=True, text=True)
        actual_output = result.stdout.strip()

        if args.update:
            expected_file.write_text(actual_output + "\n")
            print(f"[UPDATED] {expected_file.name}")
            continue

        if not expected_file.exists():
            print(f"[SKIP] Missing expected file for {test.name}")
            continue

        expected_output = expected_file.read_text().strip()

        if actual_output == expected_output:
            print(f"[PASS] {test.name}")
        else:
            print(f"[FAIL] {test.name}")
            print("Expected:")
            print(expected_output)
            print("Got:")
            print(actual_output)


if __name__ == "__main__":
    main()
