# Script pour recompiler le projet Qt depuis ce depot
$qtRoot = "C:\Qt\6.7.3\mingw_64"
$qtBinPath = Join-Path $qtRoot "bin"
$minGwBinPath = "C:\Qt\Tools\mingw1120_64\bin"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectFile = Join-Path $scriptDir "projet.pro"
$buildDir = Join-Path $scriptDir "build\Desktop_Qt_6_7_3_MinGW_64_bit-Debug"
$exeDir = Join-Path $buildDir "debug"
$exePath = Join-Path $exeDir "projet.exe"

function Require-Path($path, $label) {
    if (-not (Test-Path $path)) {
        Write-Error "$label introuvable: $path"
        exit 1
    }
}

function Copy-IfExists($source, $destination) {
    if (Test-Path $source) {
        Copy-Item -Path $source -Destination $destination -Force
    }
}

Require-Path $qtBinPath "Qt bin"
Require-Path $minGwBinPath "MinGW bin"
Require-Path $projectFile "Fichier projet"

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
New-Item -ItemType Directory -Force -Path $exeDir | Out-Null

$env:PATH = "$qtBinPath;$minGwBinPath;$env:PATH"
$qmakeExe = Join-Path $qtBinPath "qmake.exe"
$makeExe = Join-Path $minGwBinPath "mingw32-make.exe"

Write-Host "Generation de Makefile.Debug..."
Push-Location $buildDir
& $qmakeExe -o Makefile.Debug ..\..\projet.pro -spec win32-g++ "CONFIG+=debug" "CONFIG+=qml_debug"
if ($LASTEXITCODE -ne 0) {
    Pop-Location
    Write-Error "Echec qmake."
    exit 1
}

Write-Host "Compilation en cours..."
& $makeExe -f Makefile.Debug -B debug/projet.exe
if ($LASTEXITCODE -ne 0) {
    Pop-Location
    Write-Error "Compilation echouee."
    exit 1
}
Pop-Location

Write-Host "Copie des runtimes Qt et MinGW..."
$qtDlls = @(
    "Qt6Core.dll",
    "Qt6Gui.dll",
    "Qt6Multimedia.dll",
    "Qt6Network.dll",
    "Qt6Sql.dll",
    "Qt6Widgets.dll"
)

$mingwDlls = @(
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll"
)

foreach ($dll in $qtDlls) {
    Copy-IfExists (Join-Path $qtBinPath $dll) $exeDir
}

foreach ($dll in $mingwDlls) {
    Copy-IfExists (Join-Path $minGwBinPath $dll) $exeDir
}

$pluginDirs = @(
    "generic",
    "iconengines",
    "imageformats",
    "multimedia",
    "networkinformation",
    "platforminputcontexts",
    "platforms",
    "sqldrivers",
    "styles",
    "tls"
)

foreach ($pluginDir in $pluginDirs) {
    $sourceDir = Join-Path $qtRoot "plugins\$pluginDir"
    $targetDir = Join-Path $exeDir $pluginDir
    if (Test-Path $sourceDir) {
        New-Item -ItemType Directory -Force -Path $targetDir | Out-Null
        Copy-Item -Path (Join-Path $sourceDir "*") -Destination $targetDir -Recurse -Force
    }
}

if (Test-Path $exePath) {
    $exeInfo = Get-Item $exePath
    Write-Host "Compilation reussie."
    Write-Host "Executable: $exePath"
    Write-Host "Taille: $($exeInfo.Length) octets"
} else {
    Write-Error "Executable introuvable apres compilation."
    exit 1
}
