from textual.app import ComposeResult
from textual.containers import Vertical, Horizontal
from textual.widgets import Static, Button, Log, Input, Label
from textual.screen import Screen

class TestScreen(Screen):
    def compose(self) -> ComposeResult:
        with Vertical(id="test-main"):
            yield Static("Modo de Prueba (ST_TEST)", classes="panel-title")
            
            with Horizontal(id="test-body"):
                # Columna Izquierda: Monitoreo (Sensores & Logs)
                with Vertical(id="test-left-col"):
                    yield Static("Monitoreo de Sensores", classes="panel-sub-title")
                    
                    with Vertical(classes="test-card"):
                        yield Static("Barreras Láser (Break-Beams)", classes="card-title")
                        with Horizontal(classes="test-indicator-row"):
                            yield Static("Beam 1: -", id="lbl-beam-1", classes="beam-indicator")
                            yield Static("Beam 2: -", id="lbl-beam-2", classes="beam-indicator")
                            
                    with Vertical(classes="test-card"):
                        yield Static("Sensor de Color TCS34725", classes="card-title")
                        with Horizontal(classes="test-indicator-row"):
                            yield Static("R: -", id="lbl-color-r", classes="color-indicator")
                            yield Static("G: -", id="lbl-color-g", classes="color-indicator")
                            yield Static("B: -", id="lbl-color-b", classes="color-indicator")
                            yield Static("C: -", id="lbl-color-c", classes="color-indicator")
                            
                    with Vertical(classes="test-card"):
                        yield Static("Color Dominante Estimado", classes="card-title")
                        yield Static("[ NINGUNO ]", id="lbl-test-detected-color", classes="color-preview-box")
                            
                    with Vertical(classes="test-card"):
                        yield Static("Botones Físicos (PIC)", classes="card-title")
                        with Horizontal(classes="test-indicator-row"):
                            yield Static("UP", id="lbl-btn-up", classes="btn-indicator")
                            yield Static("DOWN", id="lbl-btn-down", classes="btn-indicator")
                            yield Static("MODE", id="lbl-btn-mode", classes="btn-indicator")
                            
                    with Vertical(classes="test-card", id="test-log-card"):
                        yield Static("Registro de Eventos (Pruebas)", classes="card-title")
                        yield Log(id="test-log")
                
                # Columna Derecha: Control de Actuadores
                with Vertical(id="test-right-col"):
                    yield Static("Control de Actuadores", classes="panel-sub-title")
                    
                    with Vertical(classes="test-card"):
                        yield Static("Modo de Mantenimiento", classes="card-title")
                        with Horizontal(classes="test-row-actions"):
                            yield Button("ENTRAR TEST", id="btn-test-enter", variant="warning")
                            yield Button("SALIR TEST", id="btn-test-exit", variant="success")
                            
                    with Vertical(classes="test-card"):
                        yield Static("Motor y Encoder", classes="card-title")
                        with Horizontal(classes="test-row-actions"):
                            yield Button("Girar Motor (2s)", id="btn-test-motor")
                            yield Button("Reset Encoder", id="btn-test-enc-reset", variant="warning")
                            
                    with Vertical(classes="test-card"):
                        yield Static("Servomotores", classes="card-title")
                        with Horizontal(classes="test-row-inline"):
                            yield Label("Servo ID:")
                            yield Input(placeholder="1-2", id="inp-servo-id", restrict=r"^[1-2]{0,1}$")
                            yield Label("Ángulo:")
                            yield Input(placeholder="0-180", id="inp-servo-angle", restrict=r"^\d{0,3}$")
                            yield Button("Mover", id="btn-test-servo")
                        with Horizontal(classes="test-row-actions"):
                            yield Button("Probar Home", id="btn-test-servo-home")
                            yield Button("Probar Deflect", id="btn-test-servo-defl")
            
            with Horizontal(id="test-footer"):
                yield Button("Volver al Dashboard", id="btn-back", variant="primary")

    def on_mount(self) -> None:
        # Polling hardware states every 500ms
        self.pending_test_action = None
        self.set_interval(0.5, self.poll_hardware)

    def poll_hardware(self) -> None:
        if self.app.bt.connected:
            self.app.bt_send("TEST_BEAM")
            self.app.bt_send("TEST_BUTTON_ECHO")
            self.app.bt_send("TEST_COLOR")

    def handle_servo_config(self, servo_id: int, config_parts: list) -> None:
        try:
            if not hasattr(self, "pending_test_action") or not self.pending_test_action:
                return
            action, sid = self.pending_test_action
            if sid != servo_id or len(config_parts) < 4:
                return
            
            home_val = int(config_parts[1])
            defl_val = int(config_parts[2])
            
            target_angle = home_val if action == "home" else defl_val
            self.app.bt_send(f"SERVO {servo_id} {target_angle}")
            self.log_event(f"Probando {action.upper()} en Servo {servo_id}: Moviendo a {target_angle}°")
            self.pending_test_action = None
        except Exception as e:
            self.log_event(f"Error al probar servo: {e}")

    def update_beam(self, station: int, state: str) -> None:
        try:
            widget = self.query_one(f"#lbl-beam-{station}", Static)
            widget.remove_class("beam-clear")
            widget.remove_class("beam-blocked")
            if state == "B":
                widget.update(f"Beam {station}: OBSTRUIDO [●]")
                widget.add_class("beam-blocked")
            else:
                widget.update(f"Beam {station}: LIBRE [○]")
                widget.add_class("beam-clear")
        except:
            pass

    def update_beams_multi(self, beams: dict) -> None:
        for sid, state in beams.items():
            self.update_beam(sid, state)

    def update_color_raw(self, r: int, g: int, b: int, c: int) -> None:
        try:
            self.query_one("#lbl-color-r", Static).update(f"R: {r}")
            self.query_one("#lbl-color-g", Static).update(f"G: {g}")
            self.query_one("#lbl-color-b", Static).update(f"B: {b}")
            self.query_one("#lbl-color-c", Static).update(f"C: {c}")
            
            # Estimación del color dominante
            widget = self.query_one("#lbl-test-detected-color", Static)
            widget.remove_class("preview-rojo")
            widget.remove_class("preview-verde")
            widget.remove_class("preview-azul")
            
            # Evitar ruido de oscuridad o fondo (si Clear < 150)
            if c < 150:
                widget.update("[ NINGUNO ]")
            elif r > g and r > b:
                widget.update("[ ROJO ]")
                widget.add_class("preview-rojo")
            elif g > r and g > b:
                widget.update("[ VERDE ]")
                widget.add_class("preview-verde")
            elif b > r and b > g:
                widget.update("[ AZUL ]")
                widget.add_class("preview-azul")
            else:
                widget.update("[ INDETERMINADO ]")
        except:
            pass

    def update_buttons(self, up: bool, down: bool, mode: bool) -> None:
        try:
            for btn_name, pressed in [("up", up), ("down", down), ("mode", mode)]:
                widget = self.query_one(f"#lbl-btn-{btn_name}", Static)
                widget.remove_class("btn-pressed")
                if pressed:
                    widget.add_class("btn-pressed")
        except:
            pass

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
        elif button_id == "btn-test-enc-reset":
            self.app.bt_send("TEST_ENCODER_RESET")
            self.log_event("Enviado: TEST_ENCODER_RESET")
        elif button_id == "btn-test-servo":
            try:
                sid = int(self.query_one("#inp-servo-id", Input).value)
                ang = int(self.query_one("#inp-servo-angle", Input).value)
                if (sid == 1 or sid == 2) and 0 <= ang <= 180:
                    self.app.bt_send(f"SERVO {sid} {ang}")
                    self.log_event(f"Enviado: SERVO {sid} {ang}")
                else:
                    self.app.notify("ID de servo o ángulo fuera de rango", severity="error")
            except ValueError:
                self.app.notify("Ingrese ID y ángulo válidos", severity="error")
        elif button_id == "btn-test-servo-home":
            try:
                sid = int(self.query_one("#inp-servo-id", Input).value)
                if sid == 1 or sid == 2:
                    self.pending_test_action = ("home", sid)
                    self.app.bt_send(f"SERVO_GET_CONFIG {sid}")
                    self.log_event(f"Solicitando config de Servo {sid} para probar HOME...")
                else:
                    self.app.notify("ID de servo debe ser 1 o 2", severity="error")
            except ValueError:
                self.app.notify("Ingrese un Servo ID válido", severity="error")
        elif button_id == "btn-test-servo-defl":
            try:
                sid = int(self.query_one("#inp-servo-id", Input).value)
                if sid == 1 or sid == 2:
                    self.pending_test_action = ("defl", sid)
                    self.app.bt_send(f"SERVO_GET_CONFIG {sid}")
                    self.log_event(f"Solicitando config de Servo {sid} para probar DEFLECT...")
                else:
                    self.app.notify("ID de servo debe ser 1 o 2", severity="error")
            except ValueError:
                self.app.notify("Ingrese un Servo ID válido", severity="error")

    def on_resize(self, event) -> None:
        if event.size.width >= 90:
            self.add_class("wide-screen")
            self.remove_class("narrow-screen")
        else:
            self.add_class("narrow-screen")
            self.remove_class("wide-screen")
