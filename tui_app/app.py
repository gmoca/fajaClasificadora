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
import sys
import platform

class FajaApp(App):
    TITLE = "Faja Transportadora"
    SUB_TITLE = "PIC18F4550 Control"
    # agy: Load the stylesheet file
    CSS_PATH = "app.tcss"

    # agy: Added F1-F4 keyboard navigation bindings
    BINDINGS = [
        Binding("q", "quit", "Salir"),
        Binding("c", "connect", "Conectar BT"),
        Binding("f1", "show_dashboard", "F1: Dash"),
        Binding("f2", "show_config", "F2: Config"),
        Binding("f3", "show_test", "F3: Test"),
        Binding("f4", "show_logs", "F4: Logs"),
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

    # agy: Action method to cleanly switch between screens preventing stack leakage
    def switch_to_screen(self, screen_name: str):
        while len(self.screen_stack) > 2:
            self.pop_screen()
        if screen_name != "dashboard":
            self.push_screen(screen_name)

    def action_show_dashboard(self):
        self.switch_to_screen("dashboard")

    def action_show_config(self):
        self.switch_to_screen("config")

    def action_show_test(self):
        self.switch_to_screen("test")

    def action_show_logs(self):
        self.switch_to_screen("logs")

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

    def get_screen_safe(self, name: str, class_type):
        # Primero intentamos obtener la pantalla por su nombre registrado
        try:
            return self.get_screen(name)
        except Exception:
            pass
        # Si no, probamos si está activa y podemos buscarla por tipo
        try:
            return self.query_one(class_type)
        except Exception:
            pass
        # Si la pantalla actual es de ese tipo, la retornamos directamente
        if isinstance(self.screen, class_type):
            return self.screen
        return None

    def update_dashboard(self, data: dict):
        try:
            dash = self.get_screen_safe("dashboard", DashboardScreen)
            if not dash:
                return
                
            t = data.get("type", "")
            
            # agy: Handle compound telemetry parsing (multiple values in one packet)
            if t == "COMPOUND":
                parts = data.get("parts", {})
                if "state" in parts:
                    dash.state = parts["state"].upper()
                if "speed" in parts:
                    dash.speed = int(parts["speed"])
                if "pulses" in parts:
                    dash.pulses = int(parts["pulses"])
            
            elif t == "STATE":
                dash.state = data.get("value", "unknown").upper()
            elif t == "SPEED":
                dash.speed = data.get("value", 0)
            elif t == "ENCODER_COUNT":
                dash.pulses = data.get("pulses", 0)
            elif t == "DETECT":
                c_idx = str(data.get("color", "-"))
                # agy: increment color counter on dashboard
                dash.increment_color_count(c_idx)
                dash.last_color = self.COLOR_MAP.get(c_idx, f"Idx: {c_idx}")
            elif t == "JAM":
                dash.state = "ERROR"
                dash.log_event(f"[ALERT] Jam detected! Source: {data.get('source', 'unknown')}")
            
            # agy: route config messages to the ConfigScreen if mounted
            if t == "SERVO_CONFIG":
                cfg_scr = self.get_screen_safe("config", ConfigScreen)
                if cfg_scr:
                    try:
                        cfg_scr.update_servo_config(data.get("servo"), data.get("config"))
                    except Exception as e:
                        with open("tui_log.txt", "a") as f:
                            f.write(f"Error actualizando config screen: {e}\n")
                    
            # agy: route breakbeams and button status messages to TestScreen if mounted
            elif t in ("BEAM", "BEAM_MULTI", "BUTTONS", "BUTTON"):
                ts = self.get_screen_safe("test", TestScreen)
                if ts:
                    try:
                        if t == "BEAM":
                            ts.update_beam(data.get("station"), data.get("state"))
                        elif t == "BEAM_MULTI":
                            ts.update_beams_multi(data.get("beams"))
                        elif t == "BUTTONS":
                            ts.update_buttons(data.get("up"), data.get("down"), data.get("mode"))
                    except Exception as e:
                        with open("tui_log.txt", "a") as f:
                            f.write(f"Error actualizando test screen: {e}\n")

            raw_line = str(data)
            dash.log_event(raw_line)
            
            log_scr = self.get_screen_safe("logs", LogViewerScreen)
            if log_scr:
                try:
                    log_scr.log_event(raw_line)
                except Exception:
                    pass
                    
            ts_scr = self.get_screen_safe("test", TestScreen)
            if ts_scr:
                try:
                    ts_scr.log_event(raw_line)
                except Exception:
                    pass
        except Exception as e:
            with open("tui_log.txt", "a") as f:
                f.write(f"Error en update_dashboard: {e}\n")

    def bt_send(self, cmd: str):
        asyncio.create_task(self.bt.send(cmd))

    async def action_connect(self, silent=False):
        if self.bt.connected:
            return
            
        import os
        is_termux = "TERMUX_VERSION" in os.environ
        is_windows = platform.system() == "Windows"
        
        # Prioridad de puertos según plataforma
        if is_termux:
            # En celular/Termux, priorizar TCP (puente Serial BT) en puertos comunes
            tcp_ports = [8080, 9000, 1234]
            for t_port in tcp_ports:
                if await self.bt.connect_tcp("127.0.0.1", t_port):
                    if not silent:
                        self.notify(f"Conectado vía TCP (127.0.0.1:{t_port})")
                    return
            
            # Fallback a puertos seriales locales de Android/Linux
            serial_ports = ["/dev/rfcomm0", "/dev/ttyUSB0", "/dev/ttyACM0"]
            for port in serial_ports:
                if await self.bt.connect(port):
                    if not silent:
                        self.notify(f"Conectado a {port}")
                    return
        else:
            # En PC, priorizar puertos COM locales (Windows) o rfcomm (Linux)
            ports = [
                "COM3" if is_windows else "/dev/rfcomm0",
                "COM4" if is_windows else "/dev/ttyUSB0",
                "COM5" if is_windows else "/dev/ttyACM0",
                "COM6", "COM7"
            ]
            for port in ports:
                if await self.bt.connect(port):
                    if not silent:
                        self.notify(f"Conectado a {port}")
                    return
            
            # Fallback a TCP en PC
            if await self.bt.connect_tcp("127.0.0.1", 8080):
                if not silent:
                    self.notify("Conectado vía TCP (127.0.0.1:8080)")
                return
                
        if not silent:
            self.notify("No se pudo establecer conexión", severity="error")


if __name__ == "__main__":
    app = FajaApp()
    app.run()
