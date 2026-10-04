param([string]$CompilerPath = '')
$ErrorActionPreference = 'Stop'
# Builds only this source file and runs its automated tests.
# Does not install software or change your PowerShell execution policy.
try {
    if (-not $CompilerPath) {
        $taskCompiler = Get-Command 'g++' -ErrorAction SilentlyContinue
        if (-not $taskCompiler) {
            $taskCompiler = Get-Command 'clang++' -ErrorAction SilentlyContinue
        }
        if (-not $taskCompiler) {
            throw 'No C++ compiler found. Configure g++/clang++ or pass -CompilerPath with its full executable path. See README.md.'
        }
        $CompilerPath = $taskCompiler.Source
    }
    $taskSource = Join-Path $PSScriptRoot 'DSU_Complete.cpp'
    $taskBinary = Join-Path $PSScriptRoot 'dsu.exe'
    & $CompilerPath '-std=c++17' '-O2' '-Wall' '-Wextra' '-pedantic' $taskSource '-o' $taskBinary
    if ($LASTEXITCODE -ne 0) { throw 'Compilation failed. Read the compiler messages above.' }
    & $taskBinary '--tests'
    if ($LASTEXITCODE -ne 0) { throw 'One or more tests failed.' }
    Write-Host 'Build and tests finished successfully. Run .\dsu.exe --all for the demonstrations.'
} catch {
    Write-Error $_
    exit 1
}
