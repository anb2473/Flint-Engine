# 🧪 Flint Engine – Testing Guide (Windows Only)

This document explains how to compile and run Flint Engine test scripts on Windows using PowerShell.
All tests are written in PowerShell (.ps1) and are designed to run on the Windows.

**This does not mean the engine itself can only be run on Windows, however testing support for other operating systems is not included.** Feel free to read the `CONTRIBUTING.md` file, and add scripts and documentation for other operating systems.

## 📂 Test Environment Structure

Your test environment should be structured as shown in the `FILE_GRAPH.md` file. If you have changed any file names, be sure to apply those changes throughout the entire project. Specifically, remember to:

1. Adjust all include paths in the `/include` and `/src` directories.
2. Adjust paths in the `ps1` scripts. (Specifically, if you change the name of the directory to anything other than `Flint-Engine` you will need to edit the `ps1` scripts to search for your new directory name)

## ▶️ Running a Test

This project contains testing `ps1` scripts to automate testing.

1. Enter the scripts directory. if you are in the root `Flint-Engine` directory,`cd testing/scripts`
2. Run the `full-test.ps1` script to run all tests: `./full-test.ps1`. You can also run each test individually.

## ⚠️Important Notes

- All test binaries are stored in `testing/binaries/` to keep the workspace clean.
- All test scripts exit with the program’s exit code so they can be chained in CI/CD pipelines.
- If one test fails the full test script will still continue to then next test.

## ✅ Best Practices

- Document each test’s purpose at the top of its `.ps1` file.
- Keep tests modular – put each test in a separate `.ps1` file.
- Clean up binaries periodically with the `cls-binaries.ps1` script inside the `scripts` directory. (**NOT** the `testing/scripts` directory. You can run the script with `./cls-binaries.ps1`)

## ⚙️ All Tests

| 🚩 Test Name | 📋 Description                      | 🛠️ Script/Command               | 📝 Notes                              |
|--------------|-----------------------------------|--------------------------------|-------------------------------------|
| init         | Tests creating a new database tooling | `./test-init.ps1` (PowerShell) | 🪟 Windows only; runs `test-init.c` |
