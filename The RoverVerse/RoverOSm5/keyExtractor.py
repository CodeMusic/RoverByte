import re

# Replace with the path to your Flipper NFC file
file_path = 'LaundryLoaded.nfc'

# Define a regular expression to match keys
key_pattern = re.compile(r'Key[A|B]:\s*([0-9A-Fa-f]{12})')

# Read and extract keys
keys = []
with open(file_path, 'r') as file:
    for line in file:
        match = key_pattern.search(line)
        if match:
            keys.append(match.group(1))

# Output the extracted keys
print("Extracted Keys:")
for idx, key in enumerate(keys):
    print(f"Key {idx + 1}: {key}")

# Optional: Save to a file
with open('extracted_keys.txt', 'w') as output_file:
    for key in keys:
        output_file.write(key + '\n')