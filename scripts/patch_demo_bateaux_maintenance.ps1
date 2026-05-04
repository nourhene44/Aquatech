param(
    [string]$Dsn = "Nourhene",
    [string]$User = "hr",
    [string]$Password = "hr"
)

$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.Data

$connectionString = "DSN=$Dsn;Uid=$User;Pwd=$Password;"
$conn = [System.Data.Odbc.OdbcConnection]::new($connectionString)

function Invoke-Query {
    param(
        [System.Data.Odbc.OdbcConnection]$Connection,
        [string]$Sql
    )

    $cmd = $Connection.CreateCommand()
    $cmd.CommandText = $Sql

    $reader = $cmd.ExecuteReader()
    try {
        $table = New-Object System.Data.DataTable
        $table.Load($reader)
        return $table
    }
    finally {
        $reader.Close()
    }
}

function Invoke-NonQuery {
    param(
        [System.Data.Odbc.OdbcConnection]$Connection,
        [System.Data.Odbc.OdbcTransaction]$Transaction,
        [string]$Sql
    )

    $cmd = $Connection.CreateCommand()
    $cmd.Transaction = $Transaction
    $cmd.CommandText = $Sql
    [void]$cmd.ExecuteNonQuery()
}

try {
    $conn.Open()

    Write-Host "Connected to DSN '$Dsn' as '$User'." -ForegroundColor Green

    $before = Invoke-Query -Connection $conn -Sql @"
SELECT ID_BATEAU,
       NOM,
       DATE_DERNIERE_MAINTENANCE,
       PROCHAINE_MAINTENANCE AS "Prochaine maintenance",
       (DATE_DERNIERE_MAINTENANCE + PROCHAINE_MAINTENANCE) AS NEXT_DUE,
       TRUNC((DATE_DERNIERE_MAINTENANCE + PROCHAINE_MAINTENANCE) - SYSDATE) AS DAYS_REMAINING
FROM BATEAUX
WHERE ID_BATEAU IN (261008, 261009)
ORDER BY ID_BATEAU
"@

    Write-Host "Before:" -ForegroundColor Cyan
    $before | Format-Table -AutoSize | Out-String | Write-Host

    $tx = $conn.BeginTransaction()
    try {
        # URGENT (dans 5 jours): freq 37 => base = today - (37-5)
        Invoke-NonQuery -Connection $conn -Transaction $tx -Sql @"
UPDATE BATEAUX
SET DATE_DERNIERE_MAINTENANCE = TRUNC(SYSDATE) - (37 - 5),
    PROCHAINE_MAINTENANCE     = 37
WHERE ID_BATEAU = 261009
"@

        # AVERTISSEMENT (dans 20 jours): freq 50 => base = today - (50-20)
        Invoke-NonQuery -Connection $conn -Transaction $tx -Sql @"
UPDATE BATEAUX
SET DATE_DERNIERE_MAINTENANCE = TRUNC(SYSDATE) - (50 - 20),
    PROCHAINE_MAINTENANCE     = 50
WHERE ID_BATEAU = 261008
"@

        $tx.Commit()
    }
    catch {
        try { $tx.Rollback() } catch { }
        throw
    }

    $after = Invoke-Query -Connection $conn -Sql @"
SELECT ID_BATEAU,
       NOM,
       DATE_DERNIERE_MAINTENANCE,
       PROCHAINE_MAINTENANCE AS "Prochaine maintenance",
       (DATE_DERNIERE_MAINTENANCE + PROCHAINE_MAINTENANCE) AS NEXT_DUE,
       TRUNC((DATE_DERNIERE_MAINTENANCE + PROCHAINE_MAINTENANCE) - SYSDATE) AS DAYS_REMAINING
FROM BATEAUX
WHERE ID_BATEAU IN (261008, 261009)
ORDER BY ID_BATEAU
"@

    Write-Host "After:" -ForegroundColor Cyan
    $after | Format-Table -AutoSize | Out-String | Write-Host

    Write-Host "Done. Refresh la page 'Bateaux' dans l'app pour recalculer les alertes." -ForegroundColor Green
}
finally {
    if ($conn.State -eq [System.Data.ConnectionState]::Open) {
        $conn.Close()
    }
}
