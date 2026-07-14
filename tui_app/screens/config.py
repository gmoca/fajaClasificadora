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

            yield Static("Configuración de Servos (EEPROM)", classes="panel-title")
            
            with Horizontal(classes="config-row"):
                yield Label("Servo ID (1 o 2):")
                yield Input(placeholder="Ej: 1", id="inp-servo-cfg-id")
                
            with Horizontal(classes="config-row"):
                yield Button("Guardar Ángulo Actual como Home", id="btn-save-home", variant="warning")
                yield Button("Guardar Ángulo Actual como Deflexión", id="btn-save-defl", variant="warning")

            with Horizontal(classes="config-row"):
                yield Label("Tiempo Dwell (ms):")
                yield Input(placeholder="Ej: 500", id="inp-servo-dwell")
                yield Button("Guardar Dwell", id="btn-save-dwell")
                
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
        elif button_id == "btn-save-home":
            try:
                sid = int(self.query_one("#inp-servo-cfg-id", Input).value)
                if sid == 1 or sid == 2:
                    self.app.bt_send(f"SERVO_SAVE_HOME {sid}")
                    self.app.notify(f"Home guardado para Servo {sid}")
                else:
                    self.app.notify("Error: Servo ID debe ser 1 o 2", severity="error")
            except ValueError:
                self.app.notify("Error: Ingrese un Servo ID válido", severity="error")
        elif button_id == "btn-save-defl":
            try:
                sid = int(self.query_one("#inp-servo-cfg-id", Input).value)
                if sid == 1 or sid == 2:
                    self.app.bt_send(f"SERVO_SAVE_DEFLECT {sid}")
                    self.app.notify(f"Deflexión guardada para Servo {sid}")
                else:
                    self.app.notify("Error: Servo ID debe ser 1 o 2", severity="error")
            except ValueError:
                self.app.notify("Error: Ingrese un Servo ID válido", severity="error")
        elif button_id == "btn-save-dwell":
            try:
                sid = int(self.query_one("#inp-servo-cfg-id", Input).value)
                dwell_val = self.query_one("#inp-servo-dwell", Input).value
                dwell = int(dwell_val)
                if (sid == 1 or sid == 2) and dwell > 0:
                    self.app.bt_send(f"SET_DWELL {sid} {dwell}")
                    self.app.notify(f"Dwell de {dwell}ms guardado para Servo {sid}")
                else:
                    self.app.notify("Error: Datos de Servo ID o Dwell inválidos", severity="error")
            except ValueError:
                self.app.notify("Error: Ingrese valores numéricos", severity="error")
