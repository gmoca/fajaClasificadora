# Configuración de Termux para la TUI

## Requisitos

- Android 8+ con Bluetooth
- Termux **desde F-Droid** (NO Google Play — versión desactualizada)
- HC-05 configurado a 115200 baud (ver `docs/hc05-setup.md`)

## Instalación

### 1. Instalar Termux desde F-Droid

```
https://f-droid.org/packages/com.termux/
```

### 2. Abrir Termux y actualizar

```bash
pkg update && pkg upgrade -y
pkg install python clang make git -y
```

### 3. Clonar el repositorio

```bash
git clone https://github.com/gmoca/fajaClasificadora
cd fajaClasificadora/tui_app
```

### 4. Instalar dependencias Python

```bash
pip install textual pyserial pyserial-asyncio
```

## Conexión Bluetooth

Hay dos métodos. Recomendado el segundo (no requiere root).

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

Usa la app **Serial Bluetooth Terminal** de Kai Morich como puente TCP.

```
https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal
```

#### Pasos:

1. **Parear el HC-05** desde Ajustes → Bluetooth del teléfono
2. **Abrir Serial Bluetooth Terminal**
3. Presionar el ícono de conexión (↕) → seleccionar **Devices**
4. Elegir el HC-05 de la lista
5. Una vez conectado, presionar el botón de menú (⋮) → **TCP Server**
6. Elegir puerto **8080** (debe coincidir con el que intenta la TUI)
7. Activar **Start Server** — la app muestra "TCP server started on port 8080"
8. En Termux, ejecutar la TUI:

```bash
cd fajaClasificadora/tui_app
python app.py
```

La TUI intentará conexión serial a `/dev/rfcomm0`, `/dev/ttyUSB0`, `COM3`, y si falla, probará TCP `127.0.0.1:8080` automáticamente.

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

## Script de inicio automático

```bash
cd fajaClasificadora/tui_app
bash start.sh
```

Esto crea un entorno virtual, instala dependencias y ejecuta la app.

## Solución de problemas

| Problema | Causa | Solución |
|----------|-------|----------|
| `ModuleNotFoundError: serial` | Faltan dependencias | `pip install pyserial` |
| `Connection refused` al conectar TCP | Serial BT Terminal no inició TCP Server | Abrir app, menú → TCP Server → Start |
| `No device` en TCP | Puerto incorrecto | Verificar puerto en la app (default 8080) |
| Datos corruptos o caracteres raros | Baud rate incorrecto | HC-05 debe estar en 115200 (ver `docs/hc05-setup.md`) |
| `git clone` falla | Sin git | `pkg install git` |
| La TUI no responde | HC-05 no conectado | Verificar LED del HC-05 (debe parpadear rápido) |
