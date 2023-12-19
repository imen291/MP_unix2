



#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

GtkWidget *auth_window, *menu_window, *username_entry, *password_entry, *output_label;
int client_socket;
gboolean authenticated = FALSE;

void authenticate(GtkWidget *widget, gpointer data) {
    const gchar *username = gtk_entry_get_text(GTK_ENTRY(username_entry));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(password_entry));

    // Placez ici le code d'authentification avec le serveur
    // Exemple simple (à remplacer par une authentification réelle)
    if (g_strcmp0(username, "unix") == 0 && g_strcmp0(password, "unix") == 0) {
        gtk_label_set_text(GTK_LABEL(output_label), "Authentification réussie.");
        authenticated = TRUE;

        // Masquer la fenêtre d'authentification
        gtk_widget_hide(auth_window);

        // Afficher la fenêtre du menu
        gtk_widget_show_all(menu_window);
    } else {
        gtk_label_set_text(GTK_LABEL(output_label), "Échec de l'authentification.");
    }
}

void showMenu(GtkWidget *widget, gpointer data) {
    if (authenticated) {
        const char *menuOptions[] = {
            "Services disponibles :",
            "1. Obtenir la date et l'heure",
            "2. Liste des fichiers du répertoire",
            "3. Contenu d'un fichier",
            "4. Durée écoulée depuis la connexion",
            "0. Quitter"
        };

        gchar *menuText = g_strjoinv("\n", (gchar **)menuOptions);
        gtk_label_set_text(GTK_LABEL(output_label), menuText);
        g_free(menuText);

        // Assurez-vous que la fenêtre du menu est visible
        if (!gtk_widget_get_visible(menu_window)) {
            gtk_widget_show_all(menu_window);
        }
    }
}

void handleServerResponse(GtkWidget *widget, gpointer data) {
    if (authenticated) {
        GtkWidget *dialog;
        GtkWidget *content_area;
        GtkWidget *entry_choice;

        // Créer une boîte de dialogue
        dialog = gtk_dialog_new_with_buttons("Choix de l'utilisateur", GTK_WINDOW(menu_window),
                                             GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, NULL);

        // Récupérer la zone de contenu de la boîte de dialogue
        content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        // Ajouter une entrée de texte à la boîte de dialogue
        entry_choice = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entry_choice), "Choix");
        gtk_container_add(GTK_CONTAINER(content_area), entry_choice);

        // Afficher la boîte de dialogue
        gtk_widget_show_all(dialog);

        // Attendre la réponse de l'utilisateur
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
            // Récupérer le choix de l'utilisateur
            const gchar *entry_text = gtk_entry_get_text(GTK_ENTRY(entry_choice));
            int choice = atoi(entry_text);

            // Envoyer le choix au serveur
            write(client_socket, &choice, sizeof(int));

            // Gérer la réponse du serveur
            switch (choice) {
                case 1: {
                     char dateTime[100];
                    read(client_socket, dateTime, sizeof(dateTime));
                    gchar *label_text = g_strdup_printf("Date et heure du serveur : %s", dateTime);
                    gtk_label_set_text(GTK_LABEL(output_label), label_text);
                    g_free(label_text);
                    break;
                }
                case 2: {
                    char fileList[1024];
                    read(client_socket, fileList, sizeof(fileList));
                    printf("Liste des fichiers :\n%s\n", fileList);
                    break;
                }
                case 3: {
                    char filename[256];
                    // Utiliser une boîte de dialogue pour saisir le nom du fichier
                    GtkWidget *file_dialog = gtk_file_chooser_dialog_new("Saisir le nom du fichier", GTK_WINDOW(menu_window),
                                                                          GTK_FILE_CHOOSER_ACTION_OPEN, "OK", GTK_RESPONSE_ACCEPT,
                                                                          "Annuler", GTK_RESPONSE_CANCEL, NULL);

                    if (gtk_dialog_run(GTK_DIALOG(file_dialog)) == GTK_RESPONSE_ACCEPT) {
                        strcpy(filename, gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_dialog)));
                        write(client_socket, filename, sizeof(filename));

                        char fileContent[1024];
                        read(client_socket, fileContent, sizeof(fileContent));
                        printf("Contenu du fichier :\n%s\n", fileContent);
                    }

                    gtk_widget_destroy(file_dialog);
                    break;
                }
                case 4: {
                    char duration[50];
                    read(client_socket, duration, sizeof(duration));
                    printf("Durée depuis la connexion : %s\n", duration);
                    break;
                }
                default:
                    break;
            }
        }

        // Fermer la boîte de dialogue
        gtk_widget_destroy(dialog);
    }
}


int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Créer la fenêtre d'authentification
    auth_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(auth_window), "Authentification");
    gtk_container_set_border_width(GTK_CONTAINER(auth_window), 10);
    gtk_widget_set_size_request(auth_window, 300, 150);

    g_signal_connect(auth_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *auth_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(auth_window), auth_grid);

    GtkWidget *auth_label = gtk_label_new("Nom d'utilisateur:");
    gtk_grid_attach(GTK_GRID(auth_grid), auth_label, 0, 0, 1, 1);

    username_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(auth_grid), username_entry, 1, 0, 1, 1);

    auth_label = gtk_label_new("Mot de passe:");
    gtk_grid_attach(GTK_GRID(auth_grid), auth_label, 0, 1, 1, 1);

    password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_grid_attach(GTK_GRID(auth_grid), password_entry, 1, 1, 1, 1);

    GtkWidget *auth_button = gtk_button_new_with_label("Se connecter");
    g_signal_connect(auth_button, "clicked", G_CALLBACK(authenticate), NULL);
    gtk_grid_attach(GTK_GRID(auth_grid), auth_button, 1, 2, 1, 1);

    output_label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(auth_grid), output_label, 0, 3, 2, 1);

    // Créer la fenêtre du menu
    menu_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(menu_window), "Menu");
    gtk_container_set_border_width(GTK_CONTAINER(menu_window), 10);
    gtk_widget_set_size_request(menu_window, 400, 300);

    g_signal_connect(menu_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *menu_grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(menu_window), menu_grid);

    GtkWidget *menu_button = gtk_button_new_with_label("Afficher le menu");
    g_signal_connect(menu_button, "clicked", G_CALLBACK(showMenu), NULL);
    gtk_grid_attach(GTK_GRID(menu_grid), menu_button, 0, 0, 1, 1);

    GtkWidget *response_button = gtk_button_new_with_label("Réponse du serveur");
    g_signal_connect(response_button, "clicked", G_CALLBACK(handleServerResponse), NULL);
    gtk_grid_attach(GTK_GRID(menu_grid), response_button, 1, 0, 1, 1);

    output_label = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(menu_grid), output_label, 0, 1, 2, 1);

    // Se connecter au serveur ici (avant d'entrer dans la boucle principale GTK)
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Afficher la fenêtre d'authentification
    gtk_widget_show_all(auth_window);

    // Entrer dans la boucle principale GTK
    gtk_main();

    // Fermer le socket client après la sortie de la boucle principale GTK
    close(client_socket);

    return 0;
}
