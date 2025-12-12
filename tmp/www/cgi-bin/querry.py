#!/usr/bin/env python3
import os

query = os.environ.get('QUERY_STRING', '')

print("Content-Type: text/html")
print("Status: 200")
print()
print(f"<h1>Query: {query}</h1>")