import requests

url = "https://www.gutenberg.org/cache/epub/30155/pg30155.txt"
print(f"Downloading Albert Einstein's 'Relativity' for brain training...")
response = requests.get(url)

if response.status_code == 200:
    with open("einstein_book.txt", "w", encoding="utf-8") as f:
        f.write(response.text)
    print("Download complete: einstein_book.txt")
else:
    print(f"Failed to download. Status code: {response.status_code}")
