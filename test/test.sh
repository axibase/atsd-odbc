#!/usr/bin/env bash

# this test requires running clickhouse server and configured ~/.odbc.ini
# cp -n /usr/share/doc/clickhouse-odbc/examples/odbc.ini ~/.odbc.ini
# apt install unixodbc

function q {
    echo "$*" | isql clickhouse -b -v
}

q "DROP TABLE IF EXISTS test.odbc1;"
q "CREATE TABLE test.odbc1 (ui64 UInt64, string String, date Date, datetime DateTime) ENGINE = Memory;"
q "INSERT INTO test.odbc1 VALUES (1, '2', 3, 4);"
q "SELECT * FROM test.odbc1 WHERE ui64=1;"

q 'SELECT {fn CONVERT(1, SQL_BIGINT)}'
q "SELECT {fn CONVERT(100000, SQL_TINYINT)}"
q "SELECT {fn CONCAT('a', 'b')}"
q 'SELECT CAST({fn TRUNCATE(1.1 + 2.4, 1)} AS INTEGER) AS `yr_date_ok`'

q $'SELECT COUNT({fn ABS(`test`.`odbc1`.`ui64`)}) FROM test.odbc1'

q $'SELECT {fn TIMESTAMPDIFF(SQL_TSI_DAY,CAST(`test`.`odbc1`.`datetime` AS DATE),CAST(`test`.`odbc1`.`date` AS DATE))} AS `Calculation_503558746242125826`, SUM({fn CONVERT(1, SQL_BIGINT)}) AS `sum_Number_of_Records_ok` FROM `test`.`odbc1` WHERE (CAST(`test`.`odbc1`.`datetime` AS DATE) <> {d \'1970-01-01\'}) GROUP BY `Calculation_503558746242125826`'

q $'SELECT COUNT({fn ABS(`test`.`odbc1`.`ui64`)}) AS `TEMP_Calculation_559572257702191122__2716881070__0_`, SUM({fn ABS(`test`.`odbc1`.`ui64`)}) AS `TEMP_Calculation_559572257702191122__3054398615__0_`  FROM test.odbc1;'

q $'SELECT SUM((CASE WHEN (`test`.`odbc1`.`ui64` * `test`.`odbc1`.`ui64`) < 0 THEN NULL ELSE {fn SQRT((`test`.`odbc1`.`ui64` * `test`.`odbc1`.`ui64`))} END)) AS `TEMP_Calculation_559572257701634065__1464080195__0_`, COUNT((CASE WHEN (`test`.`odbc1`.`ui64` * `test`.`odbc1`.`ui64`) < 0 THEN NULL ELSE {fn SQRT((`test`.`odbc1`.`ui64` * `test`.`odbc1`.`ui64`))} END)) AS `TEMP_Calculation_559572257701634065__2225718044__0_` FROM test.odbc1;'

#SELECT (CASE WHEN (NOT = 'True') OR (`test`.`odbc1`.`string` = 'True') OR (`test`.`odbc1`.`string2` = 'True') THEN 1 WHEN NOT (NOT = 'True') OR (`test`.`odbc1`.`string` = 'True') OR (`test`.`odbc1`.`string` = 'True') OR (`test`.`odbc1`.`string2` = 'True') THEN 0 ELSE NULL END) AS `Calculation_597289912116125696`,
#SUM({fn CONVERT(1, SQL_BIGINT)}) AS `sum_Number_of_Records_ok` FROM `test`.`odbc1` GROUP BY `Calculation_597289912116125696`, `string`, `ui64`