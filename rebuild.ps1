# Script pour recompiler le projet Qt
# Note: adapte aux chemins locaux (workspace) + toolchain Qt/MinGW.
$qtInstallPath = "C:\Qt\6.7.3"
$minGWBinPath = "C:\Qt\Tools\mingw1120_64\bin"

# Le dossier projet = dossier du script
$projectDir = $PSScriptRoot

# Utiliser le build déjà généré dans ce dépôt
$buildDir = Join-Path $projectDir "build\Desktop_Qt_6_7_3_MinGW_64_bit-Debug"

# Ajouter Qt et MinGW au PATH
$env:PATH = "$minGWBinPath;$env:PATH"
$env:PATH = "$qtInstallPath\mingw_64\bin;$env:PATH"

# Vérifier que nous avons les outils
Write-Host "Vérification des outils..."
$qmake = Get-Command qmake -ErrorAction SilentlyContinue
$make = Get-Command mingw32-make -ErrorAction SilentlyContinue

if ($null -eq $qmake) {
    Write-Host "Tentative de trouver qmake dans $minGWBinPath\qmake.exe..."
    if (Test-Path "$qtInstallPath\mingw_64\bin\qmake.exe") {
        & "$qtInstallPath\mingw_64\bin\qmake.exe" --version
    } else {
        Write-Host "ERREUR: qmake not found!"
        exit 1
    }
}

if ($null -eq $make) {
    Write-Host "Tentative de trouver mingw32-make dans $minGWBinPath\mingw32-make.exe..."
    if (Test-Path "$minGWBinPath\mingw32-make.exe") {
        & "$minGWBinPath\mingw32-make.exe" --version
    } else {
        Write-Host "ERREUR: mingw32-make not found!"
        exit 1
    }
}

Write-Host "Changement vers le répertoire de build..."
Push-Location $buildDir

Write-Host "Lancement de la compilation..."
& mingw32-make -j4

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation réussie!"
    Write-Host "Exécutable devrait être disponible à: $buildDir\debug\projet.exe"
} else {
    Write-Host "Compilation échouée!"
    exit 1
}

Pop-Location
