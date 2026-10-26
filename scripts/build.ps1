[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',
    [string]$QtRoot = $env:QT_ROOT
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $projectRoot 'build\windows-msvc'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'

if (-not (Test-Path -LiteralPath $vswhere)) {
    throw 'Instale o Visual Studio Build Tools 2022 com as ferramentas de desktop C++.'
}
$vsPath = & $vswhere -latest -products '*' -version '[17.0,18.0)' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) {
    throw 'MSVC 2022 x64 nao encontrado. Instale a carga de trabalho de desktop C++.'
}

$cmake = Join-Path $vsPath 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
if (-not (Test-Path -LiteralPath $cmake)) {
    $cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakeCommand) { throw 'CMake nao encontrado. Instale as ferramentas CMake do Visual Studio.' }
    $cmake = $cmakeCommand.Source
}

if (-not $QtRoot -and (Test-Path -LiteralPath 'C:\Qt')) {
    $qtVersion = Get-ChildItem -LiteralPath 'C:\Qt' -Directory |
        Where-Object { $_.Name -match '^6\.\d+\.\d+$' } |
        Sort-Object { [version]$_.Name } -Descending |
        Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'msvc2022_64\lib\cmake\Qt6\Qt6Config.cmake') } |
        Select-Object -First 1
    if ($qtVersion) { $QtRoot = Join-Path $qtVersion.FullName 'msvc2022_64' }
}
if (-not $QtRoot -or -not (Test-Path -LiteralPath (Join-Path $QtRoot 'lib\cmake\Qt6\Qt6Config.cmake'))) {
    throw 'Qt MSVC 2022 x64 nao encontrado. Informe -QtRoot ou defina QT_ROOT com a pasta do kit.'
}
$deployQt = Join-Path $QtRoot 'bin\windeployqt.exe'
if (-not (Test-Path -LiteralPath $deployQt)) { throw 'windeployqt nao encontrado no kit Qt.' }

Write-Host "Qt: $QtRoot"
& $cmake -S $projectRoot -B $buildDir -G 'Visual Studio 17 2022' -A x64 "-DCMAKE_PREFIX_PATH=$QtRoot"
if ($LASTEXITCODE -ne 0) { throw 'Falha na configuracao do CMake.' }
& $cmake --build $buildDir --config $Configuration --parallel
if ($LASTEXITCODE -ne 0) { throw 'Falha na compilacao.' }

$qtMode = if ($Configuration -eq 'Debug') { '--debug' } else { '--release' }
$executable = Join-Path $buildDir "$Configuration\calc3D.exe"
$previousVcInstallDir = $env:VCINSTALLDIR
try {
    $env:VCINSTALLDIR = Join-Path $vsPath 'VC'
    & $deployQt $qtMode --no-translations $executable
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao copiar as dependencias Qt.' }
}
finally {
    $env:VCINSTALLDIR = $previousVcInstallDir
}
Write-Host "Executavel pronto: $executable"
