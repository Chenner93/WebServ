#!/usr/bin/env python3
import time
import sys

def lignes_droles_et_intelligentes():
    # Headers CGI
    print("Content-Type: text/plain; charset=utf-8")
    print("Status: 200")
    print()  # Ligne vide, SEP for header
    
    sys.stdout.flush()  # ← CRITIQUE
    print("Patate")
    sys.stdout.flush()

    time.sleep(60)

if __name__ == "__main__":
    lignes_droles_et_intelligentes()