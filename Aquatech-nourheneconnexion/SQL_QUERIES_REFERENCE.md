# SQL Queries for CAPTURES CRUD Operations

## Overview

This document contains all SQL queries used in the CRUD system for the CAPTURES table. Each query is formatted for Oracle database via ODBC connection.

## Database Connection

```
DSN: Taher
User: hr
Password: hr
Database: Oracle 11g+
Driver: ODBC
```

## Table Schema

```sql
CREATE TABLE CAPTURES (
    ID_CAPTURE      NUMBER PRIMARY KEY,
    ID_BATEAU       NUMBER,                  -- NULL allowed
    TYPE_POISSON    VARCHAR2(50) NOT NULL,
    QUANTITE        NUMBER NOT NULL,
    POIDS           NUMBER(10,2) NOT NULL,
    DATE_CAPTURE    DATE NOT NULL
);
```

## 1. SELECT Queries (READ Operations)

### 1.1 Load All Captures

**Purpose**: Populate the QTableWidget with all records from database

**Query**:
```sql
SELECT 
    "ID_CAPTURE",
    "ID_BATEAU",
    "TYPE_POISSON",
    "QUANTITE",
    "POIDS",
    "DATE_CAPTURE"
FROM CAPTURES
ORDER BY "ID_CAPTURE";
```

**C++ Implementation**:
```cpp
QSqlQuery query;
query.prepare("SELECT \"ID_CAPTURE\", \"ID_BATEAU\", \"TYPE_POISSON\", \"QUANTITE\", \"POIDS\", \"DATE_CAPTURE\" "
              "FROM CAPTURES "
              "ORDER BY \"ID_CAPTURE\"");

if (query.exec()) {
    while (query.next()) {
        int idCapture = query.value(0).toInt();
        int idBateau = query.value(1).toInt();
        QString type = query.value(2).toString();
        int quantity = query.value(3).toInt();
        double weight = query.value(4).toDouble();
        QDate date = query.value(5).toDate();
        
        // Add to table widget row
    }
}
```

### 1.2 Find Capture by ID

**Purpose**: Verify capture exists before updating

**Query**:
```sql
SELECT COUNT(*) FROM CAPTURES WHERE "ID_CAPTURE" = ?;
```

**C++ Implementation**:
```cpp
QSqlQuery query;
query.prepare("SELECT COUNT(*) FROM CAPTURES WHERE \"ID_CAPTURE\" = :id");
query.bindValue(":id", idCapture);

if (query.exec() && query.next()) {
    int count = query.value(0).toInt();
    bool exists = (count > 0);
}
```

### 1.3 Get Single Capture Details

**Purpose**: Load specific capture data for editing

**Query**:
```sql
SELECT 
    "ID_BATEAU",
    "TYPE_POISSON",
    "QUANTITE",
    "POIDS",
    "DATE_CAPTURE"
FROM CAPTURES
WHERE "ID_CAPTURE" = ?;
```

**C++ Implementation**:
```cpp
QSqlQuery query;
query.prepare("SELECT \"ID_BATEAU\", \"TYPE_POISSON\", \"QUANTITE\", \"POIDS\", \"DATE_CAPTURE\" "
              "FROM CAPTURES "
              "WHERE \"ID_CAPTURE\" = :id");
query.bindValue(":id", idCapture);

if (query.exec() && query.next()) {
    int idBateau = query.value(0).toInt();
    QString type = query.value(1).toString();
    int quantite = query.value(2).toInt();
    double poids = query.value(3).toDouble();
    QDate date = query.value(4).toDate();
    
    // Populate form fields
}
```

## 2. INSERT Queries (CREATE Operations)

### 2.1 Add New Capture

**Purpose**: Insert new capture record from form submission

**Query**:
```sql
INSERT INTO CAPTURES (
    "ID_CAPTURE",
    "ID_BATEAU",
    "TYPE_POISSON",
    "QUANTITE",
    "POIDS",
    "DATE_CAPTURE"
) VALUES (?, ?, ?, ?, ?, ?);
```

**C++ Implementation** (from captures.cpp):
```cpp
bool Captures::ajouter()
{
    QSqlQuery query;
    query.prepare("INSERT INTO captures "
                  "(ID_CAPTURE, ID_BATEAU, TYPE_POISSON, QUANTITE, POIDS, DATE_CAPTURE) "
                  "VALUES (:idCapture, :idBateau, :type, :quantite, :poids, :dateCapture)");
    
    query.bindValue(":idCapture", idCapture);
    query.bindValue(":idBateau", idBateau == -1 ? QVariant() : idBateau);  // NULL if -1
    query.bindValue(":type", typePoisson);
    query.bindValue(":quantite", quantite);
    query.bindValue(":poids", poids);
    query.bindValue(":dateCapture", QVariant(dateCapture));
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        m_lastQuery = query.lastQuery();
        return false;
    }
    
    QSqlDatabase::database().commit();
    return true;
}
```

