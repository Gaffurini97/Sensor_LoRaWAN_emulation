import os
import pandas as pd
import json

def generate_device_config_json(index, device_info, output_folder, output_cfg_folder):
    config_data = {
        "initial_sleep_in_sec": 0,
        "communication_type": 0,
        "lorawan_info": {
            "devEUI": device_info["dev_eui"].upper(),
            "joinEUI": device_info["dev_eui"].upper(),  # Utilizziamo devEUI come joinEUI per esempio
            "appKey": device_info["application_key"],
            "nodeName": f"eui-{device_info['dev_eui']}",
            "last_dev_nonce": 300
        },
        "mqtt_communication": {
            "host_name": "ie-databus",
            "port": 1883,
            "client_id": f"userendnode_{device_info['dev_eui']}",
            "username": "usren",
            "password": "usren"
        },
        "actions_mapping": [
            {"action": "RUS", "topic": r"\/Radio\/Uplink\/"},
            {"action": "RDR", "topic": "\/Radio\/Downlink\/"},
            {"action": "JR", "topic": "\/JoinReq"},
            {"action": "JA", "topic": "\/JoinAcpt"},
            {"action": "UPn", "topic": "\/Uplink"},
            {"action": "DWn", "topic": "\/Downlink"},
            {"action": "disUPn", "topic": "\/DUplink\/"},
            {"action": "disDWn", "topic": f"\/DDownlink\/"+str(index)},
            {"action": "disbALT", "topic": "\/DbALT"}
        ],
        "node": {
            "serial": {
                "port": "\/dev\/ttyACM0",  # Imposta la porta seriale desiderata
                "baudrate": 57600,
                "bytesize": 8,
                "parity": "N",
                "stopbits": 1,
                "timeout": 5
            },
            "radio": {
                "params": [
                    {"name": "bt", "value": "none"},
                    {"name": "mod", "value": "lora"},
                    {"name": "freq", "value": "868000000"},
                    {"name": "pwr", "value": 4},
                    {"name": "sf", "value": "sf7"},
                    {"name": "afcbw", "value": 125},
                    {"name": "rxbw", "value": 250},
                    {"name": "bitrate", "value": 5000},
                    {"name": "fdev", "value": 5000},
                    {"name": "prlen", "value": 8},
                    {"name": "crc", "value": "on"},
                    {"name": "iqi", "value": "off"},
                    {"name": "cr", "value": r"4\/5"},
                    {"name": "wdt", "value": 2000},
                    {"name": "sync", "value": "34"},
                    {"name": "bw", "value": 125}
                ],
                "default_tx_port": 1,
                "rx_windows_size": 0
            },
            "wait-after-tx-in-s": 5,
            "default-confirmed": False
        }
    }

    
    # Crea la cartella se non esiste
    os.makedirs(output_folder+"/"+output_cfg_folder+str(index+1), exist_ok=True)

    # Crea un nome di file unico basato sull'ID del dispositivo
    filename = os.path.join(output_folder+"/"+output_cfg_folder+str(index+1), f"config.json")

    # Scrivi le informazioni nel file JSON
    with open(filename, 'w') as json_file:
        #write file with indent=2
        json_file.write(json.dumps(config_data, indent=4).replace('\\\\','\\'))
        


        #json.dump(config_data, json_file, indent=2)
        ##subsitute \\ to \ in the filename
        #json_file.seek(0)
        #json_file.write(json_file.read().replace('\\\\','\\'))

# Leggi il file Excel
excel_filename = "device_info.xlsx"
df = pd.read_excel(excel_filename)

# Cartella di output per i file JSON
output_folder = "device_configs"
output_cfg_folder = "cfg-data"

# Crea la cartella se non esiste
os.makedirs(output_folder, exist_ok=True)

# Genera un file JSON per ogni dispositivo
for index, row in df.iterrows():
    generate_device_config_json(index, row, output_folder, output_cfg_folder)

print(f"I file JSON sono stati generati nella cartella '{output_folder}'.")
