$original_location = (Get-Location).Path
while ((Get-Item -Path .).Name -ne "Flint-Engine") {
    Set-Location ..
}
$new_location = (Get-Location).Path
$test_env = Join-Path $new_location 'test-env'
gcc testing/test-init.c src/icd-functions/*.c src/state-functions/*.c src/struct-functions/*.c src/tooling/*.c -Iinclude -o test 
$test_env | ./test.exe
cd $original_location