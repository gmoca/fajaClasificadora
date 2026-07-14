@echo off
title Faja Transportadora TUI - Lanzador Automático

echo ======================================================
echo   Iniciando Lanzador Automático (Windows)
echo ======================================================

:: Verificar si Python está instalado
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Python no está instalado o no está agregado al PATH de Windows.
    echo Por favor, descarga e instala Python desde: https://www.python.org/
    pause
    exit /b
)

:: Crear entorno virtual si no existe
if not exist venv (
    echo [INFO] Creando entorno virtual de Python (venv)...
    python -m venv venv
    if %errorlevel% neq 0 (
        echo [ERROR] No se pudo crear el entorno virtual.
        pause
        exit /b
    )
)

:: Activar entorno e instalar dependencias
echo [INFO] Activando entorno virtual...
call venv\Scripts\activate

echo [INFO] Verificando e instalando dependencias (Textual, PySerial, etc.)...
pip install --upgrade pip
pip install -e .

echo [INFO] Iniciando la aplicación...
python app.py

pause
