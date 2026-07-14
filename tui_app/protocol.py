CMD_START = "START"
CMD_STOP = "STOP"
CMD_SET_SPEED = "SET_SPEED {}"
CMD_STATUS = "STATUS"
CMD_CALIBRATE = "CALIBRATE"
CMD_SET_MODE = "SET_MODE {}"
CMD_SERVO = "SERVO {} {}"
CMD_SET_SPACING = "SET_SPACING {}"
CMD_SET_THRESHOLD = "SET_THRESHOLD {} {} {} {} {}"
CMD_TEST_ENTER = "TEST_ENTER"
CMD_TEST_EXIT = "TEST_EXIT"
CMD_SERVO_SET = "SERVO_SET {} {}"
CMD_SERVO_SAVE_HOME = "SERVO_SAVE_HOME {}"
CMD_SERVO_SAVE_DEFLECT = "SERVO_SAVE_DEFLECT {}"
CMD_SERVO_GET_CONFIG = "SERVO_GET_CONFIG {}"
CMD_SET_DWELL = "SET_DWELL {} {}"
CMD_TEST_MOTOR = "TEST_MOTOR {} {}"
CMD_TEST_ENCODER_RESET = "TEST_ENCODER_RESET"
CMD_TEST_ENCODER_READ = "TEST_ENCODER_READ"
CMD_TEST_BEAM = "TEST_BEAM {}"
CMD_TEST_BUTTON_ECHO = "TEST_BUTTON_ECHO {}"


def parse_telemetry(line: str) -> dict:
    if ":" not in line:
        return {"type": "UNKNOWN", "raw": line}
    key, _, val = line.partition(":")
    if key == "STATE":
        return {"type": "STATE", "value": val}
    elif key == "SPEED":
        return {"type": "SPEED", "value": int(val)}
    elif key == "COLOR":
        parts = val.split(",")
        return {
            "type": "COLOR",
            "r": int(parts[0]), "g": int(parts[1]),
            "b": int(parts[2]), "c": int(parts[3]),
        }
    elif key == "DETECT":
        return {"type": "DETECT", "color": val}
    elif key == "JAM":
        return {"type": "JAM", "source": val or "unknown"}
    elif key.startswith("STATUS_RESP"):
        parts = val.split(",")
        return {"type": "STATUS", "state": parts[0], "speed": int(parts[1]) if len(parts) > 1 else 0}
    elif key == "CALIB_DONE":
        return {"type": "CALIB_DONE"}
    elif key == "SERVO_CONFIG":
        parts = val.split(",")
        return {"type": "SERVO_CONFIG", "servo": int(parts[0]) if parts else 0, "config": parts}
    elif key == "ENCODER_COUNT":
        return {"type": "ENCODER_COUNT", "pulses": int(val)}
    elif key == "BEAM":
        parts = val.split(":")
        return {"type": "BEAM", "station": int(parts[0]), "state": parts[1]}
    elif key == "BUTTON":
        return {"type": "BUTTON", "id": int(val)}
    return {"type": "UNKNOWN", "raw": line}
