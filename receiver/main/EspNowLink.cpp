#include "EspNowLink.h"

#include <esp_event.h>
#include <esp_mac.h>
#include <esp_netif.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <string.h>

#include "Config.h"
#include "Link.h"

namespace espnow {
namespace {

bool hasController = false;
bool hasRobot = false;

// Set by the callback (Wi-Fi task), printed by report()
volatile bool unknownSenderHeard = false;
uint8_t unknownSender[6] = {};
volatile int badLength = -1;

bool isSet(const uint8_t mac[6]) {
    for (int i = 0; i < 6; i++) {
        if (mac[i] != 0) return true;
    }
    return false;
}

bool addPeer(const uint8_t mac[6]) {
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, mac, 6);
    peer.channel = 0;  // the current channel
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;
    return esp_now_add_peer(&peer) == ESP_OK;
}

void onReceive(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
    const uint8_t* mac = info->src_addr;

    if (hasController && memcmp(mac, config::CONTROLLER_MAC, 6) == 0) {
        if (len != sizeof(ControlPacket) || data[0] != HEXAPOD_PROTOCOL_VERSION) {
            badLength = len;
            return;
        }
        ControlPacket packet;
        memcpy(&packet, data, sizeof(packet));
        link::controllerInput(packet);
        const HexPacket reply = link::status();
        esp_now_send(config::CONTROLLER_MAC, reinterpret_cast<const uint8_t*>(&reply), sizeof(reply));
        return;
    }

    if (hasRobot && memcmp(mac, config::ROBOT_MAC, 6) == 0) {
        // The robot's reply to a relayed ControlPacket
        if (len == sizeof(HexPacket) && data[0] == HEXAPOD_PROTOCOL_VERSION) {
            HexPacket status;
            memcpy(&status, data, sizeof(status));
            link::setStatus(status);
        } else {
            badLength = len;
        }
        return;
    }

    memcpy(unknownSender, mac, 6);
    unknownSenderHeard = true;
}

}  // namespace

bool begin() {
    hasController = isSet(config::CONTROLLER_MAC);
    hasRobot = isSet(config::ROBOT_MAC);

    // Wi-Fi in station mode, not connected to anything: ESP-NOW only
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK || esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK ||
        esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK || esp_wifi_start() != ESP_OK) {
        printf("Error starting Wi-Fi for ESP-NOW\n");
        return false;
    }

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    printf("Receiver MAC (the controller's receiverMAC, or the robot's CONTROLLER_MAC): "
           "%02X:%02X:%02X:%02X:%02X:%02X\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if (esp_now_init() != ESP_OK) {
        printf("Error initializing ESP-NOW\n");
        return false;
    }
    esp_now_register_recv_cb(onReceive);

    bool ok = true;
    if (hasController && !addPeer(config::CONTROLLER_MAC)) {
        printf("Failed to add the controller as an ESP-NOW peer\n");
        ok = false;
    }
    if (hasRobot && !addPeer(config::ROBOT_MAC)) {
        printf("Failed to add the robot as an ESP-NOW peer\n");
        ok = false;
    }
    printf("ESP-NOW: handheld controller %s, relay to the robot %s\n", hasController ? "on" : "off",
           hasRobot ? "on" : "off");
    return ok;
}

void relay() {
    if (!hasRobot) return;
    const ControlPacket packet = link::control();
    esp_now_send(config::ROBOT_MAC, reinterpret_cast<const uint8_t*>(&packet), sizeof(packet));
}

bool relaying() {
    return hasRobot;
}

void report() {
    if (unknownSenderHeard) {
        unknownSenderHeard = false;
        printf("Ignored a packet from %02X:%02X:%02X:%02X:%02X:%02X (not CONTROLLER_MAC or ROBOT_MAC in Config.h)\n",
               unknownSender[0], unknownSender[1], unknownSender[2], unknownSender[3], unknownSender[4],
               unknownSender[5]);
    }
    const int len = badLength;
    if (len >= 0) {
        badLength = -1;
        printf("Ignored a %d-byte ESP-NOW packet (expected %u from the controller, %u from the robot, "
               "version %d): is hexapod-gait-core the same version at both ends?\n",
               len, (unsigned)sizeof(ControlPacket), (unsigned)sizeof(HexPacket), HEXAPOD_PROTOCOL_VERSION);
    }
}

}  // namespace espnow