## 3. UPDATE Queries (EDIT Operations)

### 3.1 Update Capture Record

**Purpose**: Modify existing capture data

**Query**:
```sql
UPDATE CAPTURES
SET 
    "ID_BATEAU" = ?,
    "TYPE_POISSON" = ?,
    "QUANTITE" = ?,
    "POIDS" = ?,
    "DATE_CAPTURE" = ?
WHERE "ID_CAPTURE" = ?;
```

**C++ Implementation** (from GQuai.cpp):
```cpp
QSqlQuery updateQuery;
updateQuery.prepare(
    "UPDATE CAPTURES SET "
    "\"ID_BATEAU\" = :idBateau, "
    "\"TYPE_POISSON\" = :typePoisson, "
    "\"QUANTITE\" = :quantite, "
    "\"POIDS\" = :poids, "
    "\"DATE_CAPTURE\" = :dateCapture "
    "WHERE \"ID_CAPTURE\" = :idCapture"
);

updateQuery.bindValue(":idBateau", idBateau);
updateQuery.bindValue(":typePoisson", typePoisson);
updateQuery.bindValue(":quantite", quantite);
updateQuery.bindValue(":poids", poids);
updateQuery.bindValue(":dateCapture", dateCapture);
updateQuery.bindValue(":idCapture", idCapture);

if (!updateQuery.exec()) {
    QString errMsg = updateQuery.lastError().text();
    QMessageBox::warning(this, "Erreur Base de Données", 
                        "Impossible de modifier: " + errMsg);
    return;
}

QSqlDatabase::database().commit();
QMessageBox::information(this, "Succès", "Capture modifiée!");
```

## 4. DELETE Queries (REMOVE Operations)

### 4.1 Delete Capture by ID

**Purpose**: Remove capture record from database

**Query**:
```sql
DELETE FROM CAPTURES
WHERE "ID_CAPTURE" = ?;
```

**C++ Implementation** (from GQuai.cpp):
```cpp
void MainWindow::deleteCaptureRow(int row)
{
    QTableWidget *tbl = ui->cap_tableWidget;
    int idCapture = tbl->item(row, 0)->text().toInt();
    
    // Confirmation dialog
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirmation",
        QString("Êtes-vous sûr de vouloir supprimer la capture #%1 ?").arg(idCapture),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM CAPTURES WHERE \"ID_CAPTURE\" = :idCapture");
        deleteQuery.bindValue(":idCapture", idCapture);
        
        if (deleteQuery.exec()) {
            QSqlDatabase::database().commit();
            QMessageBox::information(this, "Succès", "Capture supprimée!");
            loadCapturesTable();  // Refresh table
        } else {
            QString errMsg = deleteQuery.lastError().text();
            QMessageBox::warning(this, "Erreur Base de Données", 
                                "Impossible de supprimer: " + errMsg);
        }
    }
}
```

### 4.2 Delete All Captures (Admin Only)

**Purpose**: Clear entire table (be careful!)

**Query**:
```sql
DELETE FROM CAPTURES;
```

**C++ Implementation**:
```cpp
QSqlQuery deleteAllQuery;
deleteAllQuery.prepare("DELETE FROM CAPTURES");

if (deleteAllQuery.exec()) {
    QSqlDatabase::database().commit();
    qDebug() << "All captures deleted";
} else {
    qDebug() << "Error:" << deleteAllQuery.lastError().text();
}
```

## 5. Transaction Management

### 5.1 Commit Transaction

After any INSERT/UPDATE/DELETE:

```cpp
if (query.exec()) {
    QSqlDatabase::database().commit();  // Commit changes to Oracle
    // Success handling
} else {
    // Error - automatic rollback occurs
    qDebug() << "Query failed:" << query.lastError().text();
}
```

### 5.2 Rollback Transaction (if needed)

```cpp
QSqlDatabase db = QSqlDatabase::database();
if (someErrorCondition) {
    db.rollback();  // Undo changes
    qDebug() << "Transaction rolled back";
}
```

## 6. Query Parameter Binding (Prevents SQL Injection)

### 6.1 Positional Binding
```cpp
query.prepare("INSERT INTO CAPTURES (ID_CAPTURE, TYPE_POISSON) VALUES (?, ?)");
query.addBindValue(idCapture);
query.addBindValue(typePoisson);
```

