@echo off
REM Nettoyage complet de l'état Git
cd /d c:\gclient

REM Supprimer tous les fichiers de merge
del /f /q ".git\MERGE*" 2>nul
del /f /q ".git\.MERGE*" 2>nul  
del /f /q ".git\AUTO_MERGE" 2>nul

REM Attendre un peu
timeout /t 1 /nobreak

REM Vérifier le statut
echo Configuration Git mise à jour
echo Email: Fourat.Attia@esprit.tn
echo.

REM Afficher les branches
git branch -a

echo.
echo Script completed successfully
pause
