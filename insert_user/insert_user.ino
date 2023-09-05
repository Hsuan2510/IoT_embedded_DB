#include <Crypto.h>
#include <SHA256.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <SPI.h>
#include <FS.h>
#include "SD_MMC.h"

#define SHA256_SIZE 32
sqlite3 *db;

const char* data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   Serial.printf("%s: ", (const char*)data);
   for (i = 0; i<argc; i++){
       Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   Serial.printf("\n");
   return 0;
}

int openDb(const char *filename, sqlite3 **db) {
   int rc = sqlite3_open(filename, db);
   if (rc) {
       Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
       return rc;
   } else {
       sqlite3_exec(*db, "PRAGMA encoding = \"UTF-8\";", NULL, NULL, NULL);
       Serial.printf("Opened database successfully\n");
   }
   return rc;
}
char *zErrMsg = 0;
int db_exec(sqlite3 *db, const char *sql) {
   Serial.println(sql);
   long start = micros();
   int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if (rc != SQLITE_OK) {
       Serial.printf("SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
   } else {
       Serial.printf("Operation done successfully\n");
   }
   Serial.print(F("Time taken:"));
   Serial.println(micros()-start);
   return rc;
}

String generateRandomSalt(int length) {
    String salt;
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    
    for (int i = 0; i < length; ++i) {
        salt += charset[random(strlen(charset))];
    }
    
    return salt;
}

String calculatePasswordHash(const String &password, const String &salt) {
    SHA256 sha256;
    String passwordSalt = password + salt;
    uint8_t passwordHash[SHA256_SIZE];
    
    sha256.update(passwordSalt.c_str(), passwordSalt.length());
    sha256.finalize(passwordHash,32);
    
    String passwordHashHex;
    
    for (size_t i = 0; i < SHA256_SIZE; i++) {
        passwordHashHex += String(passwordHash[i], HEX);
    }
    
    return passwordHashHex;
}

void insertDataToUserTable(const char* usr, const char* pwd, const char* slt, const char* data_) {
    // 初始化資料庫連接，與您之前的程式碼相同
    String username = usr;   // 修改变量名，避免与参数名冲突
    String password = pwd;
    String salt = slt;
    String status_ = data_;
    String sql = "INSERT INTO user_table (username, password, salt, status) VALUES ('" + String(username) + "', '" + String(password) + "', '" + String(salt) + "', '" + String(status_) + "')";
    Serial.println(sql);
    const char* sqlQuery = sql.c_str();
    int rc = db_exec(db, sqlQuery);
    if (rc != SQLITE_OK) {
        Serial.println("Failed to insert data!");
        sqlite3_close(db);
        return;
    } else {
        Serial.println("Data inserted successfully!");
        return;
    }
    
}
void setup() {
    Serial.begin(115200);

    char *zErrMsg = 0;
    int rc;
   //SPI.begin(); 
   //SD_MMC.begin();
    SD_MMC.setPins(14,15,2,4,12,13);  
    pinMode(2, INPUT_PULLUP);  //D0
    pinMode(4, INPUT_PULLUP);  //D1
    pinMode(12, INPUT_PULLUP); //D2
    pinMode(13, INPUT_PULLUP); //D3
    pinMode(14, INPUT_PULLUP); //CLK
    pinMode(15, INPUT_PULLUP); //CMD
    if(!SD_MMC.begin("/sdcard",false,false,20000,5)){
        Serial.println("Card Mount Failed");
        return;
    }

    sqlite3_initialize();


    // 初始化資料庫並創建資料表
    if (openDb("/sdcard/temptest.db", &db) != SQLITE_OK) {
      return;
    }
    // create table
    
    char *sql = "CREATE TABLE IF NOT EXISTS user_table (id INTEGER PRIMARY KEY AUTOINCREMENT,username TEXT NOT NULL,password TEXT NOT NULL,salt TEXT NOT NULL,status TEXT NOT NULL DEFAULT 'pending')";
    rc = db_exec(db, sql);
    if (rc != SQLITE_OK) {
        Serial.println("Failed to create table!");
        sqlite3_close(db);  // 關閉資料庫
        return;
    }
    else{
        Serial.println("成功建立user_table!");
    }
    //sqlite3_close(db);
    
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Enter username:");
  while (!Serial.available()) {
      // 等待使用者輸入
  }
  String username = Serial.readStringUntil('\n');
  Serial.println("Enter password:");
  while (!Serial.available()) {
        // 等待使用者輸入
    }
  String password = Serial.readStringUntil('\n');
  
  String salt = generateRandomSalt(5); // 自己實作生成隨機 salt 的函數
  Serial.println(salt);

  String passwordHash = calculatePasswordHash(password, salt); // 自己實作計算哈希值的函數
  
  insertDataToUserTable(username.c_str(), passwordHash.c_str(), salt.c_str(), "approved"); // 自己實作插入資料的函數
  
}
