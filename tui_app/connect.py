import asyncio


class BTManager:
    def __init__(self):
        self.reader = None
        self.writer = None
        self._connected = False
        self._mode = None

    @property
    def connected(self) -> bool:
        return self._connected

    async def connect(self, port: str, baud: int = 115200) -> bool:
        try:
            import serial_asyncio
            self.reader, self.writer = await serial_asyncio.open_serial_connection(
                url=port, baudrate=baud
            )
            self._connected = True
            self._mode = "serial"
            return True
        except Exception:
            self._connected = False
            return False

    async def connect_tcp(self, host: str = "127.0.0.1", port: int = 8080) -> bool:
        try:
            with open("tui_log.txt", "a") as f:
                f.write(f"Intentando conectar TCP a {host}:{port}...\n")
            self.reader, self.writer = await asyncio.open_connection(host, port)
            self._connected = True
            self._mode = "tcp"
            with open("tui_log.txt", "a") as f:
                f.write("¡Conexión TCP exitosa!\n")
            return True
        except Exception as e:
            self._connected = False
            with open("tui_log.txt", "a") as f:
                f.write(f"Fallo conexión TCP: {e}\n")
            return False

    async def disconnect(self):
        if self.writer:
            self.writer.close()
            if hasattr(self.writer, "wait_closed"):
                await self.writer.wait_closed()
        self._connected = False
        self._mode = None
        with open("tui_log.txt", "a") as f:
            f.write("Desconectado.\n")

    async def send(self, cmd: str):
        if self.writer and self._connected:
            with open("tui_log.txt", "a") as f:
                f.write(f"Enviado: {cmd}\n")
            self.writer.write((cmd + "\n").encode())
            await self.writer.drain()

    async def read_line(self) -> str | None:
        if not self.reader or not self._connected:
            return None
        try:
            # Aumentamos el timeout a 0.4s para evitar falsos timeouts con datos segmentados
            line = await asyncio.wait_for(self.reader.readline(), timeout=0.4)
            if line:
                decoded = line.decode(errors='ignore').strip()
                with open("tui_log.txt", "a") as f:
                    f.write(f"Leído: {decoded}\n")
                return decoded
            return None
        except asyncio.TimeoutError:
            # No loggear timeouts para no llenar el archivo, es normal si el PIC no envía nada
            return None
        except Exception as e:
            self._connected = False
            with open("tui_log.txt", "a") as f:
                f.write(f"Excepción en lectura: {e}\n")
            return None
