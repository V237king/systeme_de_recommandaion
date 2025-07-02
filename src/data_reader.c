#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./../include/data_reader.h"

int read_ratings_file(const char* filename, Transaction** transactions, int* count,
                      int filter_by_date, long start_time, long end_time,
                      int remove_rare, int min_occurrences,
                      int is_test, Transaction* train_transactions, int train_count) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier ratings.txt");
        return -1;
    }

    // Initialiser un tableau dynamique
    int capacity = 100; // Taille initiale
    Transaction* temp_transactions = (Transaction*)malloc(capacity * sizeof(Transaction));
    if (temp_transactions == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour temp_transactions\n");
        fclose(file);
        return -1;
    }

    int temp_count = 0;
    char line[256]; // Buffer pour chaque ligne
    while (fgets(line, sizeof(line), file)) {
        // Supprimer le caractère de nouvelle ligne. strcpn va nous donner la position du caractère '\n' (la taille de la chaine sans le '\n')
        line[strcspn(line, "\n")] = 0;

        // Lire les champs séparés par des virgules
        int user_id, item_id, category_id;
        float rating;
        long timestamp;

        if (sscanf(line, "%d,%d,%d,%f,%ld",
                   &user_id, &item_id, &category_id, &rating, &timestamp) == 5) {
            // Redimensionner si nécessaire
            if (temp_count >= capacity) {
                capacity *= 2;
                Transaction* new_temp = (Transaction*)realloc(temp_transactions, capacity * sizeof(Transaction));
                if (new_temp == NULL) {
                    fprintf(stderr, "Erreur de réallocation mémoire pour temp_transactions\n");
                    free(temp_transactions);
                    fclose(file);
                    return -1;
                }
                temp_transactions = new_temp;
            }

            temp_transactions[temp_count].user_id = user_id;
            temp_transactions[temp_count].item_id = item_id;
            temp_transactions[temp_count].category_id = category_id;
            temp_transactions[temp_count].rating = rating;
            temp_transactions[temp_count].timestamp = timestamp;
            temp_count++;
        } else {
            fprintf(stderr, "Erreur de format dans la ligne : %s\n", line);
            continue;
        }
    }

    fclose(file);

    // Appliquer le filtrage par date
    Transaction* date_filtered;
    int date_filtered_count;
    if (filter_by_date) {
        if (filter_transactions_by_date(temp_transactions, temp_count, start_time,
                                       end_time, &date_filtered, &date_filtered_count) != 0) {
            free(temp_transactions);
            return -1;
        }
    } else {
        date_filtered = temp_transactions;
        date_filtered_count = temp_count;
        temp_transactions = NULL; // Éviter de libérer deux fois
    }

    // Appliquer la suppression des items rares si demandé
    Transaction* rare_filtered;
    int rare_filtered_count;
    if (remove_rare) {
        if (remove_rare_items(date_filtered, date_filtered_count, min_occurrences,
                              &rare_filtered, &rare_filtered_count) != 0) {
            free(date_filtered);
            free(temp_transactions);
            return -1;
        }
        free(date_filtered);
    } else {
        rare_filtered = date_filtered;
        rare_filtered_count = date_filtered_count;
        date_filtered = NULL;
    }

    // Nettoyer les données de test si is_test est activé
    if (is_test) {
        if (clean_test_data(rare_filtered, rare_filtered_count,
                            train_transactions, train_count,
                            transactions, count) != 0) {
            free(rare_filtered);
            free(temp_transactions);
            free(date_filtered);
            return -1;
        }
        free(rare_filtered);
    } else {
        *transactions = rare_filtered;
        *count = rare_filtered_count;
        rare_filtered = NULL;
    }

    free(temp_transactions);
    free(date_filtered);
    return 0;
}

int filter_transactions_by_date(Transaction* transactions, int count, long start_time,
                               long end_time, Transaction** filtered, int* filtered_count) {
    // Allouer un tableau dynamique pour les transactions filtrées
    int capacity = count > 0 ? count : 1;
    *filtered = (Transaction*)malloc(capacity * sizeof(Transaction));
    if (*filtered == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour filtered\n");
        return -1;
    }

    *filtered_count = 0;
    for (int i = 0; i < count; i++) {
        if (transactions[i].timestamp >= start_time && transactions[i].timestamp <= end_time) {
            if (*filtered_count >= capacity) {
                capacity *= 2;
                Transaction* new_filtered = (Transaction*)realloc(*filtered, capacity * sizeof(Transaction));
                if (new_filtered == NULL) {
                    fprintf(stderr, "Erreur de réallocation mémoire pour filtered\n");
                    free(*filtered);
                    return -1;
                }
                *filtered = new_filtered;
            }
            (*filtered)[*filtered_count] = transactions[i];
            (*filtered_count)++;
        }
    }

    return 0;
}

