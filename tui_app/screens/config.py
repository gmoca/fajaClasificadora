from textual.app import ComposeResult
from textual.containers import Vertical, Horizontal
from textual.widgets import Static, Button, Input, Label
from textual.screen import Screen

class ConfigScreen(Screen):
    def compose(self) -> ComposeResult:
        with Vertical(id="config-main"):
            yield Static("Configuración de Parámetros", classes="panel-title")
            
            with Horizontal(classes="config-row"):
                yield Label("Velocidad del Motor (0-255):")
                yield Input(placeholder="Ej: 180", id="inp-speed")
                yield Button("Aplicar Velocidad", id="btn-apply-speed")
                
            with Horizontal(classes="config-row"):
                yield Label("Calibrar Sensores de Color:")
                yield Button("Iniciar Calibración", id="btn-calibrate")
                
            with Horizontal(classes="config-row"):
                yield Button("Volver al Dashboard", id="btn-back", variant="primary")

    def on_button_pressed(self, event: Button.Pressed) -> None:
        button_id = event.button.id
        if button_id == "btn-back":
            self.app.pop_screen()
        elif button_id == "btn-apply-speed":
            try:
                val = self.query_one("#inp-speed", Input).value
                speed = int(val)
                if 0 <= speed <= 255:
                    self.app.bt_send(f"SET_SPEED {speed}")
                    self.app.notify(f"Comando enviado: SET_SPEED {speed}")
                else:
                    self.app.notify("Error: Velocidad debe estar entre 0 y 255", severity="error")
            except ValueError:
                self.app.notify("Error: Ingrese un valor numérico", severity="error")
        elif button_id == "btn-calibrate":
            self.app.bt_send("CALIBRATE")
            self.app.notify("Comando de calibración enviado")
