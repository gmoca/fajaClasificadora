from textual.app import ComposeResult
from textual.containers import Vertical, Horizontal
from textual.widgets import Static, Button, Log, Input
from textual.screen import Screen

class TestScreen(Screen):
    def compose(self) -> ComposeResult:
        with Vertical(id="test-main"):
            yield Static("Modo de Prueba (ST_TEST)", classes="panel-title")
            
            with Horizontal():
                yield Button("Activar Modo TEST", id="btn-test-enter", variant="warning")
                yield Button("Salir de Modo TEST", id="btn-test-exit", variant="success")
            
            yield Static("Control de Motor", classes="panel-title")
            with Horizontal():
                yield Button("Pulsar Motor (2s watchdog)", id="btn-test-motor")
            
            yield Static("Control de Servos", classes="panel-title")
            with Horizontal():
                yield Input(placeholder="Servo ID (1 o 2)", id="inp-servo-id")
                yield Input(placeholder="Ángulo (0-180)", id="inp-servo-angle")
                yield Button("Mover Servo", id="btn-test-servo")
                
            yield Static("Log de Pruebas", classes="panel-title")
            yield Log(id="test-log")
            
            with Horizontal():
                yield Button("Volver al Dashboard", id="btn-back", variant="primary")

    def log_event(self, message: str) -> None:
        try:
            self.query_one("#test-log", Log).write_line(message)
        except:
            pass

    def on_button_pressed(self, event: Button.Pressed) -> None:
        button_id = event.button.id
        if button_id == "btn-back":
            self.app.pop_screen()
        elif button_id == "btn-test-enter":
            self.app.bt_send("TEST_ENTER")
            self.log_event("Enviado: TEST_ENTER")
        elif button_id == "btn-test-exit":
            self.app.bt_send("TEST_EXIT")
            self.log_event("Enviado: TEST_EXIT")
        elif button_id == "btn-test-motor":
            self.app.bt_send("TEST_MOTOR")
            self.log_event("Enviado: TEST_MOTOR")
        elif button_id == "btn-test-servo":
            try:
                sid = int(self.query_one("#inp-servo-id", Input).value)
                ang = int(self.query_one("#inp-servo-angle", Input).value)
                self.app.bt_send(f"SERVO {sid} {ang}")
                self.log_event(f"Enviado: SERVO {sid} {ang}")
            except ValueError:
                self.app.notify("Ingrese ID y ángulo válidos", severity="error")
