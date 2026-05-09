import socket
import sys
import time


HOST = "127.0.0.1"
PORT = 4000


def main() -> int:
    print(f"[SERIAL] conectando em {HOST}:{PORT} ...")
    while True:
        try:
            with socket.create_connection((HOST, PORT), timeout=5) as sock:
                print("[SERIAL] conectado. Reinicie a simulacao Wokwi se nao aparecerem logs.")
                sock.settimeout(1)
                buffer = b""
                while True:
                    try:
                        chunk = sock.recv(1024)
                    except socket.timeout:
                        continue
                    if not chunk:
                        print("\n[SERIAL] conexao encerrada pelo Wokwi")
                        return 0
                    buffer += chunk
                    while b"\n" in buffer:
                        line, buffer = buffer.split(b"\n", 1)
                        print(line.decode("utf-8", errors="replace").rstrip())
        except (ConnectionRefusedError, TimeoutError, OSError) as error:
            print(f"[SERIAL] aguardando Wokwi expor serial: {error}")
            time.sleep(2)
        except KeyboardInterrupt:
            print("\n[SERIAL] encerrado")
            return 0


if __name__ == "__main__":
    sys.exit(main())
