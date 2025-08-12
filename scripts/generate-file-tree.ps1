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
$readme_location = Join-Path $new_location 'FILE_GRAPH.md'

Write-Output "Writing file tree to FILE_GRAPH.md"
tree /F /A > $readme_location

Set-Location $original_location