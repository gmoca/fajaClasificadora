# Configuración de Termux y PC para la TUI

## Requisitos

- Android 8+ con Bluetooth (para celular) o PC (Windows/Linux/macOS)
- Termux **desde F-Droid** (NO Google Play — versión desactualizada)
- HC-05 configurado a 115200 baud (ver [hc05-setup.md](file:///c:/Users/gusta/source/repos/gmoca/pryMicroFaja.X/docs/hc05-setup.md))

---

## Instalación y Arranque Rápido

### 1. Preparar Termux (Android) o PC
*   **En Android (Termux):** Abre la app y prepara los paquetes iniciales ejecutando:
    ```bash
    pkg update && pkg upgrade -y
    pkg install python clang make git -y
    ```
*   **En PC (Windows):** Asegúrate de tener instalado Python 3.8+ y Git agregados al PATH del sistema.

### 2. Clonar el repositorio
Abre tu terminal y ejecuta:
```bash
git clone https://github.com/gmoca/fajaClasificadora
cd fajaClasificadora/tui_app
```

---

### Opción A: Autoinstalación y Arranque (Recomendado / Más Rápido)

El proyecto cuenta con scripts que automatizan la creación de un entorno virtual aislado de Python (`.venv`), instalan las dependencias necesarias (`textual`, `pyserial-asyncio`, etc.) y lanzan la aplicación TUI con un solo comando.

*   **En Android (Termux) o Linux/macOS:**
    Dale permisos de ejecución al script `start.sh` y ejecútalo:
    ```bash
    chmod +x start.sh
    ./start.sh
    ```
*   **En Windows:**
    Simplemente haz doble clic en `start.bat` o ejecútalo desde tu consola:
    ```cmd
    start.bat
    ```

> [!NOTE]
> Estos scripts son inteligentes: en la primera ejecución descargarán e instalarán todo de forma transparente, y en los arranques posteriores iniciarán la TUI directamente en milisegundos.

---

### Opción B: Instalación Manual Paso a Paso (Alternativo)

Si prefieres realizar la instalación y configuración manualmente sin usar los scripts:

1.  **Crear y activar el entorno virtual:**
    *   *En Termux / Linux / macOS:*
        ```bash
        python -m venv .venv
        source .venv/bin/activate
        ```
    *   *En Windows (PowerShell):*
        ```powershell
        python -m venv .venv
        .venv\Scripts\Activate.ps1
        ```
2.  **Instalar dependencias:**
    Instala los paquetes en modo editable utilizando el archivo de configuración del proyecto:
    ```bash
    pip install --upgrade pip
    pip install -e .
    ```
3.  **Lanzar la TUI:**
    ```bash
    python app.py
    ```

---

## Conexión Bluetooth

Hay dos métodos de vinculación en celular. Se recomienda el segundo (Método B) porque no requiere root.

### Método A: Directo por rfcomm (requiere root)

```bash
# Escanear dispositivos Bluetooth
sudo hcitool scan

# Vincular con la dirección MAC del HC-05
# (hacer desde Ajustes → Bluetooth del teléfono)

# Crear el puerto serie virtual
sudo rfcomm bind 0 <MAC_DEL_HC05> 1

# Probar
python app.py
# La app intentará /dev/rfcomm0 automáticamente
```

### Método B: Serial Bluetooth Terminal (recomendado, sin root)

Usa la app **Serial Bluetooth Terminal** de Kai Morich en Android como puente TCP local.

```
https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal
```

#### Pasos:

1.  **Parear el HC-05** desde Ajustes → Bluetooth del teléfono.
2.  **Abrir Serial Bluetooth Terminal**.
3.  Presionar el ícono de conexión (↕) → seleccionar **Devices**.
4.  Elegir el HC-05 de la lista.
5.  Una vez conectado, presionar el botón de menú (⋮) → **TCP Server**.
6.  Elegir puerto **8080** (o 9000 / 1234).
7.  Activar **Start Server** — la app mostrará "TCP server started on port 8080".
8.  En Termux, ejecuta la TUI con el script automático o comando manual.
    La TUI detectará que corre en Termux e intentará la conexión TCP local de forma inmediata.

#### Verificar conexión:

```bash
python -c "
import asyncio
async def test():
    r, w = await asyncio.open_connection('127.0.0.1', 8080)
    w.write(b'STATUS\n')
    await w.drain()
    data = await asyncio.wait_for(r.readline(), timeout=2)
    print('Respuesta:', data.decode().strip())
    w.close()
asyncio.run(test())
"
```

Debería responder: `STATUS_RESP:...`

### Método C: USB-OTG + cable USB-TTL (alternativa sin Bluetooth)

Si no usas Bluetooth, puedes conectar un cable USB-TTL directamente al PIC:

```
USB-TTL → HC-05 (solo RX/TX, sin alimentar)
O directo al PIC: TX→RC7, RX→RC6
```

Conectas el cable USB-TTL al teléfono vía USB-OTG. En Termux:

```bash
# Instalar soporte USB serie
pkg install usbutils
lsusb  # debe aparecer el adaptador USB-TTL
# Normalmente aparece como /dev/ttyUSB0 o /dev/ttyACM0

python app.py
# La app intentará /dev/ttyUSB0 automáticamente
```

---

## Solución de problemas

| Problema | Causa | Solución |
|----------|-------|----------|
| `ModuleNotFoundError: serial` | Faltan dependencias | Ejecutar con `start.sh` / `start.bat` o correr `pip install -e .` en el entorno virtual activo. |
| `Connection refused` al conectar TCP | Serial BT Terminal no inició TCP Server | Abrir app, menú → TCP Server → Start |
| `No device` en TCP | Puerto incorrecto | Verificar puerto en la app (debe ser 8080, 9000 o 1234) |
| Datos corruptos o caracteres raros | Baud rate incorrecto | HC-05 debe estar en 115200 (ver `docs/hc05-setup.md`) |
| `git clone` falla | Sin git | `pkg install git` |
| La TUI no responde | HC-05 no conectado | Verificar LED del HC-05 (debe parpadear rápido) |
