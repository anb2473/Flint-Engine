param(
    [string]$test_env_path
)

Write-Output "Compiling test-init.c"
gcc "testing/test-init.c" src/icd-functions/*.c src/state-functions/*.c src/struct-functions/*.c src/tooling/*.c -Iinclude -o "testing/binaries/test-init"

if ($LASTEXITCODE -ne 0) {
    Write-Error "Compilation failed"
    exit $LASTEXITCODE
}

Write-Output "Running executable"
if (Test-Path "testing/binaries/test-init.exe") {
    $test_env_path | ./"testing/binaries/test-init.exe"
    if ($LASTEXITCODE -ne 0) {
        Write-Error "test-init ended with non 0 exit code"
        exit $LASTEXITCODE
    }
} else {
    Write-Error "Failed to locate test-init executable"
    exit 1
}

Write-Output "Completed test-init"
exit 0