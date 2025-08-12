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

Remove-Item -Path "testing/binaries/*.exe"