#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <SPI.h>
#include <FS.h>
#include "SD_MMC.h"
#include <SimpleDHT.h> 
#include <TimeLib.h>
#include "DHT.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <ArduinoJson.h>
#include <map>
#include <AES.h> 
#include <SHA256.h>

const char *ssid = "Wifi名稱";
const char *password = "Wifi密碼";
unsigned long previousMillis = 0;  // 儲存上次執行的時間
const long interval = 1000;  // 設定更新間隔為1秒（1000毫秒）
float temperature = 0;
float humidity = 0;
String startDate;
String startTime;
String endDate;
String endTime;
bool loggedIn = false; // 用于跟踪用户是否已登录
std::map<String, int> loginAttempts; // 使用者名稱和登入嘗試次數的映射
const char* host = "api.thingspeak.com"; // This should not be changed
const int httpPort = 80; // This should not be changed

const char* loginFormHtml = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css">
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/chart.js/dist/Chart.min.css">
  <script src="https://code.jquery.com/jquery-3.5.1.min.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js"></script>
</head>
<body>

<div class="container mt-4">
  <h1>Login</h1>

  <!-- 登录表单 -->
  <form id="loginForm">
    <div class="form-group">
      <label for="username">Username:</label>
      <input type="text" id="username" class="form-control">
    </div>

    <div class="form-group">
      <label for="password">Password:</label>
      <input type="password" id="password" class="form-control">
    </div>

    <button id="loginButton" class="btn btn-primary">Login</button>
  </form>

  <!-- 显示登录失败消息 -->
  <div id="loginErrorMessage" class="alert alert-danger mt-3" style="display: none;"></div>
</div>

<script>
  $(document).ready(function() {
    
    $('#loginButton').click(function(event){
      event.preventDefault();
      var username = $('#username').val();
      var password = $('#password').val();

      // 使用 AJAX 发送用户名和密码到后端
        $.ajax({
          type: 'POST', // 使用POST方法发送数据
          data: { username: username, password: password },
          success: function(response) {
            // 登录成功后的操作
            window.location.href = "/"; // 跳转到主页面
          },
          error: function(xhr) {
           if (xhr.status === 401) {
            var errorMessage = xhr.responseText;
            $('#loginErrorMessage').text(errorMessage);
            $('#loginErrorMessage').show();
            }
          }
        });
    });
    
  });
</script>

</body>
</html>
)=====";


const char* htmlContent = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css">
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/chart.js/dist/Chart.min.css">
  <script src="https://code.jquery.com/jquery-3.5.1.min.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js"></script>
