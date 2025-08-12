$original_location = (Get-Location).Path

Write-Output "Locating the Flint-Engine directory"
while ((Get-Item -Path .).Name -ne "Flint-Engine") {
    if ((Get-Location).Path -eq [System.IO.Path]::GetPathRoot((Get-Location).Path)) {
        Write-Error "Failed to locate Flint-Engine directory."
        Write-Error "Make sure the directory is named correctly and retry."
        Set-Location $original_location
        exit 1  # or use `return` if inside a function
    }
    Set-Location ..
}

Write-Output "Successfully located Flint-Engine directory"

$new_location = (Get-Location).Path
$test_env = Join-Path $new_location 'test-env'

$script_path = Join-Path $original_location 'test-init.ps1'
if (Test-Path $script_path) {
    Write-Output "Successfully located test-init.ps1 script"
    & $script_path $test_env
} else {
    Write-Error "Cannot find test-init.ps1 at $script_path"
}

Set-Location $original_location
