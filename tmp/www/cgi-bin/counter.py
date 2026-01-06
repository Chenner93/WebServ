#!/usr/bin/env python3
# counter.py

import os
import http.cookies

# Lire les cookies existants
cookie_string = os.environ.get('HTTP_COOKIE', '')
cookies = http.cookies.SimpleCookie()
if cookie_string:
    cookies.load(cookie_string)

# Récupérer le compteur (ou 0 si premier visit)
count = 0
if 'visit_count' in cookies:
    try:
        count = int(cookies['visit_count'].value)
    except ValueError:
        count = 0

# Incrémenter
count += 1

# Créer le nouveau cookie
new_cookie = http.cookies.SimpleCookie()
new_cookie['visit_count'] = count
new_cookie['visit_count']['path'] = '/'
new_cookie['visit_count']['max-age'] = 3600  # 1 heure

# Envoyer la réponse
print("Status: 200")
print("Content-Type: text/html; charset=utf-8")
print(new_cookie.output())  # Set-Cookie header
print()  # Ligne vide obligatoire

print(f"""
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="utf-8">
    <title>Compteur de Visites 🍪</title>
    <style>
        body {{
            font-family: system-ui, sans-serif;
            text-align: center;
            margin-top: 3rem;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            min-height: 100vh;
        }}
        .counter {{
            font-size: 5rem;
            font-weight: bold;
            margin: 2rem 0;
            text-shadow: 3px 3px 6px rgba(0,0,0,0.3);
        }}
        button {{
            padding: 1rem 2rem;
            font-size: 1.2rem;
            border: none;
            border-radius: 50px;
            background: white;
            color: #667eea;
            cursor: pointer;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
            transition: transform 0.2s;
        }}
        button:hover {{
            transform: scale(1.05);
        }}
        .info {{
            margin-top: 2rem;
            opacity: 0.9;
        }}
    </style>
</head>
<body>
    <h1>🍪 Compteur de Visites</h1>
    
    <div class="counter">{count}</div>
    
    <p style="font-size: 1.5rem;">
        {"Première visite ! 🎉" if count == 1 else f"Visite n°{count}"}
    </p>
    
    <form method="get">
        <button type="submit">Recharger 🔄</button>
    </form>
    
    <div class="info">
        <p>Cookie: <code>visit_count={count}</code></p>
        <p>Ce compteur est stocké dans un cookie pendant 1 heure</p>
    </div>
</body>
</html>
""")