</head>
<body>

  <script src="https://code.jquery.com/jquery-3.5.1.min.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/js/bootstrap.min.js"></script>
  
  <div class="container mt-4">
    <h1>SQLite Server</h1>
    
    <div class="form-group">
      <label for="startDate">Start Date:</label>
      <input type="date" id="startDate" class="form-control">
    </div>
    
    <div class="form-group">
      <label for="startTime">Start Time:</label>
      <input type="time" id="startTime" class="form-control">
    </div>
    
    <div class="form-group">
      <label for="endDate">End Date:</label>
      <input type="date" id="endDate" class="form-control">
    </div>
    
    <div class="form-group">
      <label for="endTime">End Time:</label>
      <input type="time" id="endTime" class="form-control">
    </div>
    <button id="filterButton" class="btn btn-primary">Filter Data</button>
    <button id="initializeButton" class="btn btn-danger">Initialize Database</button>
    <button id="allDeleteButton" class="btn btn-danger">ALL Delete</button>
    
    <table id="dataTable" class="table mt-4">
      <thead>
        <tr>
          <th>Temperature</th>
          <th>Humidity</th>
          <th>Date</th>
          <th>Time</th>
          <th>Action</th>
        </tr>
      </thead>
      <tbody id="dataBody"></tbody>
    </table>

    <canvas id="temperatureChartCanvas" width="400" height="200"></canvas>
    
    <script>
      
      $(document).ready(function() {
        var chartInstance;
        function fetchData() {
            var startDate = $('#startDate').val();
            var startTime = $('#startTime').val();
            var endDate = $('#endDate').val();
            var endTime = $('#endTime').val();
            
            $.get('/data', { start_date: startDate, start_time: startTime, end_date: endDate, end_time: endTime }, function(data) {
              $('#dataBody').empty();
              
              data.forEach(function(record) {
                var row = '<tr>';
                row += '<td>' + record.temperature + '</td>';
                row += '<td>' + record.humidity + '</td>';
                row += '<td>' + record.date + '</td>';
                row += '<td>' + record.time + '</td>';
                row += '<td><button class="btn btn-danger deleteButton" data-id="' + record.id + '">Delete</button></td>';
                row += '</tr>';
                
                $('#dataBody').append(row);
              });
        
              drawLineChart(data);
            });
        }
        fetchData();
        function drawLineChart(data) {
            if (data.length === 0) {
                // 如果資料陣列為空，不執行繪圖操作
                return;
            }
            var ctx = document.getElementById('temperatureChartCanvas').getContext('2d');
            if (typeof chartInstance !== 'undefined') {
                chartInstance.destroy(); // 摧毀之前的圖表實例
            }
            
            var temperatureData = data.map(function(record) {
                var dateTimeStr = record.date + ' ' + record.time; // 將日期時間字符串合併
                return { x: dateTimeStr, y: record.temperature };
            });
            
            var humidityData = data.map(function(record) {
                var dateTimeStr = record.date + ' ' + record.time; // 將日期時間字符串合併
                return { x: dateTimeStr, y: record.humidity };

            });
            
            chartInstance = new Chart(ctx, {
              type: 'line',
              data: {
                datasets: [
                  { label: 'Temperature', data: temperatureData, borderColor: 'red', fill: false },
                  { label: 'Humidity', data: humidityData, borderColor: 'blue', fill: false }
                ]
              },
              options: {
                scales: {
                    x: {
                      type: 'category', // 保留日期類型
                      title: {
                        display: true,
                        text: 'Time'
                      }
                    },
                    y: {
                      beginAtZero: true,
                      title: {
                        display: true,
                        text: 'Value'
                      }
                    }
                }}});
        }
        
        $('#filterButton').click(fetchData);
        
        $('#initializeButton').click(function() {

            if (confirm("確定要初始化資料庫嗎？這將刪除所有資料！")) {
              $.post('/initialize', function(data) {
                // 清空資料表後，刷新整個頁面以更新網頁上的資料
                location.reload();
              });
            }
        });
        $('#allDeleteButton').click(function() {
            // 刪除篩選出來的所有資料
              if (confirm("確定刪除所有資料嗎?")){
                $.post('/alldelete', function(data) {
                  fetchData(); // 更新顯示的資料
                });
              }
         });
        $(document).on('click', '.deleteButton', function() {
          var id = $(this).data('id');
            $.post('/delete', { id: id }, function(data) {
                fetchData();
              });
        });
      });
    </script>
  </div>
</body>
</html>)=====";

int pinDHT22 = 18;
SimpleDHT22 dht22(pinDHT22);

float initTemp = 0;
sqlite3 *db;

WiFiUDP ntpUDP; // NTP 是基於UDP協議的，所以要使用UDP來傳輸
NTPClient timeClient(ntpUDP, "time.stdtime.gov.tw", 28800, 60000); // 時間同步的設定，第一個參數是UDP，第二個是NTP伺服器，第三個是時區，第四個是更新時間間隔
WebServer server(80); //建立一個WebServer物件，並指定port為80,也就是http的預設port

AESTiny128 aes;
byte aesKey[] = {
  0x4E, 0x9A, 0xD7, 0x3F, 0x7B, 0x8C, 0x21, 0xE6,
  0x5D, 0x0F, 0xA8, 0x6E, 0x33, 0xC2, 0x1B, 0xF4
};

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

