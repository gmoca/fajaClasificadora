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
    line = line.strip()
    if not line:
        return {"type": "UNKNOWN", "raw": line}

    # Support compound space-separated telemetry lines (e.g. "STATE:idle SPEED:0 PULSES:0")
    if " " in line and ":" in line:
        parts = line.split()
        # Verify if all space-separated parts look like key:value
        # Note: some values might contain colons themselves (like BEAM:1:B), so we check if ':' exists in each part
        if all(":" in p for p in parts):
            data = {"type": "COMPOUND", "raw": line, "parts": {}}
            for p in parts:
                k, _, v = p.partition(":")
                # Standardize key names in lowercase
                k_lower = k.lower()
                # Parse numeric values if possible
                try:
                    if v.isdigit():
                        data["parts"][k_lower] = int(v)
                    elif (v.startswith("-") and v[1:].isdigit()):
                        data["parts"][k_lower] = int(v)
                    else:
                        data["parts"][k_lower] = v
                except ValueError:
                    data["parts"][k_lower] = v
            return data

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
    elif key in ("SERVO_CONFIG", "SERVO_CFG"):
        parts = val.split(",")
        return {"type": "SERVO_CONFIG", "servo": int(parts[0]) if parts else 0, "config": parts}
    elif key in ("ENCODER_COUNT", "ENC"):
        return {"type": "ENCODER_COUNT", "pulses": int(val)}
    elif key == "BEAM":
        if " " in val:
            # Format: BEAM:1:B 2:C
            subparts = val.split()
            beams = {}
            for sp in subparts:
                sp_key, _, sp_val = sp.partition(":")
                beams[int(sp_key)] = sp_val
            return {"type": "BEAM_MULTI", "beams": beams}
        else:
            parts = val.split(":")
            return {"type": "BEAM", "station": int(parts[0]), "state": parts[1]}
    elif key == "BUTTON" or key == "BTN":
        if len(val) == 3 and all(c in "01" for c in val):
            return {
                "type": "BUTTONS",
                "up": val[0] == '1',
                "down": val[1] == '1',
                "mode": val[2] == '1',
                "raw": val
            }
        else:
            try:
                return {"type": "BUTTON", "id": int(val)}
            except ValueError:
                return {"type": "BUTTON", "raw": val}
    return {"type": "UNKNOWN", "raw": line}
