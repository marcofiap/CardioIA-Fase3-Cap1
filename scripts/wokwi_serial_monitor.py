import sys
import time

import serial


URL = "rfc2217://127.0.0.1:4000"
BAUD_RATE = 115200


def main() -> int:
    print(f"[SERIAL] conectando em {URL} @ {BAUD_RATE} ...")

    while True:
        try:
            with serial.serial_for_url(URL, baudrate=BAUD_RATE, timeout=1) as ser:
                print("[SERIAL] conectado via RFC2217.")
                print("[SERIAL] pare e reinicie a simulacao Wokwi para ver o BOOT.")
                ser.reset_input_buffer()

                while True:
                    raw = ser.readline()
                    if raw:
                        print(raw.decode("utf-8", errors="replace").rstrip())

        except KeyboardInterrupt:
            print("\n[SERIAL] encerrado")
            return 0
        except Exception as error:
            print(f"[SERIAL] aguardando Wokwi expor serial: {error}")
            time.sleep(2)


if __name__ == "__main__":
    sys.exit(main())
