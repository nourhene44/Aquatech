@echo off
setlocal enabledelayedexpansion

REM Configuration Git finale et push vers GitHub
cd /d c:\gclient

REM Nettoyer les fichiers de merge bloquants
del /f /q ".git\MERGE_MSG" 2>nul
del /f /q ".git\MERGE_HEAD" 2>nul
del /f /q ".git\MERGE_MODE" 2>nul
del /f /q ".git\AUTO_MERGE" 2>nul
del /f /q ".git\.MERGE*.swp" 2>nul

REM Attendre
timeout /t 1 /nobreak

REM Configurer l'email correctement
git config user.name "fouratattia"
git config user.email "Fourat.Attia@esprit.tn"

REM Ajouter la remote si elle n'existe pas
git config --get remote.origin.url >nul 2>&1
if !errorlevel! neq 0 (
    git remote add origin https://github.com/nourhene44/Aquatech.git
)

REM Vérifier le statut
echo.
echo ========================================
echo Statut du repository:
echo ========================================
git status

REM Créer la branche GClient et switch
git checkout -b GClient 2>nul
git checkout GClient 2>nul

REM Ajouter tous les fichiers
git add .

REM Faire le commit si des changements existent
git diff-index --quiet HEAD -- 2>nul
if !errorlevel! neq 0 (
    echo.
    echo Création d'un nouveau commit...
    git commit -m "GClient - Qt project avec configuration correcte"
) else (
    echo.
    echo Aucun nouveau fichier à commiter
)

REM Push vers la branche GClient
echo.
echo ========================================
echo Début du push vers GitHub...
echo ========================================
git push -u origin GClient

echo.
echo ========================================
echo Mail configuré: Fourat.Attia@esprit.tn
echo Utilisateur: fouratattia
echo Repository: https://github.com/nourhene44/Aquatech.git
echo Branche: GClient
echo ========================================

pause