### 6.2 Named Binding (Recommended)
```cpp
query.prepare("INSERT INTO CAPTURES (\"ID_CAPTURE\", \"TYPE_POISSON\") "
              "VALUES (:id, :type)");
query.bindValue(":id", idCapture);
query.bindValue(":type", typePoisson);
```

### 6.3 Null Value Binding
```cpp
// For optional columns (ID_BATEAU can be NULL)
if (idBateau <= 0) {
    query.bindValue(":idBateau", QVariant());  // NULL
} else {
    query.bindValue(":idBateau", idBateau);
}
```

## 7. Error Handling Examples

### 7.1 Check if Query Executed Successfully
```cpp
if (!query.exec()) {
    QString errorText = query.lastError().text();
    QString queryText = query.lastQuery();
    qDebug() << "Error:" << errorText;
    qDebug() << "Query:" << queryText;
    return false;
}
```

### 7.2 Handle Row Not Found
```cpp
query.prepare("SELECT * FROM CAPTURES WHERE \"ID_CAPTURE\" = :id");
query.bindValue(":id", idCapture);

if (query.exec()) {
    if (query.next()) {
        // Row found - process data
    } else {
        // Row not found
        QMessageBox::warning(this, "Non trouvé", 
                            QString("Capture #%1 n'existe pas").arg(idCapture));
    }
} else {
    // Query error
    QMessageBox::warning(this, "Erreur", query.lastError().text());
}
```

## 8. Debugging Helper Functions

### 8.1 Print Last Error
```cpp
void printDatabaseError(const QSqlQuery &query)
{
    qDebug() << "SQL Error:" << query.lastError().text();
    qDebug() << "Last Query:" << query.lastQuery();
    qDebug() << "Error Type:" << query.lastError().type();
}
```

### 8.2 Verify Connection
```cpp
if (QSqlDatabase::database().isOpen()) {
    qDebug() << "Database connected";
} else {
    qDebug() << "Database NOT connected";
    qDebug() << "Connection error:" << QSqlDatabase::database().lastError().text();
}
```

### 8.3 List All Tables
```cpp
QStringList tables = QSqlDatabase::database().tables();
qDebug() << "Available tables:";
for (const QString &table : tables) {
    qDebug() << "  -" << table;
}
```

## 9. Performance Tips

### 9.1 Use Prepared Statements (Already Done ✅)
- Faster than raw SQL strings
- Prevents SQL injection
- Automatic parameter escaping

### 9.2 Batch Operations
```cpp
// Instead of updating one at a time, batch updates:
QSqlQuery query;
query.prepare("UPDATE CAPTURES SET \"QUANTITE\" = ? WHERE \"ID_CAPTURE\" = ?");

for (const auto &item : capturestoUpdate) {
    query.addBindValue(item.quantity);
    query.addBindValue(item.id);
    if (!query.exec()) {
        qWarning() << "Batch update failed:" << query.lastError().text();
    }
}
QSqlDatabase::database().commit();
```

### 9.3 Use Transactions for Multiple Operations
```cpp
QSqlDatabase db = QSqlDatabase::database();

if (!db.transaction()) {
    qWarning() << "Cannot start transaction";
    return false;
}

// Do multiple operations...
query1.exec();
query2.exec();
query3.exec();

if (someErrorOccurred) {
    db.rollback();  // Undo all changes
} else {
    db.commit();    // Save all changes
}
```

## 10. Oracle-Specific Notes

### 10.1 Column Name Quoting
Always quote column names in Oracle:
```cpp
query.prepare("SELECT \"ID_CAPTURE\", \"TYPE_POISSON\" FROM CAPTURES");
// NOT: query.prepare("SELECT ID_CAPTURE, TYPE_POISSON FROM CAPTURES");
```

### 10.2 Date Format
Oracle expects DATE in ISO format (YYYY-MM-DD):
```cpp
QDate date = ui->dateEdit->date();
query.bindValue(":date", QVariant(date));  // Qt handles conversion
```

### 10.3 NULL Values
```cpp
// Insert NULL (not empty string)
query.bindValue(":optional", QVariant());  // This is NULL
// NOT: query.bindValue(":optional", "");  // This is empty string
```

## Summary

| Operation | Query Type | CRUD | Confirmation | Table Refresh |
|-----------|-----------|------|--------------|----------------|
| Load Table | SELECT | R | No | Auto |
| Edit Form | - | R | No | No |
| Save Edit | UPDATE | U | By form | Yes |
| Delete Row | DELETE | D | **Yes** | Yes |
| Refresh | SELECT | R | No | Yes |
| Add New | INSERT | C | No | Yes |

All queries use **prepared statements** and **parameter binding** for security and performance.

