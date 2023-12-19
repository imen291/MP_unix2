#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 5

void sendDateTime(int client_socket) {
    time_t currentTime;
    struct tm *localTime;
    char dateTime[100];

    currentTime = time(NULL);
    localTime = localtime(&currentTime);
    strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M:%S", localTime);

    write(client_socket, dateTime, strlen(dateTime));
}

void sendFileList(int client_socket, char *directory) {
    DIR *dir;
    struct dirent *entry;

    dir = opendir(directory);
    if (dir == NULL) {
        char *errorMsg = "Error opening directory.";
        write(client_socket, errorMsg, strlen(errorMsg));
        return;
    }

    char fileList[1024] = "";
    while ((entry = readdir(dir)) != NULL) {
        strcat(fileList, entry->d_name);
        strcat(fileList, "\n");
    }
    closedir(dir);

    write(client_socket, fileList, strlen(fileList));
}

void sendFileContent(int client_socket, char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        char *errorMsg = "Error opening file.";
        write(client_socket, errorMsg, strlen(errorMsg));
        return;
    }

    char buffer[1024] = "";
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        write(client_socket, buffer, bytesRead);
    }
    fclose(file);
}

void sendClientDuration(int client_socket, time_t startTime) {
    time_t currentTime = time(NULL);
    int duration = difftime(currentTime, startTime);
    char durationStr[50];
    snprintf(durationStr, sizeof(durationStr), "Duration: %d seconds\n", duration);

    write(client_socket, durationStr, strlen(durationStr));
}

void handleClient(int client_socket) {
    time_t startTime = time(NULL);

    int choice;
    do {
        read(client_socket, &choice, sizeof(int));

        switch (choice) {
            case 1:
                sendDateTime(client_socket);
                break;
            case 2:
                sendFileList(client_socket, "./");
                break;
            case 3: {
                char filename[256];
                read(client_socket, filename, sizeof(filename));
                sendFileContent(client_socket, filename);
                break;
            }
            case 4:
                sendClientDuration(client_socket, startTime);
                break;
            default:
                break;
        }
    } while (choice != 0);

    close(client_socket);
}

int main() {
    int server_fd, client_socket, opt = 1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        handleClient(client_socket);
    }

    return 0;
}
