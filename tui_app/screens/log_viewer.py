from textual.app import ComposeResult
from textual.containers import Vertical
from textual.widgets import Static, Button, Log
from textual.screen import Screen

class LogViewerScreen(Screen):
    def compose(self) -> ComposeResult:
        with Vertical(id="logviewer-main"):
            yield Static("Visor de Logs Completo", classes="panel-title")
            yield Log(id="full-sys-log")
            yield Button("Volver al Dashboard", id="btn-back", variant="primary")

    def log_event(self, message: str) -> None:
        try:
            self.query_one("#full-sys-log", Log).write_line(message)
        except:
            pass

    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == "btn-back":
            self.app.pop_screen()
