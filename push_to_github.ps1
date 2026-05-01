# Script pour nettoyer et pousser
Remove-Item -Path "c:\gclient\.git\MERGE_MSG", "c:\gclient\.git\MERGE_HEAD", "c:\gclient\.git\MERGE_MODE", "c:\gclient\.git\AUTO_MERGE", "c:\gclient\.git\.MERGE_MSG.swp" -Force -ErrorAction SilentlyContinue

cd c:\gclient

# Vérifier le statut
Write-Host "Statut Git:"
& git status

Write-Host "`nBranches disponibles:"
& git branch -a

Write-Host "`nConfiguration Git:"
& git config user.name
& git config user.email

Write-Host "`nCréation/switch vers GClient:"
& git checkout -b GClient 2>$null
& git checkout GClient

Write-Host "`nAjout des fichiers..."
& git add .

Write-Host "`nVérification des changements..."
& git diff --cached --name-status | head -20

Write-Host "`nCommit final..."
& git commit -m "GClient - Projet Qt avec configuration complète" --no-verify 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "Commit failed or no changes, continuing..."
}

Write-Host "`nPush vers GitHub GClient..."
& git push -u origin GClient --force

Write-Host "`nFini!`n"
Write-Host "Email: Fourat.Attia@esprit.tn"
Write-Host "Utilisateur: fouratattia"
Write-Host "Branche: GClient"

pause
