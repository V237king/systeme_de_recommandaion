import random
import time

# Paramètres
num_train_transactions = 1000
num_test_transactions = 200
num_users = 50
num_items = 100
num_categories = 10
rating_min = 1.0
rating_max = 5.0
start_timestamp = int(time.mktime(time.strptime("2024-01-01", "%Y-%m-%d")))
end_timestamp = int(time.mktime(time.strptime("2025-12-31", "%Y-%m-%d")))

# Générer train_ratings.txt
train_data = []
for _ in range(num_train_transactions):
    user_id = random.randint(1, num_users)
    item_id = random.randint(1, num_items)
    category_id = random.randint(1, num_categories)
    rating = round(random.uniform(rating_min, rating_max), 1)
    timestamp = random.randint(start_timestamp, end_timestamp)
    train_data.append(f"{user_id},{item_id},{category_id},{rating},{timestamp}")

# Écrire train_ratings.txt
with open("data/train_ratings.txt", "w") as f:
    f.write("\n".join(train_data))

# Générer test_ratings.txt (user_id et item_id doivent exister dans train_data)
train_user_ids = set(line.split(",")[0] for line in train_data)
train_item_ids = set(line.split(",")[1] for line in train_data)
test_data = []
for _ in range(num_test_transactions):
    user_id = random.choice(list(train_user_ids))
    item_id = random.choice(list(train_item_ids))
    category_id = random.randint(1, num_categories)
    rating = round(random.uniform(rating_min, rating_max), 1)
    timestamp = random.randint(start_timestamp, end_timestamp)
    test_data.append(f"{user_id},{item_id},{category_id},{rating},{timestamp}")

# Écrire test_ratings.txt
with open("data/test_ratings.txt", "w") as f:
    f.write("\n".join(test_data))

print("Fichiers générés : data/train_ratings.txt et data/test_ratings.txt")