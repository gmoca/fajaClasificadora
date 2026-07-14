from textual.app import ComposeResult
from textual.containers import Horizontal, Vertical
from textual.widgets import Static, Button, Digits, Log
from textual.screen import Screen
from textual.reactive import reactive

class DashboardScreen(Screen):
    state = reactive("IDLE")
    speed = reactive(0)
    last_color = reactive("-")

    def compose(self) -> ComposeResult:
        with Horizontal(id="dash-main"):
            with Vertical(id="dash-stats"):
                yield Static("Estado del Sistema", classes="panel-title")
                yield Static("IDLE", id="lbl-state")
                
                yield Static("Velocidad (mm/s)", classes="panel-title")
                yield Digits("0", id="lbl-speed")
                
                yield Static("Último Color Detectado", classes="panel-title")
                yield Static("-", id="lbl-color")
            
            with Vertical(id="dash-log"):
                yield Static("Log de Eventos", classes="panel-title")
                yield Log(id="sys-log")
                
        with Horizontal(id="dash-controls"):
            yield Button("START", id="btn-start", variant="success")
            yield Button("STOP", id="btn-stop", variant="error")
            yield Button("Configuración", id="btn-config")
            yield Button("Visor Logs", id="btn-logs")
            yield Button("Modo TEST", id="btn-test")

    def watch_state(self, new_state: str) -> None:
        try:
            self.query_one("#lbl-state", Static).update(f"{new_state}")
        except:
            pass

    def watch_speed(self, new_speed: int) -> None:
        try:
            self.query_one("#lbl-speed", Digits).update(f"{new_speed}")
        except:
            pass
            
    def watch_last_color(self, new_color: str) -> None:
        try:
            self.query_one("#lbl-color", Static).update(f"{new_color}")
        except:
            pass
            
    def log_event(self, message: str) -> None:
        try:
            self.query_one("#sys-log", Log).write_line(message)
        except:
            pass

    def on_button_pressed(self, event: Button.Pressed) -> None:
        button_id = event.button.id
        if button_id == "btn-start":
            self.app.bt_send("START")
        elif button_id == "btn-stop":
            self.app.bt_send("STOP")
        elif button_id == "btn-config":
            self.app.push_screen("config")
        elif button_id == "btn-logs":
            self.app.push_screen("logs")
        elif button_id == "btn-test":
            self.app.push_screen("test")
