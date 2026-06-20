#!/usr/bin/python

import subprocess
from pathlib import Path

TESTS_DIR = Path("./tests")
FCC = "./fcc"


def main():
    test_files = list(TESTS_DIR.glob("*.c"))

    for test in test_files:
        expected_file = test.with_suffix(".expected")

        if not expected_file.exists():
            print(f"[SKIP] Missing expected file for {test.name}")

        result = subprocess.run([FCC, str(test)], capture_output=True, text=True)

        actual_output = result.stdout.strip()
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
