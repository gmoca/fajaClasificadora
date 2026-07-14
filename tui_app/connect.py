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
            self.reader, self.writer = await asyncio.open_connection(host, port)
            self._connected = True
            self._mode = "tcp"
            return True
        except Exception:
            self._connected = False
            return False

    async def disconnect(self):
        if self.writer:
            self.writer.close()
            if hasattr(self.writer, "wait_closed"):
                await self.writer.wait_closed()
        self._connected = False
        self._mode = None

    async def send(self, cmd: str):
        if self.writer and self._connected:
            self.writer.write((cmd + "\n").encode())
            await self.writer.drain()

    async def read_line(self) -> str | None:
        if not self.reader or not self._connected:
            return None
        try:
            line = await asyncio.wait_for(self.reader.readline(), timeout=0.1)
            return line.decode().strip() if line else None
        except asyncio.TimeoutError:
            return None
        except Exception:
            self._connected = False
            return None
