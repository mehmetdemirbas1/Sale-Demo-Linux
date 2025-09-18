#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <openssl/sha.h>
#include <termios.h>


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

void getPassword(char* password, int maxLen) {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int i = 0;
    char ch;
    while ((ch = getchar()) != '\n' && i < maxLen - 1) {
        password[i++] = ch;
        printf("*");
    }
    password[i] = '\0';
    printf("\n");
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}


int main()
{
    char userIP[20];
    int portNo = 0;
    char userName[128];
    char password[128];
    char passwordHash[SHA256_DIGEST_LENGTH*2 + 1];
    char serverResponse[1024];

 
    printf("\nSunucu IP adresi: ");
    scanf("%19s", userIP);
    printf("Sunucu Port numarasi: ");
    scanf("%d", &portNo);

    int scket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == scket) {
        perror("Hata: Soket olusturulamadi");
        return 1;
    }

    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(portNo);
    
    if (inet_pton(AF_INET, userIP, &sockaddr.sin_addr) <= 0) {
        fprintf(stderr, "Hata: Gecersiz IP adresi formati.\n");
        close(scket);
        return 1;
    }

    if (sockaddr.sin_addr.s_addr == htonl(INADDR_ANY) || sockaddr.sin_addr.s_addr == htonl(INADDR_BROADCAST)) {
        fprintf(stderr, "Hata: Bu IP adresi (%s) bir sunucu hedefi olarak kullanilamaz.\n", userIP);
        close(scket);
        return 1;
    }

    if (connect(scket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1) {
        perror("Hata: Sunucuya baglanilamadi (IP veya Port yanlis olabilir)");
        close(scket);
        return 1;
    }
    
    printf("Sunucuya baglanti basarili.\n");

    
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    printf("Kullanici Adi: ");
    scanf("%127s", userName);

    while ((c = getchar()) != '\n' && c != EOF);

    printf("Sifre: ");
    getPassword(password, sizeof(password));

    
    sha256Hash(password, passwordHash);

    
    char loginData[256];
    sprintf(loginData, "%s %s", userName, passwordHash);

    if (send(scket, loginData, strlen(loginData), 0) == -1) {
        perror("Hata: Giris bilgileri gonderilemedi");
        close(scket);
        return 1;
    }
    printf("Giris bilgileri gonderildi, sunucu yaniti bekleniyor...\n");

    
    memset(serverResponse, 0, sizeof(serverResponse));
    if (recv(scket, serverResponse, sizeof(serverResponse) - 1, 0) == -1) {
        perror("Hata: Sunucu yaniti alinamadi");
    } else {
        printf("Sunucu yaniti: %s\n", serverResponse);
        
        if (strcmp(serverResponse, "LOGIN_SUCCESS") == 0) {
            printf("Giris basarili! Ana uygulama baslatiliyor...\n");
            close(scket);

            execlp("./saledemo_app", "saledemo_app", (char *)NULL);
            
            perror("Hata: Ana uygulama (saledemo_app) baslatilamadi");
            return 1;
        } else {
            printf("Giris basarisiz. Lutfen bilgilerinizi kontrol edin.\n");
        }
    }
    
    close(scket);
    return 0;
}