#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sqlite3.h>
#include <openssl/sha.h>


void sha256Hash(const char* input, char* outputBuffer) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, strlen(input));
    SHA256_Final(hash, &sha256);
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = '\0';
}


void createUsersTable(sqlite3* db) {
    const char* sql = "CREATE TABLE IF NOT EXISTS Users(UserID TEXT PRIMARY KEY, PasswordHash TEXT);";
    char* err_msg = 0;
    sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (err_msg) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}


int authenticateUser(sqlite3* db, const char* userID, const char* passwordHash) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT PasswordHash FROM Users WHERE UserID = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_text(stmt, 1, userID, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* storedHash = sqlite3_column_text(stmt, 0);
        if (strcmp(passwordHash, (const char*)storedHash) == 0) {
            sqlite3_finalize(stmt);
            return 1; 
        }
    }
    sqlite3_finalize(stmt);
    return 0; 
}


int registerUser(sqlite3* db, const char* userID, const char* passwordHash) {
    sqlite3_stmt* stmt;
    
    const char* sql_check = "SELECT UserID FROM Users WHERE UserID = ?;";
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, userID, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return 2; 
    }
    sqlite3_finalize(stmt);

    
    const char* sql_insert = "INSERT INTO Users (UserID, PasswordHash) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "SQL prepare error: %s\n", sqlite3_errmsg(db));
        return -1; 
    }
    sqlite3_bind_text(stmt, 1, userID, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, passwordHash, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "SQL insert error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1; 
    }

    sqlite3_finalize(stmt);
    return 1; 
}


int main()
{
    int scket, cAccept;
    struct sockaddr_in sockaddr, clientt;
    socklen_t clientlen = sizeof(clientt);
    char clientMsg[1024];
    const int PORT = 4500;
    sqlite3* db;
    int opt = 1;

    
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    createUsersTable(db);
    
    
    scket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == scket) {
        perror("Socket creation failed");
        return 1;
    }
    
    
    if (setsockopt(scket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_addr.s_addr = INADDR_ANY; 
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(PORT);

    if (bind(scket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1) {
        perror("Bind failed");
        close(scket);
        return 1;
    }

    
    if (listen(scket, 5) == -1) {
        perror("Listen failed");
        close(scket);
        return 1;
    }
    
    printf("Sunucu baslatildi. Port %d dinleniyor...\n", PORT);

    
    while (1) {
        printf("\nYeni istemci baglantisi bekleniyor...\n");
        cAccept = accept(scket, (struct sockaddr*)&clientt, &clientlen);
        if (-1 == cAccept) {
            perror("Accept failed");
            continue; 
        }
        
        char *client_ip = inet_ntoa(clientt.sin_addr);
        printf("Istemci baglandi: %s\n", client_ip);

        
        memset(clientMsg, 0, sizeof(clientMsg));
        int recieve_len = recv(cAccept, clientMsg, sizeof(clientMsg) - 1, 0);

        if (recieve_len > 0) {
            printf("Mesaj alindi: %s\n", clientMsg);

            char command[10], userID[128], passwordHash[SHA256_DIGEST_LENGTH*2 + 1];
            
            
            if (strncmp(clientMsg, "REGISTER", 8) == 0) {
                sscanf(clientMsg, "%s %s %s", command, userID, passwordHash);
                int result = registerUser(db, userID, passwordHash);
                if (result == 1) {
                    send(cAccept, "REGISTER_SUCCESS", strlen("REGISTER_SUCCESS"), 0);
                    printf("Yeni kullanici eklendi: %s\n", userID);
                } else if (result == 2) {
                    send(cAccept, "REGISTER_FAILED:USER_EXISTS", strlen("REGISTER_FAILED:USER_EXISTS"), 0);
                    printf("Kayit basarisiz, kullanici zaten var: %s\n", userID);
                } else {
                    send(cAccept, "REGISTER_FAILED:SERVER_ERROR", strlen("REGISTER_FAILED:SERVER_ERROR"), 0);
                    printf("Kayit sirasinda sunucu hatasi.\n");
                }
            } else { 
                sscanf(clientMsg, "%s %s", userID, passwordHash);
                if (authenticateUser(db, userID, passwordHash)) {
                    send(cAccept, "LOGIN_SUCCESS", strlen("LOGIN_SUCCESS"), 0);
                    printf("Giris basarili. LOGIN_SUCCESS gonderildi: %s\n", userID);
                } else {
                    send(cAccept, "LOGIN_FAILED", strlen("LOGIN_FAILED"), 0);
                    printf("Giris basarisiz. LOGIN_FAILED gonderildi: %s\n", userID);
                }
            }
        } else {
            printf("Istemci baglantiyi kapatti veya bir hata olustu.\n");
        }
        
        
        close(cAccept);
    }
    
    
    close(scket);
    sqlite3_close(db);
    return 0;
}