int remove_rare_items(Transaction* transactions, int count, int min_occurrences,
                      Transaction** filtered, int* filtered_count) {
    // Compter les occurrences de chaque item
    int max_item_id = 0;
    for (int i = 0; i < count; i++) {
        if (transactions[i].item_id > max_item_id) {
            max_item_id = transactions[i].item_id;
        }
    }

    int* item_counts = (int*)calloc(max_item_id + 1, sizeof(int));
    if (item_counts == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour item_counts\n");
        return -1;
    }

    // Compter les occurrences
    for (int i = 0; i < count; i++) {
        item_counts[transactions[i].item_id]++;
    }

    // Allouer un tableau dynamique pour les transactions filtrées
    int capacity = count > 0 ? count : 1;
    *filtered = (Transaction*)malloc(capacity * sizeof(Transaction));
    if (*filtered == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour filtered\n");
        free(item_counts);
        return -1;
    }

    // Filtrer les transactions
    *filtered_count = 0;
    for (int i = 0; i < count; i++) {
        if (item_counts[transactions[i].item_id] >= min_occurrences) {
            if (*filtered_count >= capacity) {
                capacity *= 2;
                Transaction* new_filtered = (Transaction*)realloc(*filtered, capacity * sizeof(Transaction));
                if (new_filtered == NULL) {
                    fprintf(stderr, "Erreur de réallocation mémoire pour filtered\n");
                    free(*filtered);
                    free(item_counts);
                    return -1;
                }
                *filtered = new_filtered;
            }
            (*filtered)[*filtered_count] = transactions[i];
            (*filtered_count)++;
        }
    }

    free(item_counts);
    return 0;
}

int clean_test_data(Transaction* test_transactions, int test_count,
                    Transaction* train_transactions, int train_count,
                    Transaction** cleaned, int* cleaned_count) {
    // Trouver les user_id et item_id valides dans les données d'entraînement
    int max_user_id = 0, max_item_id = 0;
    for (int i = 0; i < train_count; i++) {
        if (train_transactions[i].user_id > max_user_id) {
            max_user_id = train_transactions[i].user_id;
        }
        if (train_transactions[i].item_id > max_item_id) {
            max_item_id = train_transactions[i].item_id;
        }
    }

    int* valid_users = (int*)calloc(max_user_id + 1, sizeof(int));
    int* valid_items = (int*)calloc(max_item_id + 1, sizeof(int));
    if (valid_users == NULL || valid_items == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour valid_users ou valid_items\n");
        free(valid_users);
        free(valid_items);
        return -1;
    }

    // Marquer les user_id et item_id présents dans les données d'entraînement
    for (int i = 0; i < train_count; i++) {
        valid_users[train_transactions[i].user_id] = 1;
        valid_items[train_transactions[i].item_id] = 1;
    }

    // Allouer un tableau dynamique pour les transactions nettoyées
    int capacity = test_count > 0 ? test_count : 1;
    *cleaned = (Transaction*)malloc(capacity * sizeof(Transaction));
    if (*cleaned == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour cleaned\n");
        free(valid_users);
        free(valid_items);
        return -1;
    }

    // Filtrer les transactions de test
    *cleaned_count = 0;
    for (int i = 0; i < test_count; i++) {
        if (valid_users[test_transactions[i].user_id] && valid_items[test_transactions[i].item_id]) {
            if (*cleaned_count >= capacity) {
                capacity *= 2;
                Transaction* new_cleaned = (Transaction*)realloc(*cleaned, capacity * sizeof(Transaction));
                if (new_cleaned == NULL) {
                    fprintf(stderr, "Erreur de réallocation mémoire pour cleaned\n");
                    free(*cleaned);
                    free(valid_users);
                    free(valid_items);
                    return -1;
                }
                *cleaned = new_cleaned;
            }
            (*cleaned)[*cleaned_count] = test_transactions[i];
            (*cleaned_count)++;
        }
    }

    free(valid_users);
    free(valid_items);
    return 0;
}