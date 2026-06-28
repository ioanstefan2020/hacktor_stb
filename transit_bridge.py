import asyncio
import logging
import random
import requests
import sys
from bleak import BleakScanner, BleakClient

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[logging.StreamHandler(sys.stdout)],
)

SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0"
CHARACTERISTIC_UUID = "12345678-1234-5678-1234-56789abcdef3"

DEVICE_NAME = "Hacktor_Watch"
UPDATE_INTERVAL_SECONDS = 30

STB_API_URL = "https://api.stb.ro/v1/live/station/1234"
STB_API_KEY = ""


def fetch_stb_data():
    try:
        headers = {"Authorization": f"Bearer {STB_API_KEY}"}
        response = requests.get(STB_API_URL, headers=headers, timeout=5)

        if response.status_code == 200:
            data = response.json()

            line = data["routes"][0]["id"]
            dest = data["routes"][0]["destination"]
            minutes = data["routes"][0]["estimated_arrival_minutes"]
            return f"{line},{dest},{minutes}"

        else:
            logging.warning(
                f"STB API returned {response.status_code}. Using local fallback data."
            )

    except requests.RequestException as e:
        logging.warning(f"HTTP Request failed: {e}. Using local fallback data.")

    routes = [
        {"line": "41", "dest": "Ghencea", "min_range": (1, 12)},
        {"line": "41", "dest": "Piata Presei", "min_range": (2, 10)},
        {"line": "335", "dest": "Complex Com.", "min_range": (3, 20)},
        {"line": "1", "dest": "Sura Mare", "min_range": (1, 15)},
        {"line": "10", "dest": "Eroii Revolut", "min_range": (4, 18)},
    ]

    selected = random.choice(routes)
    minutes = random.randint(*selected["min_range"])

    return f"{selected['line']},{selected['dest']},{minutes}"


async def main():
    logging.info("Starting STB Transit Bridge Daemon...")
    logging.info(f"Targeting Service: {SERVICE_UUID}")

    logging.info(f"Scanning for BLE device named '{DEVICE_NAME}'...")
    device = await BleakScanner.find_device_by_filter(
        lambda d, ad: d.name and DEVICE_NAME in d.name, timeout=15.0
    )

    if not device:
        logging.error(
            f"Could not find device '{DEVICE_NAME}'. Make sure the watch is on and advertising."
        )
        return

    logging.info(f"Found device! Address: {device.address}. Connecting...")

    async with BleakClient(device) as client:
        if client.is_connected:
            logging.info(f"Successfully paired with {DEVICE_NAME}!")
        else:
            logging.error("Failed to establish pairing connection.")
            return

        try:
            while True:
                payload_str = fetch_stb_data()
                payload_bytes = bytearray(payload_str, "utf-8")

                logging.info(f"Sending payload: '{payload_str}'")

                await client.write_gatt_char(
                    CHARACTERISTIC_UUID, payload_bytes, response=True
                )

                logging.info(f"Sleeping for {UPDATE_INTERVAL_SECONDS} seconds...")
                await asyncio.sleep(UPDATE_INTERVAL_SECONDS)

        except asyncio.CancelledError:
            logging.info("Bridge loop stopped.")
        except Exception as e:
            logging.error(f"Error during BLE communication: {e}")
        finally:
            logging.info("Disconnecting from Hacktor Watch.")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logging.info("\nTerminated manually via Ctrl+C.")
