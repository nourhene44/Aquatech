# Script pour recompiler le projet Qt
$qtInstallPath = "C:\Qt\6.7.3"
$minGWBinPath = "$qtInstallPath\mingw_64\bin"
$projectDir = "c:\Users\esrab\Downloads\Aquatech-CRUD1111"
$buildDir = "$projectDir\build\Desktop_Qt_6_7_3_MinGW_64_bit_qt_qt6-Debug"

# Ajouter Qt et MinGW au PATH
$env:PATH = "$minGWBinPath;$env:PATH"

# Vérifier que nous avons les outils
Write-Host "Vérification des outils..."
$qmake = Get-Command qmake -ErrorAction SilentlyContinue
$make = Get-Command mingw32-make -ErrorAction SilentlyContinue

if ($null -eq $qmake) {
    Write-Host "Tentative de trouver qmake dans $minGWBinPath\qmake.exe..."
    if (Test-Path "$minGWBinPath\qmake.exe") {
        & "$minGWBinPath\qmake.exe" --version
    } else {
        Write-Host "ERREUR: qmake not found!"
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
