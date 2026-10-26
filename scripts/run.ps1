[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$executable = Join-Path $projectRoot "build\windows-msvc\$Configuration\calc3D.exe"
if (-not (Test-Path -LiteralPath $executable)) {
    throw "Executavel nao encontrado. Execute primeiro: .\scripts\build.ps1 -Configuration $Configuration"
}
& $executable
