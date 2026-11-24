#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define SERVER_PORT 8080
#define CLIENT_PORT 5000

// CLIENT FUNCTION
void *client_thread(void *arg)
{
    int sock;
    struct sockaddr_in client_addr, server_addr;
    char buffer[1024];

    // 1. Create client socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1)
    {
        perror("Client socket creation failed");
        return NULL;
    }

    printf("\nClient socket created successfully.\n");

    // 2. Configure client address
    memset(&client_addr, 0, sizeof(client_addr));

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);
    client_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // 3. Bind client address to client socket
    if (bind(sock, (struct sockaddr *)&client_addr,
             sizeof(client_addr)) == -1)
    {
        perror("Client bind failed");
        close(sock);
        return NULL;
    }

    printf("Client bound to port %d.\n", CLIENT_PORT);

    // 4. Configure server address
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // 5. Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1)
    {
        perror("Connection failed");
        close(sock);
        return NULL;
    }

    printf("Client connected to server successfully!\n");

    // 6. Send message to server
    char *message = "Hello Server! This is TCP Client.";

    send(sock, message, strlen(message), 0);

    printf("Message sent to server.\n");

    // 7. Receive response from server
    int n = recv(sock, buffer, sizeof(buffer) - 1, 0);

    if (n > 0)
    {
        buffer[n] = '\0';
        printf("Server says: %s\n", buffer);
    }

    // 8. Close client socket
    close(sock);

    printf("Client socket closed.\n");

    return NULL;
}


// SERVER FUNCTION
int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    char buffer[1024];
    pthread_t client_id;

    // 1. Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("Server socket creation failed");
        return 1;
    }

    printf("Server socket created successfully.\n");

    // 2. Configure server address
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // 3. Bind server address to server socket
    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1)
    {
        perror("Server bind failed");
        close(server_fd);
        return 1;
    }

    printf("Server bound to port %d.\n", SERVER_PORT);

    // 4. Listen for client connections
    if (listen(server_fd, 5) == -1)
    {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("Server is listening...\n");

    // 5. Start client thread
    if (pthread_create(&client_id, NULL, client_thread, NULL) != 0)
    {
        perror("Thread creation failed");
        close(server_fd);
        return 1;
    }

    // 6. Accept client connection
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1)
    {
        perror("Accept failed");
        pthread_join(client_id, NULL);
        close(server_fd);
        return 1;
    }

    printf("\nServer accepted client connection!\n");

    // 7. Receive message from client
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (n > 0)
    {
        buffer[n] = '\0';

        printf("Client says: %s\n", buffer);

        // 8. Send response to client
        char *reply = "Hello Client! Message received.";

        send(client_fd, reply, strlen(reply), 0);

        printf("Response sent to client.\n");
    }

    // 9. Close accepted client socket
    close(client_fd);

    // 10. Wait for client thread to finish
    pthread_join(client_id, NULL);

    // 11. Close server socket
    close(server_fd);

    printf("\nServer closed successfully.\n");

    return 0;
}