String byteArrayToHexString(byte *array, int length) {
    String hexString = "";
    for (int i = 0; i < length; i++) {
        if (array[i] < 16) {
            hexString += "0"; // 确保输出的十六进制数是两位
        }
        hexString += String(array[i], HEX);
    }
    return hexString;
}
void hexStringToByteArray(const char *hexString, byte *array, int length) {
    for (int i = 0; i < length; i++) {
        sscanf(hexString + 2 * i, "%2hhx", &array[i]);
    }
}
void insertDataToDb(float H,float T){
    unsigned long epochTime = timeClient.getEpochTime(); // 取得時間戳記
    
    // 使用 time() 函數來解析時間戳記，獲取日期和時間
    time_t time = epochTime;
    struct tm *timeinfo;
    timeinfo = localtime(&time);

    // 取得日期
    char dayStamp[11]; // 11 字符，包括結束的 '\0'
    sprintf(dayStamp, "%04d-%02d-%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);

    // 取得時間
    char timeStamp[9]; // 9 字符，包括結束的 '\0'
    sprintf(timeStamp, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    // 輸出日期和時間
    Serial.println(dayStamp);
    Serial.println("------------");
    Serial.println(timeStamp);
    Serial.println("------------");

    //加密
    aes.setKey(aesKey, sizeof(aesKey)); // 设置AES密钥
    byte encryptedTemp[16];
    byte encryptedHumidity[16];
    aes.encryptBlock(encryptedTemp, (const byte *)&T);
    aes.encryptBlock(encryptedHumidity, (const byte *)&H);

    String encryptedTempHex = byteArrayToHexString(encryptedTemp, sizeof(encryptedTemp));
    String encryptedHumidityHex = byteArrayToHexString(encryptedHumidity, sizeof(encryptedHumidity));
    int rc;
    String sql = "INSERT INTO sensor_data (temp, humidity, date, time) VALUES ('" + encryptedTempHex + "', '" + encryptedHumidityHex  + "', '" + dayStamp + "', '" + timeStamp + "');";
    Serial.println(sql);
    //sprintf(sql, "INSERT INTO test1 (time,temp,hum) VALUES (%Lu,%.1f,%.1f);", time, temp, hum);
    const char* sqlQuery = sql.c_str();
    rc = db_exec(db, sqlQuery);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db);
        return;
    }
}

