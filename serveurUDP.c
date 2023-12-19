#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define NMAX 100

GtkWidget *output_label;

void start_server(GtkWidget *widget, gpointer data) {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(gtk_entry_get_text(GTK_ENTRY(data))));

    bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    char output_text[1000] = "Serveur en attente...\n";
    gtk_label_set_text(GTK_LABEL(output_label), output_text);

    while (1) {
        int n;
        unsigned int len = sizeof(cliaddr);
        recvfrom(sockfd, &n, sizeof(n), 0, (struct sockaddr *)&cliaddr, &len);

        char client_message[100];
        sprintf(client_message, "Client a envoyé : %d\n", n);

        for (int i = 0; i < n; i++) {
            int random_num = rand() % NMAX + 1;
            sendto(sockfd, &random_num, sizeof(random_num), 0, (const struct sockaddr *)&cliaddr, len);
        }

        strcat(output_text, client_message);
        gtk_label_set_text(GTK_LABEL(output_label), output_text);
    }

    close(sockfd);
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *button;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Serveur GUI");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    GtkWidget *port_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), port_entry, 0, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Port:"), 1, 0, 1, 1);

    output_label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), output_label, 0, 1, 2, 1);

    button = gtk_button_new_with_label("Start Server");
    g_signal_connect(button, "clicked", G_CALLBACK(start_server), port_entry);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 2, 1);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

