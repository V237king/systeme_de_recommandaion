#ifndef DATA_READER_H
#define DATA_READER_H

// Structure pour stocker une transaction
typedef struct {
    int user_id;
    int item_id;
    int category_id;
    float rating;
    long timestamp;
} Transaction;

// Structure pour stocker une recommandation
typedef struct {
    int item_id;
    float score;
} Recommendation;

// Lit le fichier ratings.txt et stocke les transactions dans un tableau
// Paramètres : filename (nom du fichier), transactions (pointeur vers tableau alloué dynamiquement),
//              count (nombre max de transactions), filter_by_date (activer le filtrage par date),
//              start_time (début de la période), end_time (fin de la période),
//              remove_rare (activer la suppression des items rares), min_occurrences (seuil minimum),
//              is_test (indique si c'est un fichier de test), train_transactions (transactions d'entraînement),
//              train_count (nombre de transactions d'entraînement)
// Retour : 0 si succès, -1 si erreur
int read_ratings_file(const char* filename, Transaction** transactions, int* count,
                      int filter_by_date, long start_time, long end_time,
                      int remove_rare, int min_occurrences,
                      int is_test, Transaction* train_transactions, int train_count);

// Filtre les transactions selon une plage de dates
// Paramètres : transactions (tableau d'entrée), count (nombre de transactions),
//              start_time (début), end_time (fin),
//              filtered (tableau de sortie alloué dynamiquement), filtered_count (nombre de transactions filtrées)
// Retour : 0 si succès, -1 si erreur
int filter_transactions_by_date(Transaction* transactions, int count, long start_time,
                               long end_time, Transaction** filtered, int* filtered_count);

// Supprime les items apparaissant moins de min_occurrences fois
// Paramètres : transactions (tableau d'entrée), count (nombre de transactions),
//              min_occurrences (seuil minimum), filtered (tableau de sortie alloué dynamiquement),
//              filtered_count (nombre de transactions filtrées)
// Retour : 0 si succès, -1 si erreur
int remove_rare_items(Transaction* transactions, int count, int min_occurrences,
                      Transaction** filtered, int* filtered_count);

// Nettoie les données de test pour correspondre aux données d'entraînement
// Paramètres : test_transactions (transactions de test), test_count (nombre de transactions de test),
//              train_transactions (transactions d'entraînement), train_count (nombre de transactions d'entraînement),
//              cleaned (tableau de sortie alloué dynamiquement), cleaned_count (nombre de transactions nettoyées)
// Retour : 0 si succès, -1 si erreur
int clean_test_data(Transaction* test_transactions, int test_count,
                    Transaction* train_transactions, int train_count,
                    Transaction** cleaned, int* cleaned_count);

#endif