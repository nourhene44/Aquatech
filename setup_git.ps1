# Nettoyage de l'état de merge
Remove-Item -Path "c:\gclient\.git\MERGE_MSG" -Force -ErrorAction SilentlyContinue
Remove-Item -Path "c:\gclient\.git\MERGE_HEAD" -Force -ErrorAction SilentlyContinue

# Configuration Git correcte
cd c:\gclient

# Réinitialiser et re-configurer
$gitConfigPath = "c:\gclient\.git\config"
$config = @"
[core]
	repositoryformatversion = 0
	filemode = false
	bare = false
	logallrefupdates = true
	symlinks = false
	ignorecase = true
[user]
	name = fouratattia
	email = Fourat.Attia@esprit.tn
[remote "origin"]
	url = https://github.com/nourhene44/Aquatech.git
	fetch = +refs/heads/*:refs/remotes/origin/*
[branch "GClient"]
	remote = origin
	merge = refs/heads/GClient
"@

Set-Content -Path $gitConfigPath -Value $config -Encoding UTF8

Write-Host "Git configuration updated successfully"
Write-Host "Email: Fourat.Attia@esprit.tn"

# Vérifier le statut
& git status
