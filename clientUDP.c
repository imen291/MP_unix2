#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define NMAX 100

GtkWidget *output_label;

void run_client(GtkWidget *widget, gpointer data) {
    const gchar *server_address = gtk_entry_get_text(GTK_ENTRY(data));
    const gchar *port_text = gtk_entry_get_text(GTK_ENTRY(data));

    int port = atoi(port_text);

    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, server_address, &servaddr.sin_addr);

    int n = rand() % NMAX + 1;

    sendto(sockfd, &n, sizeof(n), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    char result_text[1000];
    sprintf(result_text, "Client a envoyé : %d\nRéception des nombres du serveur :\n", n);

    for (int i = 0; i < n; i++) {
        int received_num;
        recvfrom(sockfd, &received_num, sizeof(received_num), 0, NULL, NULL);
        char num_text[20];
        sprintf(num_text, "%d\n", received_num);
        strcat(result_text, num_text);
    }

    gtk_label_set_text(GTK_LABEL(output_label), result_text);

    close(sockfd);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *button;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Client GUI");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    GtkWidget *server_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), server_entry, 0, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Server Address:"), 1, 0, 1, 1);

    GtkWidget *port_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), port_entry, 2, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Port:"), 3, 0, 1, 1);

    output_label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), output_label, 0, 1, 4, 1);

    button = gtk_button_new_with_label("Run Client");
    g_signal_connect(button, "clicked", G_CALLBACK(run_client), port_entry);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 4, 1);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
