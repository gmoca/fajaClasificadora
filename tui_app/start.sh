#!/bin/bash

echo "======================================================"
echo "  Iniciando Lanzador Automático (Linux/macOS/Termux)  "
echo "======================================================"

# 1. Detectar si estamos en Termux (Android)
if [ -d "/data/data/com.termux" ]; then
    echo "[INFO] Detectado entorno Android (Termux)."
    echo "[INFO] Verificando paquetes de sistema requeridos..."
    
    # Comprobar python3
    if ! command -v python3 &> /dev/null; then
        echo "[INFO] Instalando python..."
        pkg update -y && pkg install python -y
    fi
    # Comprobar clang/make/git (por si se compila alguna rueda de pip)
    if ! command -v clang &> /dev/null; then
        echo "[INFO] Instalando compiladores clang/make/git..."
        pkg install clang make git -y
    fi
else
    echo "[INFO] Detectado sistema Unix-like (PC Linux/macOS)."
    # Comprobar si python3 existe en PC
    if ! command -v python3 &> /dev/null; then
        echo "[ERROR] Python 3 no está instalado en este equipo."
        echo "Instálalo usando el gestor de paquetes de tu distribución o descárgalo de python.org"
        exit 1
    fi
fi

# 2. Crear entorno virtual si no existe
if [ ! -d "venv" ]; then
    echo "[INFO] Creando entorno virtual de Python (venv)..."
    python3 -m venv venv
    if [ $? -ne 0 ]; then
        echo "[ERROR] No se pudo crear el entorno virtual. Asegúrate de tener python3-venv instalado."
        exit 1
    fi
fi

# 3. Activar entorno virtual e instalar dependencias
echo "[INFO] Activando entorno virtual..."
source venv/bin/activate

echo "[INFO] Instalando y actualizando dependencias de la TUI..."
pip install --upgrade pip
pip install -e .

# 4. Iniciar la aplicación
echo "[INFO] Iniciando la aplicación..."
python3 app.py
