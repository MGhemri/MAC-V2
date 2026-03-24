# Multi-Application Controller (MAC-V2) - ESP32 Firmware

## 📋 Présentation
Le **MAC-V2** est un contrôleur industriel basé sur un ESP32 permettant de piloter **8 relais électriques**. Il dispose d'une interface physique (LCD + Encodeur) et d'une API complète (REST + WebSockets) pour une intégration transparente avec une application mobile Capacitor.

---

## 🛠 Spécifications Matérielles (Pinout)
- **Encodeur KY-040** : CLK (16), DT (17), SW (18).
- **Écran LCD I2C (20x4)** : SDA (21), SCL (22). Adresse : `0x27`.
- **Relais (1-8)** : GPIOs `2, 4, 5, 12, 13, 14, 15, 27`.

---

## 📡 Connectivité & Découverte (Provisioning)

### 1. Mode Point d'Accès (Provisioning)
Si aucun réseau Wi-Fi n'est configuré ou si la connexion échoue, l'ESP32 crée un point d'accès ouvert :
- **SSID** : `MAC-V2-CONFIG`
- **IP AP** : `192.168.4.1`

### 2. Découverte Automatique (UDP & mDNS)
L'application Capacitor peut retrouver l'ESP32 sur le réseau local via deux méthodes :
- **mDNS** : L'appareil répond à l'adresse `http://mac-controller.local`.
- **UDP Discovery** :
  - **Port** : `4210`
  - **Message à envoyer** : `DISCOVER_MAC_V2`
  - **Réponse reçue** : `MAC_V2_ALIVE` (L'adresse IP de réponse est celle de l'émetteur du paquet UDP).

---

## 🔗 API REST (Endpoints)
Toutes les réponses incluent les headers **CORS** (`Access-Control-Allow-Origin: *`).

### Système
- **GET `/api/status`** : Retourne l'état complet (relais, modes, noms, config système).
- **POST `/api/system/lcd`** : Allume/Éteint le rétroéclairage.
  - *Payload* : `{ "enabled": true/false }`
- **POST `/api/wifi/setup`** : Configure le Wi-Fi et redémarre l'appareil.
  - *Payload* : `{ "ssid": "MonWiFi", "pass": "MonMotDePasse" }`

### Relais
- **POST `/api/relay/{id}/toggle`** : Inverse l'état du relais (ID 1 à 8).
- **PUT `/api/relay/{id}/config`** : Modifie le mode et le nom personnalisé.
  - *Payload* : `{ "mode": "custom", "custom_name": "Piscine" }`
  - *Contrainte* : `custom_name` max 8 caractères.

---

## 🔄 WebSockets (Temps Réel)
Connectez-vous à `ws://{IP_ESP32}/ws`.
L'ESP32 pousse un événement à chaque changement d'état (physique ou via API) :
```json
{
  "event": "relay_update",
  "id": 1,
  "state": 1
}
```

---

## 🖥️ Interface Physique (LCD)
### Acknowledgement LCD
Après une configuration Wi-Fi réussie, l'ESP32 affiche son adresse IP sur l'écran. Un **clic sur l'encodeur** est nécessaire pour fermer cet écran et revenir au fonctionnement normal.

---

## ⚙️ Architecture Logicielle (FreeRTOS)
Le firmware est conçu pour être stable et non-bloquant :
- **Task_Input** (Prio 4) : Anti-rebond (debounce) logiciel et gestion de l'encodeur.
- **Task_LCD** (Prio 3) : Gestion de l'affichage et du menu physique.
- **Task_Relay** (Prio 3) : Commande GPIO et rafraîchissement d'état.
- **Task_API** (Prio 2) : Serveur HTTP, WebSockets et Discovery UDP.

---

## 🚀 Installation (PlatformIO)
1. Installez PlatformIO.
2. Clonez ce dépôt.
3. Exécutez `pio run` pour compiler.
4. Exécutez `pio run -t upload` pour flasher l'ESP32.
