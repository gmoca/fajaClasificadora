from textual.app import ComposeResult
from textual.containers import Vertical, Horizontal
from textual.widgets import Static, Button, Input, Label, TabbedContent, TabPane
from textual.screen import Screen
import asyncio

class ConfigScreen(Screen):
    def compose(self) -> ComposeResult:
        with Vertical(id="config-main"):
            yield Static("Configuración de Parámetros", classes="panel-title")
            
            with Horizontal(id="config-body"):
                # Columna Izquierda: Faja, Motor, Servos
                with Vertical(id="config-left-col"):
                    yield Static("Faja y Servos", classes="panel-sub-title")
                    
                    with Vertical(classes="config-card"):
                        yield Static("Faja y Motor", classes="card-title")
                        with Horizontal(classes="config-row"):
                            yield Label("Velocidad (0-255):")
                            yield Input(placeholder="Ej: 180", id="inp-speed")
                            yield Button("Aplicar", id="btn-apply-speed")
                        with Horizontal(classes="config-row"):
                            yield Label("Espaciado (pulsos):")
                            yield Input(placeholder="Ej: 100", id="inp-spacing")
                            yield Button("Aplicar", id="btn-apply-spacing")
                            
                    with Vertical(classes="config-card"):
                        yield Static("Servo 1 (Clasificador Principal - CCP2)", classes="card-title")
                        with Horizontal(classes="config-row"):
                            yield Label("Home Angle (0-180):")
                            yield Input(placeholder="90", id="inp-s1-home")
                            yield Button("Test", id="btn-test-s1-home")
                        with Horizontal(classes="config-row"):
                            yield Label("Defl Angle (0-180):")
                            yield Input(placeholder="0", id="inp-s1-defl")
                            yield Button("Test", id="btn-test-s1-defl")
                        with Horizontal(classes="config-row"):
                            yield Label("Dwell Time (ms):")
                            yield Input(placeholder="500", id="inp-s1-dwell")
                        with Horizontal(classes="config-row"):
                            yield Label("Distance (mm):")
                            yield Input(placeholder="200", id="inp-s1-dist")
                            yield Button("Guardar", id="btn-save-s1", variant="success")
                            
                    with Vertical(classes="config-card"):
                        yield Static("Servo 2 (Clasificador Secundario - TMR3)", classes="card-title")
                        with Horizontal(classes="config-row"):
                            yield Label("Home Angle (0-180):")
                            yield Input(placeholder="90", id="inp-s2-home")
                            yield Button("Test", id="btn-test-s2-home")
                        with Horizontal(classes="config-row"):
                            yield Label("Defl Angle (0-180):")
                            yield Input(placeholder="0", id="inp-s2-defl")
                            yield Button("Test", id="btn-test-s2-defl")
                        with Horizontal(classes="config-row"):
                            yield Label("Dwell Time (ms):")
                            yield Input(placeholder="500", id="inp-s2-dwell")
                        with Horizontal(classes="config-row"):
                            yield Label("Distance (mm):")
                            yield Input(placeholder="250", id="inp-s2-dist")
                            yield Button("Guardar", id="btn-save-s2", variant="success")
                
                # Columna Derecha: Sensor de Color y Calibración
                with Vertical(id="config-right-col"):
                    yield Static("Sensor de Color", classes="panel-sub-title")
                    
                    with Vertical(classes="config-card"):
                        yield Static("Calibración y Sincronización", classes="card-title")
                        with Horizontal(classes="config-row-actions"):
                            yield Button("Cargar de EEPROM", id="btn-load-config", variant="primary")
                            yield Button("Calibrar Color", id="btn-calibrate", variant="warning")
                    
                    with Vertical(classes="config-card"):
                        yield Static("Límites de Umbral de Color (EEPROM)", classes="card-title")
                        with TabbedContent():
                            with TabPane("Rojo (ID 0)", id="tab-rojo"):
                                with Horizontal(classes="config-row"):
                                    yield Label("Rojo Min/Max:")
                                    yield Input(placeholder="Min", id="inp-color-r-min-0")
                                    yield Input(placeholder="Max", id="inp-color-r-max-0")
                                with Horizontal(classes="config-row"):
                                    yield Label("Verde Min/Max:")
                                    yield Input(placeholder="Min", id="inp-color-g-min-0")
                                    yield Input(placeholder="Max", id="inp-color-g-max-0")
                                with Horizontal(classes="config-row"):
                                    yield Label("Azul Min/Max:")
                                    yield Input(placeholder="Min", id="inp-color-b-min-0")
                                    yield Input(placeholder="Max", id="inp-color-b-max-0")
                                with Horizontal(classes="config-row"):
                                    yield Label("Servo (0=Pasa, 1=S1, 2=S2):")
                                    yield Input(placeholder="Ej: 1", id="inp-color-servo-0")
                                with Horizontal(classes="config-row"):
                                    yield Button("Guardar Rojo en EEPROM", id="btn-save-color-0", variant="success")
                            
                            with TabPane("Verde (ID 1)", id="tab-verde"):
                                with Horizontal(classes="config-row"):
                                    yield Label("Rojo Min/Max:")
                                    yield Input(placeholder="Min", id="inp-color-r-min-1")
                                    yield Input(placeholder="Max", id="inp-color-r-max-1")
                                with Horizontal(classes="config-row"):
                                    yield Label("Verde Min/Max:")
                                    yield Input(placeholder="Min", id="inp-color-g-min-1")
                                    yield Input(placeholder="Max", id="inp-color-g-max-1")
                                with Horizontal(classes="config-row"):
                                    yield Label("Azul Min/Max:")
                                    yield Input(placeholder="Min", id="inp-color-b-min-1")
                                    yield Input(placeholder="Max", id="inp-color-b-max-1")
                                with Horizontal(classes="config-row"):
                                    yield Label("Servo (0=Pasa, 1=S1, 2=S2):")
                                    yield Input(placeholder="Ej: 1", id="inp-color-servo-1")
                                with Horizontal(classes="config-row"):
                                    yield Button("Guardar Verde en EEPROM", id="btn-save-color-1", variant="success")
                                    
                            with TabPane("Azul (ID 2)", id="tab-azul"):
                                with Horizontal(classes="config-row"):
                                    yield Label("Rojo Min/Max:")
                                    yield Input(placeholder="Min", id="inp-color-r-min-2")
                                    yield Input(placeholder="Max", id="inp-color-r-max-2")
                                with Horizontal(classes="config-row"):
                                    yield Label("Verde Min/Max:")
                                    yield Input(placeholder="Min", id="inp-color-g-min-2")
                                    yield Input(placeholder="Max", id="inp-color-g-max-2")
                                with Horizontal(classes="config-row"):
                                    yield Label("Azul Min/Max:")
                                    yield Input(placeholder="Min", id="inp-color-b-min-2")
                                    yield Input(placeholder="Max", id="inp-color-b-max-2")
                                with Horizontal(classes="config-row"):
                                    yield Label("Servo (0=Pasa, 1=S1, 2=S2):")
                                    yield Input(placeholder="Ej: 2", id="inp-color-servo-2")
                                with Horizontal(classes="config-row"):
                                    yield Button("Guardar Azul en EEPROM", id="btn-save-color-2", variant="success")
            
            with Horizontal(id="config-footer"):
                yield Button("Volver al Dashboard", id="btn-back", variant="primary")

    def update_servo_config(self, servo_id: int, config_parts: list) -> None:
        try:
            if len(config_parts) < 4:
                return
            home_val = config_parts[1]
            defl_val = config_parts[2]
            dwell_val = config_parts[3]
            dist_val = config_parts[4] if len(config_parts) >= 5 else (200 if servo_id == 1 else 250)
            
            if servo_id == 1:
                self.query_one("#inp-s1-home", Input).value = str(home_val)
                self.query_one("#inp-s1-defl", Input).value = str(defl_val)
                self.query_one("#inp-s1-dwell", Input).value = str(dwell_val)
                self.query_one("#inp-s1-dist", Input).value = str(dist_val)
                self.app.notify("Configuración de Servo 1 cargada exitosamente")
            elif servo_id == 2:
                self.query_one("#inp-s2-home", Input).value = str(home_val)
                self.query_one("#inp-s2-defl", Input).value = str(defl_val)
                self.query_one("#inp-s2-dwell", Input).value = str(dwell_val)
                self.query_one("#inp-s2-dist", Input).value = str(dist_val)
                self.app.notify("Configuración de Servo 2 cargada exitosamente")
        except Exception as e:
            self.app.notify(f"Error cargando servo {servo_id}: {e}", severity="error")

    def on_button_pressed(self, event: Button.Pressed) -> None:
        button_id = event.button.id
        if button_id == "btn-back":
            self.app.pop_screen()
            
        elif button_id == "btn-load-config":
            self.app.bt_send("SERVO_GET_CONFIG 1")
            self.app.bt_send("SERVO_GET_CONFIG 2")
            self.app.notify("Solicitando configuración al microcontrolador...")
            
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
                
        elif button_id == "btn-apply-spacing":
            try:
                val = self.query_one("#inp-spacing", Input).value
                spacing = int(val)
                self.app.bt_send(f"SET_SPACING {spacing}")
                self.app.notify(f"Comando enviado: SET_SPACING {spacing}")
            except ValueError:
                self.app.notify("Error: Ingrese un valor numérico", severity="error")

        elif button_id == "btn-calibrate":
            self.app.bt_send("CALIBRATE")
            self.app.notify("Comando de calibración enviado")

        elif button_id == "btn-test-s1-home":
            try:
                val = int(self.query_one("#inp-s1-home", Input).value)
                self.app.bt_send(f"SERVO 1 {val}")
                self.app.notify("Probando Home de Servo 1")
            except ValueError:
                self.app.notify("Ingrese un ángulo de Home válido para Servo 1", severity="error")

        elif button_id == "btn-test-s1-defl":
            try:
                val = int(self.query_one("#inp-s1-defl", Input).value)
                self.app.bt_send(f"SERVO 1 {val}")
                self.app.notify("Probando Deflexión de Servo 1")
            except ValueError:
                self.app.notify("Ingrese un ángulo de Deflexión válido para Servo 1", severity="error")

        elif button_id == "btn-test-s2-home":
            try:
                val = int(self.query_one("#inp-s2-home", Input).value)
                self.app.bt_send(f"SERVO 2 {val}")
                self.app.notify("Probando Home de Servo 2")
            except ValueError:
                self.app.notify("Ingrese un ángulo de Home válido para Servo 2", severity="error")

        elif button_id == "btn-test-s2-defl":
            try:
                val = int(self.query_one("#inp-s2-defl", Input).value)
                self.app.bt_send(f"SERVO 2 {val}")
                self.app.notify("Probando Deflexión de Servo 2")
            except ValueError:
                self.app.notify("Ingrese un ángulo de Deflexión válido para Servo 2", severity="error")

        elif button_id == "btn-save-s1":
            try:
                home = int(self.query_one("#inp-s1-home", Input).value)
                defl = int(self.query_one("#inp-s1-defl", Input).value)
                dwell = int(self.query_one("#inp-s1-dwell", Input).value)
                dist = int(self.query_one("#inp-s1-dist", Input).value)
                
                async def run_save():
                    self.app.bt_send(f"SERVO 1 {home}")
                    await asyncio.sleep(0.2)
                    self.app.bt_send("SERVO_SAVE_HOME 1")
                    await asyncio.sleep(0.2)
                    self.app.bt_send(f"SERVO 1 {defl}")
                    await asyncio.sleep(0.2)
                    self.app.bt_send("SERVO_SAVE_DEFLECT 1")
                    await asyncio.sleep(0.2)
                    self.app.bt_send(f"SET_DWELL 1 {dwell}")
                    await asyncio.sleep(0.2)
                    self.app.bt_send(f"SET_DIST 1 {dist}")
                    await asyncio.sleep(0.2)
                    self.app.bt_send(f"SERVO 1 {home}")
                    self.app.notify("Parámetros de Servo 1 guardados exitosamente")
                
                asyncio.create_task(run_save())
            except ValueError:
                self.app.notify("Error: Rellene Home, Defl, Dwell y Dist para Servo 1", severity="error")

        elif button_id == "btn-save-s2":
            try:
                home = int(self.query_one("#inp-s2-home", Input).value)
                defl = int(self.query_one("#inp-s2-defl", Input).value)
                dwell = int(self.query_one("#inp-s2-dwell", Input).value)
                dist = int(self.query_one("#inp-s2-dist", Input).value)
                
                async def run_save():
                    self.app.bt_send(f"SERVO 2 {home}")
                    await asyncio.sleep(0.2)
                    self.app.bt_send("SERVO_SAVE_HOME 2")
                    await asyncio.sleep(0.2)
                    self.app.bt_send(f"SERVO 2 {defl}")
                    await asyncio.sleep(0.2)
                    self.app.bt_send("SERVO_SAVE_DEFLECT 2")
                    await asyncio.sleep(0.2)
                    self.app.bt_send(f"SET_DWELL 2 {dwell}")
                    await asyncio.sleep(0.2)
                    self.app.bt_send(f"SET_DIST 2 {dist}")
                    await asyncio.sleep(0.2)
                    self.app.bt_send(f"SERVO 2 {home}")
                    self.app.notify("Parámetros de Servo 2 guardados exitosamente")
                
                asyncio.create_task(run_save())
            except ValueError:
                self.app.notify("Error: Rellene Home, Defl, Dwell y Dist para Servo 2", severity="error")

        elif button_id.startswith("btn-save-color-"):
            try:
                idx = int(button_id.split("-")[-1])
                servo_id = int(self.query_one(f"#inp-color-servo-{idx}", Input).value)
                r_min = int(self.query_one(f"#inp-color-r-min-{idx}", Input).value)
                r_max = int(self.query_one(f"#inp-color-r-max-{idx}", Input).value)
                g_min = int(self.query_one(f"#inp-color-g-min-{idx}", Input).value)
                g_max = int(self.query_one(f"#inp-color-g-max-{idx}", Input).value)
                b_min = int(self.query_one(f"#inp-color-b-min-{idx}", Input).value)
                b_max = int(self.query_one(f"#inp-color-b-max-{idx}", Input).value)
                
                if 0 <= servo_id <= 2:
                    cmd = f"SET_THRESHOLD {idx} {r_min} {r_max} {g_min} {g_max} {b_min} {b_max} {servo_id}"
                    self.app.bt_send(cmd)
                    self.app.notify(f"Color {idx} guardado en EEPROM con Servo {servo_id}")
                else:
                    self.app.notify("Error: Servo debe ser 0 (pasa), 1 o 2", severity="error")
            except ValueError:
                self.app.notify("Error: Todos los campos de color deben ser números", severity="error")

    def on_resize(self, event) -> None:
        if event.size.width >= 90:
            self.add_class("wide-screen")
            self.remove_class("narrow-screen")
        else:
            self.add_class("narrow-screen")
            self.remove_class("wide-screen")
