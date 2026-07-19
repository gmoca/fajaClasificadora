from textual.app import ComposeResult
from textual.containers import Horizontal, Vertical
from textual.widgets import Static, Button, Digits, Log
from textual.screen import Screen
from textual.reactive import reactive

class DashboardScreen(Screen):
    state = reactive("IDLE")
    speed = reactive(0)
    pulses = reactive(0)
    last_color = reactive("-")
    red_count = reactive(0)
    green_count = reactive(0)
    blue_count = reactive(0)
    pwm = reactive(180)

    def compose(self) -> ComposeResult:
        with Horizontal(id="dash-main"):
            with Vertical(id="dash-stats"):
                yield Static("Estado del Sistema", classes="panel-title")
                yield Static("IDLE", id="lbl-state", classes="state-idle")
                
                yield Static("Velocidad (mm/s)", classes="panel-title")
                yield Digits("0", id="lbl-speed")
                
                yield Static("Pulsos Encoder", classes="panel-title")
                yield Static("0", id="lbl-pulses")

                yield Static("Último Color Detectado", classes="panel-title")
                yield Static("-", id="lbl-color")
                
                yield Static("Contadores de Clasificación", classes="panel-title")
                yield Static("Rojo: 0 | Verde: 0 | Azul: 0", id="lbl-counters")
            
            with Vertical(id="dash-log"):
                yield Static("Log de Eventos", classes="panel-title")
                yield Log(id="sys-log")
                
        with Horizontal(id="dash-controls"):
            yield Button("START", id="btn-start", variant="success")
            yield Button("STOP", id="btn-stop", variant="error")
            yield Button("Configuración", id="btn-config")
            yield Button("Visor Logs", id="btn-logs")
            yield Button("Modo TEST", id="btn-test")
            
        with Horizontal(id="dash-speed-control"):
            yield Static("Ajustar PWM Motor (0-255):", id="lbl-pwm-title", classes="panel-title")
            from textual.widgets import Input
            yield Input(placeholder="180", id="input-speed", restrict=r"^\d{0,3}$")

    def watch_state(self, new_state: str) -> None:
        try:
            widget = self.query_one("#lbl-state", Static)
            widget.update(f"{new_state}")
            
            # Reset style classes
            widget.remove_class("state-idle")
            widget.remove_class("state-run")
            widget.remove_class("state-sort")
            widget.remove_class("state-test")
            widget.remove_class("state-err")
            
            # Add dynamic class based on current state
            s = new_state.lower()
            if "idle" in s:
                widget.add_class("state-idle")
            elif "run" in s:
                widget.add_class("state-run")
            elif "sort" in s:
                widget.add_class("state-sort")
            elif "test" in s:
                widget.add_class("state-test")
            elif "err" in s or "emergency" in s:
                widget.add_class("state-err")
        except:
            pass

    def watch_speed(self, new_speed: int) -> None:
        try:
            self.query_one("#lbl-speed", Digits).update(f"{new_speed}")
        except:
            pass

    def watch_pulses(self, new_pulses: int) -> None:
        try:
            self.query_one("#lbl-pulses", Static).update(f"{new_pulses}")
        except:
            pass

    def watch_pwm(self, new_pwm: int) -> None:
        try:
            self.query_one("#lbl-pwm-title", Static).update(f"Ajustar PWM Motor (0-255) [Actual: {new_pwm}]:")
            self.query_one("#input-speed", Input).placeholder = str(new_pwm)
        except:
            pass
            
    def watch_last_color(self, new_color: str) -> None:
        try:
            widget = self.query_one("#lbl-color", Static)
            widget.update(f"{new_color}")
            
            widget.remove_class("color-rojo")
            widget.remove_class("color-verde")
            widget.remove_class("color-azul")
            
            c = new_color.lower()
            if "rojo" in c:
                widget.add_class("color-rojo")
            elif "verde" in c:
                widget.add_class("color-verde")
            elif "azul" in c:
                widget.add_class("color-azul")
        except:
            pass
            
    def increment_color_count(self, color_idx: str):
        if color_idx == "0":
            self.red_count += 1
        elif color_idx == "1":
            self.green_count += 1
        elif color_idx == "2":
            self.blue_count += 1

    def update_counters_label(self) -> None:
        try:
            self.query_one("#lbl-counters", Static).update(
                f"Rojo: {self.red_count} | Verde: {self.green_count} | Azul: {self.blue_count}"
            )
        except:
            pass

    def watch_red_count(self, new_val: int) -> None:
        self.update_counters_label()

    def watch_green_count(self, new_val: int) -> None:
        self.update_counters_label()

    def watch_blue_count(self, new_val: int) -> None:
        self.update_counters_label()
            
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

    def on_input_submitted(self, event) -> None:
        if event.input.id == "input-speed":
            try:
                val = int(event.value)
                if 0 <= val <= 255:
                    self.app.bt_send(f"SET_SPEED {val}")
                    self.log_event(f"TUI: Enviado SET_SPEED {val}")
                else:
                    self.log_event("TUI: Error - Velocidad debe ser 0-255")
            except ValueError:
                pass
            event.input.value = ""

    def on_resize(self, event) -> None:
        if event.size.width >= 90:
            self.add_class("wide-screen")
            self.remove_class("narrow-screen")
        else:
            self.add_class("narrow-screen")
            self.remove_class("wide-screen")
