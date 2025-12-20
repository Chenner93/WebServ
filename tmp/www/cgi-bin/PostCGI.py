#!/usr/bin/env python3

import cgi
import cgitb

# Active les erreurs visibles (utile en dev)
cgitb.enable()

# Indique que la réponse est du HTML
print("Content-Type: text/html\n")

# Récupération des données POST
form = cgi.FieldStorage()
message = form.getvalue("message", "aucun message reçu")

# Réponse
print(f"""
<html>
<head>
    <title>CGI Test</title>
</head>
<body>
    <h1>CGI Python OK</h1>
    <p>Message reçu :</p>
    <b>{message}</b>
</body>
</html>
""")
