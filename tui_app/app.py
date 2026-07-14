from textual.app import App, ComposeResult
from textual.binding import Binding
from textual.widgets import Header, Footer
from connect import BTManager
from protocol import parse_telemetry
from screens.dashboard import DashboardScreen
from screens.config import ConfigScreen
from screens.log_viewer import LogViewerScreen
from screens.test_screen import TestScreen
import asyncio

class FajaApp(App):
    TITLE = "Faja Transportadora"
    SUB_TITLE = "PIC18F4550 Control"

    BINDINGS = [
        Binding("q", "quit", "Salir"),
        Binding("c", "connect", "Conectar BT"),
    ]
    
    COLOR_MAP = {
        "0": "Rojo",
        "1": "Verde",
        "2": "Azul"
    }
    
    SCREENS = {
        "dashboard": DashboardScreen,
        "config": ConfigScreen,
        "logs": LogViewerScreen,
        "test": TestScreen,
    }

    def __init__(self):
        super().__init__()
        self.bt = BTManager()
        self.last_state = "unknown"

    def compose(self) -> ComposeResult:
        yield Header()
        yield Footer()

    async def on_mount(self):
        self.push_screen("dashboard")
        self.set_interval(0.5, self.poll_bluetooth)
        self.set_interval(3.0, self.auto_reconnect)

    async def auto_reconnect(self):
        if not self.bt.connected:
            await self.action_connect(silent=True)

    async def poll_bluetooth(self):
        if not self.bt.connected:
            return
        line = await self.bt.read_line()
        if line:
            data = parse_telemetry(line)
            self.update_dashboard(data)

    def update_dashboard(self, data: dict):
        # We broadcast the update to screens
        try:
            dash = self.query_one("DashboardScreen", DashboardScreen)
            t = data.get("type", "")
            if t == "STATE":
                dash.state = data.get("value", "unknown")
            elif t == "SPEED":
                dash.speed = data.get("value", 0)
            elif t == "DETECT":
                c_idx = str(data.get("color", "-"))
                dash.last_color = self.COLOR_MAP.get(c_idx, f"Idx: {c_idx}")
                
            # log it to screens if needed
            raw_line = str(data)
            dash.log_event(raw_line)
            try:
                logs = self.query_one("LogViewerScreen", LogViewerScreen)
                logs.log_event(raw_line)
            except:
                pass
                
            try:
                ts = self.query_one("TestScreen", TestScreen)
                ts.log_event(raw_line)
            except:
                pass
        except:
            pass

    def bt_send(self, cmd: str):
        asyncio.create_task(self.bt.send(cmd))

    async def action_connect(self, silent=False):
        if self.bt.connected:
            return
        for port in ["/dev/rfcomm0", "/dev/ttyUSB0", "COM3"]:
            if await self.bt.connect(port):
                if not silent:
                    self.notify(f"Conectado a {port}")
                return
        if not silent:
            self.notify("No se pudo conectar", severity="error")


if __name__ == "__main__":
    app = FajaApp()
    app.run()
