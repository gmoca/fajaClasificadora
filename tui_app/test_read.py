import asyncio

async def test_read():
    print("Intentando conectar a 127.0.0.1:8080...")
    try:
        reader, writer = await asyncio.open_connection("127.0.0.1", 8080)
        print("¡Conectado exitosamente!")
        print("Esperando datos del PIC (presiona Ctrl+C para salir)...")
        while True:
            # Leemos datos crudos (hasta 1024 bytes) en lugar de readline
            data = await reader.read(1024)
            if not data:
                print("El servidor cerró la conexión.")
                break
            print(f"Recibido crudo: {repr(data)}")
            try:
                decoded = data.decode('utf-8', errors='ignore')
                print(f"Decodificado: {decoded.strip()}")
            except Exception as e:
                print(f"Error al decodificar: {e}")
    except Exception as e:
        print(f"Error de conexión: {e}")

if __name__ == "__main__":
    try:
        asyncio.run(test_read())
    except KeyboardInterrupt:
        print("\nPrueba finalizada por el usuario.")
