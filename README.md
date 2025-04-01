# ControLab

![Version](https://img.shields.io/badge/version-1.0-blue.svg)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Windows%20%7C%20ESP32-orange.svg)

ControLab est un projet fait pour l'association CampusFab.  
Il a pour but de controller les adhérent en leurs donnant un badge qui permet de badger à l'entrée.  
Il verifie ensuite avec un serveur si le badge est correcte ou non.

## Installation

### Étapes d'installation

1. **Clonez le dépôt** :  
    Clonez le projet depuis GitHub en utilisant la commande suivante :
    ```bash
    git clone https://github.com/yourusername/ControLab.git
    ```

2. **Installez PlatformIO** :  
    Téléchargez et installez [PlatformIO Core](https://platformio.org/install) ainsi que l'IDE de votre choix (par exemple, VS Code avec l'extension PlatformIO).

3. **Modifiez le fichier de configuration de la carte** :  
    Accédez au fichier de configuration de la carte ESP32-C6-DevKitC-1 situé dans :  
    `C:\Users\<USER>\.platformio\platforms\espressif32@src-94456bc5430ff43c35a9ce9028efb1c4\boards\esp32-c6-devkitc-1.json`

    Remplacez son contenu par le suivant :
    ```json
    {
      "build": {
         "core": "esp32",
         "f_cpu": "160000000L",
         "f_flash": "80000000L",
         "flash_mode": "qio",
         "mcu": "esp32c6",
         "variant": "esp32c6"
      },
      "connectivity": [
         "wifi"
      ],
      "frameworks": [
         "arduino",
         "espidf"
      ],
      "name": "Espressif ESP32-C6-DevKitC-1",
      "upload": {
         "flash_size": "8MB",
         "maximum_ram_size": 524288,
         "maximum_size": 8388608,
         "require_upload_port": true,
         "speed": 460800
      },
      "url": "https://docs.espressif.com/projects/espressif-esp-dev-kits/en/latest/esp32c6/esp32-c6-devkitc-1/index.html",
      "vendor": "Espressif"
    }
    ```

    > **Note** : Assurez-vous de sauvegarder vos modifications avant de continuer.

4. **Compilez et téléversez** :  
    Utilisez PlatformIO pour compiler et téléverser le firmware sur votre carte ESP32-C6-DevKitC-1.

## Licence

Ce projet est sous licence [MIT License](LICENSE).

## Contact

Pour des questions n'hésitez pas [ludovicracot65@gmail.com](mailto:ludovicracot65@gmail.com).