bool checkCredentials(const String& username, const String& password) {
  // 打开数据库
  if (openDb("/sdcard/temptest.db", &db) != SQLITE_OK) {
     Serial.println("Failed to open db!");
     sqlite3_close(db);
     return false;
  }
  int rc;
  String salt;
  String hashedPasswordFromDB;
  String userStatus;
  char *sql = "CREATE TABLE IF NOT EXISTS user_table (id INTEGER PRIMARY KEY AUTOINCREMENT,username TEXT NOT NULL,password TEXT NOT NULL,salt TEXT NOT NULL,status TEXT NOT NULL DEFAULT 'pending')";
  rc = db_exec(db, sql);
    if (rc != SQLITE_OK) {
        Serial.println("Failed to create table!");
        sqlite3_close(db);  // 關閉資料庫
    }
    else{
        Serial.println("成功建立user_table!");
  }
  
  // 创建并执行 SQL 查询语句
  const char* query = "SELECT salt, password, status FROM user_table WHERE username = ?";
  sqlite3_stmt* stmt;
  rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    Serial.println("SQL error: Failed to prepare statement");
    sqlite3_close(db);
    return false;
  }
  rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    Serial.println("SQL error: Failed to bind username parameter");
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return false;
  }
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    hashedPasswordFromDB = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    userStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
  }else{
    Serial.println("User not found");    
    sqlite3_finalize(stmt); // 释放资源
    sqlite3_close(db); // 关闭数据库连接
    return false;
  }
  
  sqlite3_finalize(stmt);
  sqlite3_close(db);
  
  if (userStatus != "approved") {
      Serial.println("User not approved");
      return false;
  }
  
  if (hashedPasswordFromDB.isEmpty()) {
    Serial.println("User not found");
    return false;
  }
  else{
    Serial.println(salt);
    Serial.println(hashedPasswordFromDB);
  }

  Serial.println(password);
  // 计算 salt + password 的 SHA256 哈希
  SHA256 sha256;
  sha256.reset();
  String saltedPassword = password + salt;
  uint8_t hash[32];
  sha256.update((const uint8_t*)saltedPassword.c_str(), saltedPassword.length());
  sha256.finalize(hash,32);

  // 将哈希转换为字符串形式
  String hashedPassword;
  for (size_t i = 0; i < 32; i++) {
    hashedPassword += String(hash[i], HEX);
  }
  Serial.println(hashedPassword);
  // 检查计算得到的哈希与数据库中存储的哈希是否匹配
  bool loginSuccess = (hashedPassword == hashedPasswordFromDB);

  return loginSuccess;
}
void restrictUser(const String& username){
   // 打开数据库并执行更新操作
  if (openDb("/sdcard/temptest.db", &db) != SQLITE_OK) {
     Serial.println("Failed to open db!");
     sqlite3_close(db);
  }
  Serial.println("restrictUser");
  // 使用預編譯的 SQLite 語句
  const char* query = "UPDATE user_table SET status = 'pending' WHERE username = ?";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    Serial.println("SQL error: Failed to prepare statement");
    sqlite3_close(db);
    return;
  }
  
  // 綁定參數並執行更新操作
  rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
  if (rc != SQLITE_OK) {
    Serial.println("SQL error: Failed to bind username parameter");
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return;
  }
  
  rc = sqlite3_step(stmt);
  if (rc != SQLITE_DONE) {
    Serial.println("SQL error: Failed to update user status");
    sqlite3_finalize(stmt);
    sqlite3_close(db);
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);

}
void handleRoot() {
  if (server.method() == HTTP_POST) {
    String username = server.arg("username");
    String password = server.arg("password");
    
    Serial.println(username);
    Serial.println(password);
    
    if (checkCredentials(username, password)) {
      loggedIn = true;
      loginAttempts[username] = 0;
      server.sendHeader("Location", "/");
      server.send(302, "text/plain", ""); // 重定向到根路径
      return;
    } 
    else {
     if (loginAttempts.find(username) != loginAttempts.end()) {
       loginAttempts[username]+=1;
     } else {
        loginAttempts[username] = 1;
     }
     if (loginAttempts[username] > 3) {
        restrictUser(username);
        server.send(401, "text/html", "Too many login attempts, account locked."); // 用户被锁定的消息
      } else{ 
         server.send(401, "text/html", "Incorrect username or password. Attempts left: " + String(3 - loginAttempts[username])); // 错误消息，显示剩余尝试次数
      }
      Serial.print(username);
      Serial.print(loginAttempts[username]);
    }
  }
  if (loggedIn) {
    // 已登录，显示页面内容
    server.send(200, "text/html", htmlContent);
  } else {
    // 未登录，显示登录表单
    server.send(200, "text/html", loginFormHtml);
  }
  
}
void handleDataRequest() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  Serial.println("Get Request");
  
  aes.setKey(aesKey, sizeof(aesKey));
  
    // 獲取 URL 解碼後的參數值
  startDate = server.urlDecode(server.arg("start_date"));
  startTime = server.urlDecode(server.arg("start_time"));
  endDate = server.urlDecode(server.arg("end_date"));
  endTime = server.urlDecode(server.arg("end_time"));

  // 創建 JSON 物件以存儲數據
  DynamicJsonDocument jsonDoc(2048);
  JsonArray dataArray = jsonDoc.to<JsonArray>();

  String sql = "SELECT * FROM sensor_data WHERE (strftime('%Y-%m-%d', date) || ' ' || time) >= '" + startDate + " " + startTime + "' AND (strftime('%Y-%m-%d', date) || ' ' || time) <= '" + endDate + " " + endTime + "';";
  const char* sqlQuery = sql.c_str();
  Serial.println(sqlQuery);
  
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare_v2(db, sqlQuery, -1, &stmt, NULL);
  if (rc == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      JsonObject data = dataArray.createNestedObject();
      data["date"] = server.urlDecode((const char*)sqlite3_column_text(stmt, 3)); // 儲存日期
      data["time"] = server.urlDecode((const char*)sqlite3_column_text(stmt, 4)); // 儲存時間
      
      byte encryptedTemp[16];
      byte encryptedHumidity[16];
      const char* tempHex = (const char*)sqlite3_column_text(stmt, 1);
      const char* humidityHex = (const char*)sqlite3_column_text(stmt, 2);
      hexStringToByteArray(tempHex, encryptedTemp, 16);
      hexStringToByteArray(humidityHex, encryptedHumidity, 16);
      
      byte decryptedTemp[16];
      byte decryptedHumidity[16];
      aes.decryptBlock(decryptedTemp, encryptedTemp);
      aes.decryptBlock(decryptedHumidity, encryptedHumidity);
      float temp, humidity;
      memcpy(&temp, decryptedTemp, sizeof(float));
      memcpy(&humidity, decryptedHumidity, sizeof(float));
      
//      temp = roundf(temp * 100) / 100;
//      humidity = roundf(humidity * 100) / 100;
//      sprintf(tempStr, "%.2f", temp);
//      sprintf(humidityStr, "%.2f", humidity);
      
      data["temperature"] = String(temp);
      data["humidity"] = String(humidity);
      data["id"] = sqlite3_column_int(stmt, 0);

    }
    sqlite3_finalize(stmt);
  }
  else{
      Serial.println("SQL error: Failed to prepare statement");
  }
  
  // 將 JSON 資料轉換成字串
  String jsonData;
  serializeJson(jsonDoc, jsonData);
  // 回傳 JSON 資料給前端網頁
  server.send(200, "application/json", jsonData);
  
}
void handleDeleteRequest() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  String idToDelete = server.arg("id"); // 從 POST 請求中獲取要刪除的資料的 ID

  // 建立刪除資料的 SQL 查詢
  String sql = "DELETE FROM sensor_data WHERE ID = " + idToDelete;

  // 執行 SQL 查詢
  int rc = db_exec(db, sql.c_str());
  if (rc != SQLITE_OK) {
    server.send(500, "text/plain", "Failed to delete data");
    return;
  }
  // 成功刪除資料
  server.send(200, "text/plain", "Data deleted successfully");
}
void handleAllDeleteRequest() {
  Serial.println("ALL Delete Request");
  
  // 執行 SQL 命令來刪除所有篩選出來的資料
  String sql = "DELETE FROM sensor_data WHERE (strftime('%Y-%m-%d', date) || ' ' || time) >= '" + startDate + " " + startTime + "' AND (strftime('%Y-%m-%d', date) || ' ' || time) <= '" + endDate + " " + endTime + "';";
  int rc = db_exec(db, sql.c_str());
  if (rc != SQLITE_OK) {
    Serial.println("SQL error: Failed to delete records");
    server.send(500, "text/plain", "Failed to delete records");
    return;
  }
  
  Serial.println("Records deleted");
  server.send(200, "text/plain", "Records deleted");
}
void handleInitializeRequest() {
  Serial.println("Initialize Request");
  
  // 執行 SQL 命令來清空資料表
  const char* sql = "DELETE FROM sensor_data;";
  int rc = db_exec(db, sql);
  if (rc != SQLITE_OK) {
    Serial.println("SQL error: Failed to delete records");
    server.send(500, "text/plain", "Failed to initialize database");
    return;
  }
  
  Serial.println("Database initialized");
  server.send(200, "text/plain", "Database initialized");
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    timeClient.begin();

    

    char *zErrMsg = 0;
    int rc;
   //SPI.begin(); 
   //SD_MMC.begin();
    setTime(0, 0, 0, 1, 1, 2023); // 設定初始時間，可以根據需要修改
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

    // Open database 1
//    if (openDb("/sdcard/temptest.db", &db))
//       return;

    // 初始化資料庫並創建資料表
    if (openDb("/sdcard/temptest.db", &db) != SQLITE_OK) {
      return;
    }
    // create table
    char *sql = "CREATE TABLE IF NOT EXISTS sensor_data (ID INTEGER PRIMARY KEY AUTOINCREMENT, temp REAL NOT NULL, humidity REAL NOT NULL, date TEXT NOT NULL, time TEXT NOT NULL)";
    rc = db_exec(db, sql);
    if (rc != SQLITE_OK) {
        Serial.println("Failed to create table!");
        sqlite3_close(db);  // 關閉資料庫
        return;
    }
    server.on("/", handleRoot); // 設定根路徑的處理器
    server.on("/data", handleDataRequest);
    server.on("/delete", HTTP_POST, handleDeleteRequest);
    server.on("/initialize", HTTP_POST, handleInitializeRequest);
    server.on("/alldelete", HTTP_POST, handleAllDeleteRequest);
    // 啟動Web伺服器
    server.begin();
    
}

void loop() {
    server.handleClient(); // 處理Web請求
    
    timeClient.update();
    //監測溫度並寫入database table 中
//    Serial.println("=================================");
//    Serial.println("Start DHT22...");
    unsigned long currentMillis = millis();  // 獲取當前時間
    if (currentMillis - previousMillis >= interval) { 
      int err = SimpleDHTErrSuccess; 
      if ((err = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
        Serial.print("Read DHT22 failed, err="); Serial.print(SimpleDHTErrCode(err));
        Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(2000);
        return;
      }
      previousMillis = currentMillis;
    }
    if(abs(temperature - initTemp) >= 0.2){
          insertDataToDb(humidity,temperature);
          initTemp = temperature;
      }

}
