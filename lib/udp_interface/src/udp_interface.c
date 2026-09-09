/**
 * @file udp_interface.c
 * @brief UDP interface implementation.
 * @author Jonas Frei
 * @date 01.09.2026
 * @version 1.0
 */

#include "udp_interface.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>
#include <string.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <lwip/sockets.h>
#include <mdns.h>
#include <sequencer.h>

#define SAMPLES_PER_BATCH 20          /**< Number of samples per batch */
#define SEQUENCER_SAMPLES_PER_BATCH 5 /**< Number of sequencer steps per batch */

#define PROTO_MAGIC 0xA5              /**< Protocol identifier */
#define MSG_TYPE_HELLO 0x00           /**< Host -> ESP32: register as peer */
#define MSG_TYPE_TELEMETRY 0x01       /**< ESP32 -> Host: batched samples */
#define MSG_TYPE_CMD_SAFETY 0x02      /**< Host -> ESP32: set safety level */
#define MSG_TYPE_CMD_REMOVE_STEP 0x03 /**< Host -> ESP32: abort current step */
#define MSG_TYPE_CMD_WAIT 0x04        /**< Host -> ESP32: wait for duration */
#define MSG_TYPE_GOODBYE 0xFF         /**< Host -> ESP32: unregister as peer */

static const char *TAG = "UDP Interface"; /**< Tag for logging purposes */

static udp_interface_config_t cfg = {0}; /**< Configuration for the UDP interface */

static EventGroupHandle_t wifi_event_group; /**< UDP interface event group */
#define UDP_INTERFACE_INITIALIZED_BIT BIT0  /**< Bit indicating UDP interface is initialized in the event group */
#define WIFI_CONNECTED_BIT BIT1             /**< Bit indicating WiFi connection status in the event group */

static QueueHandle_t telemetry_safety_queue;               /**< Queue for storing telemetry safety samples */
static QueueHandle_t telemetry_control_queue;              /**< Queue for storing telemetry control samples */
static volatile uint32_t dropped_safety_sample_count = 0;  /**< Counter for dropped safety samples */
static volatile uint32_t dropped_control_sample_count = 0; /**< Counter for dropped control samples */

/**
 * @brief Telemetry Safety Sample Structure
 *
 * This structure represents a single telemetry safety sample sent over the wire.
 */
typedef struct __attribute__((packed))
{
    uint32_t timestamp_us; /**< System time in microseconds at sampling point */
    uint8_t safety_level;  /**< Current hierarchical safety level */
} telemetry_safety_sample_t;

/**
 * @brief Telemetry Control Sample Structure
 *
 * This structure represents a single telemetry control sample sent over the wire.
 */
typedef struct __attribute__((packed))
{
    uint32_t timestamp_us; /**< System time in microseconds at sampling point */
    float enc_A_pos;       /**< Encoder A position (rad) */
    float enc_B_pos;       /**< Encoder B position (rad) */
} telemetry_control_sample_t;

/**
 * @brief Telemetry Sequencer Sample Structure
 *
 * This structure represents a single telemetry sequencer sample sent over the wire.
 */
typedef struct __attribute__((packed))
{
    uint8_t type;         /**< Type of the sequencer step (MOVE_TO_POSE, WAIT, etc.) */
    uint8_t id;           /**< ID of the sequencer step */
    uint32_t duration_ms; /**< Duration for WAIT steps (ms) */
} telemetry_sequencer_sample_t;

/**
 * @brief Telemetry Packet Structure (UDP Datagram Wire Format)
 *
 * This structure defines the exact byte layout of the telemetry packet transmitted
 * from ESP32 to Host. The fixed size ensures predictable network payloads.
 */
typedef struct __attribute__((packed))
{
    uint8_t magic;                                                               /**< Protocol identifier (PROTO_MAGIC) */
    uint8_t msg_type;                                                            /**< Message type (MSG_TYPE_TELEMETRY) */
    uint16_t seq;                                                                /**< Packet sequence number for ordering/loss detection */
    uint8_t safety_sample_count;                                                 /**< Number of valid samples in safety_samples array */
    uint8_t control_sample_count;                                                /**< Number of valid samples in control_samples array */
    uint8_t sequencer_sample_count;                                              /**< Number of valid samples in sequencer_samples array */
    telemetry_safety_sample_t safety_samples[SAMPLES_PER_BATCH];                 /**< Array of safety samples */
    telemetry_control_sample_t control_samples[SAMPLES_PER_BATCH];               /**< Array of control samples */
    telemetry_sequencer_sample_t sequencer_samples[SEQUENCER_SAMPLES_PER_BATCH]; /**< Array of sequencer samples */
} telemetry_packet_t;

/**
 * @brief Command Header Structure
 *
 * This structure defines the wire-format header used to parse all incoming
 * command messages from the host.
 */
typedef struct __attribute__((packed))
{
    uint8_t magic;    /**< Protocol identifier (PROTO_MAGIC) */
    uint8_t msg_type; /**< Message type */
    uint16_t seq;     /**< Packet sequence number */
} cmd_header_t;

/**
 * @brief Safety Command Structure
 *
 * This structure represents a safety command message.
 */
typedef struct __attribute__((packed))
{
    cmd_header_t header;  /**< Command header */
    uint8_t safety_event; /**< Safety event to trigger */
} cmd_safety_t;

/**
 * @brief Wait Command Structure
 *
 * This structure represents a wait command message.
 */
typedef struct __attribute__((packed))
{
    cmd_header_t header;  /**< Command header */
    uint32_t duration_ms; /**< Duration to wait in milliseconds */
} cmd_wait_t;

/**
 * @brief Remove Sequencer Step Command Structure
 *
 * This structure represents a command to remove a sequencer step.
 */
typedef struct __attribute__((packed))
{
    cmd_header_t header; /**< Command header */
    uint8_t id;          /**< ID of the step to be removed */
} cmd_remove_sequencer_step_t;

/**
 * @brief WiFi Event Handler
 *
 * This function handles WiFi events and updates the connection status.
 *
 * @param arg Unused parameter for event handler.
 * @param event_base The base of the event (e.g., WIFI_EVENT, IP_EVENT).
 * @param event_id The specific event ID (e.g., WIFI_EVENT_STA_START, IP_EVENT_STA_GOT_IP).
 * @param event_data Pointer to event-specific data.
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGW(TAG, "WiFi disconnected, retrying...");
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t udp_interface_init(const udp_interface_config_t *config)
{
    ESP_LOGI(TAG, "Initializing UDP interface...");
    if (config == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Create event group if it doesn't exist
    if (wifi_event_group == NULL)
    {
        wifi_event_group = xEventGroupCreate();
        if (wifi_event_group == NULL)
        {
            ESP_LOGE(TAG, "Failed to create WiFi event group");
            return ESP_FAIL;
        }
    }
    else if (xEventGroupGetBits(wifi_event_group) & UDP_INTERFACE_INITIALIZED_BIT)
    {
        ESP_LOGW(TAG, "UDP interface already initialized.");
        return ESP_FAIL;
    }

    // Store the configuration
    cfg = *config;

    // Connect to WiFi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_netif_set_hostname(sta_netif, cfg.hostname));
    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set(cfg.hostname));

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {};
    memcpy(wifi_config.sta.ssid, cfg.wifi_ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, cfg.wifi_pass, sizeof(wifi_config.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to WiFi SSID '%s'...", cfg.wifi_ssid);
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    // Create Telemetry Queues for Safety and Control Samples
    telemetry_safety_queue = xQueueCreate(cfg.telemetry_queue_len, sizeof(telemetry_safety_sample_t));
    configASSERT(telemetry_safety_queue != NULL);
    telemetry_control_queue = xQueueCreate(cfg.telemetry_queue_len, sizeof(telemetry_control_sample_t));
    configASSERT(telemetry_control_queue != NULL);

    // Set the UDP interface initialized bit in the event group
    xEventGroupSetBits(wifi_event_group, UDP_INTERFACE_INITIALIZED_BIT);
    ESP_LOGI(TAG, "UDP interface initialized and WiFi connected.");
    return ESP_OK;
}

void udp_interface_add_safety_sample(safety_level_t safety_level)
{
    // Create a telemetry safety sample with the current timestamp and safety level
    telemetry_safety_sample_t sample = {
        .timestamp_us = (uint32_t)(esp_timer_get_time() & 0xFFFFFFFF),
        .safety_level = safety_level,
    };
    // Attempt to enqueue the sample; if the queue is full, increment the dropped sample counter
    if (xQueueSend(telemetry_safety_queue, &sample, 0) != pdTRUE)
    {
        dropped_safety_sample_count++;
    }
}

void udp_interface_add_control_sample(float enc_A_pos, float enc_B_pos)
{
    // Create a telemetry control sample with the current timestamp and control data
    telemetry_control_sample_t sample = {
        .timestamp_us = (uint32_t)(esp_timer_get_time() & 0xFFFFFFFF),
        .enc_A_pos = enc_A_pos,
        .enc_B_pos = enc_B_pos,
    };
    // Attempt to enqueue the sample; if the queue is full, increment the dropped sample counter
    if (xQueueSend(telemetry_control_queue, &sample, 0) != pdTRUE)
    {
        dropped_control_sample_count++;
    }
}

/**
 * @brief Handle incoming command messages.
 *
 * This function processes UDP datagrams and handles the following types:
 * - MSG_TYPE_HELLO: Registers/updates the telemetry recipient peer address.
 * - MSG_TYPE_GOODBYE: Unregisters the current telemetry peer.
 * - MSG_TYPE_CMD_SAFETY: Registers a safety event.
 *
 * @param buf Pointer to the received UDP datagram buffer.
 * @param len Length of the received datagram.
 * @param peer_addr Pointer to the current registered peer address (updated if HELLO received).
 * @param from_addr Pointer to the source address of the received datagram.
 * @param peer_known Pointer to a boolean indicating whether the peer is known.
 */
static void handle_command(const uint8_t *buf, int len, struct sockaddr_in *peer_addr, struct sockaddr_in *from_addr, bool *peer_known)
{
    if (len < (int)sizeof(cmd_header_t))
        return;

    const cmd_header_t *hdr = (const cmd_header_t *)buf;
    if (hdr->magic != PROTO_MAGIC)
        return;

    switch (hdr->msg_type)
    {
    case MSG_TYPE_HELLO:
        if (*peer_known)
        {
            /* A new HELLO unconditionally replaces the existing peer to avoid
               lockouts if a host crashes without sending GOODBYE. */
            ESP_LOGW(TAG, "Received HELLO from a new peer while already registered. Replacing the existing peer.");
        }
        *peer_addr = *from_addr;
        *peer_known = true;
        ESP_LOGI(TAG, "HELLO received — registered host with IP %s as telemetry peer", inet_ntoa(peer_addr->sin_addr));
        break;

    case MSG_TYPE_GOODBYE:
        ESP_LOGI(TAG, "GOODBYE received — unregistering host with IP %s as telemetry peer", inet_ntoa(peer_addr->sin_addr));
        *peer_known = false;
        break;

    case MSG_TYPE_CMD_SAFETY:
        if (len >= (int)sizeof(cmd_safety_t) && from_addr->sin_addr.s_addr == peer_addr->sin_addr.s_addr)
        {
            const cmd_safety_t *cmd = (const cmd_safety_t *)buf;
            safety_system_register_event(cmd->safety_event);
        }
        break;

    case MSG_TYPE_CMD_WAIT:
        if (len >= (int)sizeof(cmd_wait_t) && from_addr->sin_addr.s_addr == peer_addr->sin_addr.s_addr)
        {
            const cmd_wait_t *cmd = (const cmd_wait_t *)buf;
            sequencer_step_t step = {
                .type = WAIT,
                .wait = {
                    .duration_ms = cmd->duration_ms,
                },
            };
            sequencer_add_step_to_queue(&step);
        }
        break;

    case MSG_TYPE_CMD_REMOVE_STEP:
        if (len >= (int)sizeof(cmd_remove_sequencer_step_t) && from_addr->sin_addr.s_addr == peer_addr->sin_addr.s_addr)
        {
            const cmd_remove_sequencer_step_t *cmd = (const cmd_remove_sequencer_step_t *)buf;
            sequencer_remove_step_from_queue(cmd->id);
        }
        break;

    default:
        ESP_LOGW(TAG, "Unknown msg_type=%d", hdr->msg_type);
        break;
    }
}

void udp_interface_task(void *arg)
{
    // Register task with the safety system
    ESP_ERROR_CHECK(safety_system_register_task());

    // Wait for Network to be initialized and WiFi connection to be established
    xEventGroupWaitBits(wifi_event_group, (UDP_INTERFACE_INITIALIZED_BIT | WIFI_CONNECTED_BIT), pdFALSE, pdTRUE, portMAX_DELAY);

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        safety_system_unregister_task();
        vTaskDelete(NULL);
        return;
    }

    // Bind socket to the specified UDP port
    struct sockaddr_in listen_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(cfg.udp_port),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    if (bind(sock, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0)
    {
        ESP_LOGE(TAG, "Socket bind failed: errno %d", errno);
        closesocket(sock);
        safety_system_unregister_task();
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "UDP socket bound to port %d. Waiting for PC to send HELLO...", cfg.udp_port);

    struct sockaddr_in peer_addr;
    bool peer_known = false;
    uint16_t tx_seq = 0;

    static telemetry_packet_t packet; /* static: keeps it off the task stack */
    uint8_t rx_buf[256];

    TickType_t last_wake = xTaskGetTickCount();

    while (!safety_system_is_shutting_down())
    {
        /* 1. Non-blocking check for an incoming command / hello packet */
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        int len = recvfrom(sock, rx_buf, sizeof(rx_buf), MSG_DONTWAIT,
                           (struct sockaddr *)&from_addr, &from_len);
        if (len > 0)
        {
            handle_command(rx_buf, len, &peer_addr, &from_addr, &peer_known);
        }

        /* 2. Drain the telemetry queues into one batch and send it */
        uint8_t n = 0;
        telemetry_safety_sample_t sample;
        while (n < SAMPLES_PER_BATCH && xQueueReceive(telemetry_safety_queue, &sample, 0) == pdTRUE)
        {
            packet.safety_samples[n++] = sample;
        }
        /* Fill remaining array entries with zeros so the transmitted packet
           contains only clean data and no garbage from previous iterations. */
        memset(&packet.safety_samples[n], 0, (SAMPLES_PER_BATCH - n) * sizeof(telemetry_safety_sample_t));

        uint8_t m = 0;
        telemetry_control_sample_t control_sample;
        while (m < SAMPLES_PER_BATCH && xQueueReceive(telemetry_control_queue, &control_sample, 0) == pdTRUE)
        {
            packet.control_samples[m++] = control_sample;
        }
        /* Fill remaining array entries with zeros so the transmitted packet
           contains only clean data and no garbage from previous iterations. */
        memset(&packet.control_samples[m], 0, (SAMPLES_PER_BATCH - m) * sizeof(telemetry_control_sample_t));

        // Log dropped sample counts if any.
        // Note that the counters are approximate and may not be perfectly accurate due to
        // concurrency (non-atomic increments in producer tasks vs read/reset here).
        if (dropped_safety_sample_count > 0 || dropped_control_sample_count > 0)
        {
            ESP_LOGW(TAG, "Dropped %u safety samples and %u control samples since last report",
                     (unsigned int)dropped_safety_sample_count, (unsigned int)dropped_control_sample_count);
            dropped_safety_sample_count = 0;
            dropped_control_sample_count = 0;
        }

        /* 3. Get the first N sequencer steps in the queue and fill the packet */
        uint8_t seq_sample_count = 0;
        sequencer_step_t steps[SEQUENCER_SAMPLES_PER_BATCH] = {0};
        sequencer_get_first_N_steps_in_queue(SEQUENCER_SAMPLES_PER_BATCH, steps, &seq_sample_count);
        packet.sequencer_sample_count = seq_sample_count;
        for (uint8_t i = 0; i < seq_sample_count; i++)
        {
            packet.sequencer_samples[i].id = steps[i].id;
            packet.sequencer_samples[i].type = steps[i].type;
            if (packet.sequencer_samples[i].type == WAIT)
            {
                packet.sequencer_samples[i].duration_ms = steps[i].wait.duration_ms;
            }
        }
        /* Fill remaining array entries with zeros so the transmitted packet
           contains only clean data and no garbage from previous iterations. */
        memset(&packet.sequencer_samples[seq_sample_count], 0, (SEQUENCER_SAMPLES_PER_BATCH - seq_sample_count) * sizeof(telemetry_sequencer_sample_t));

        /* 4. Send the telemetry packet if a peer is known */
        if (peer_known)
        {
            packet.magic = PROTO_MAGIC;
            packet.msg_type = MSG_TYPE_TELEMETRY;
            packet.seq = tx_seq++;
            packet.safety_sample_count = n;
            packet.control_sample_count = m;

            int sent = sendto(sock, &packet, sizeof(telemetry_packet_t), 0,
                              (struct sockaddr *)&peer_addr, sizeof(peer_addr));
            if (sent < 0)
            {
                ESP_LOGE(TAG, "sendto failed: errno %d", errno);
            }
        }

        /* 5. Delay until the next cycle based on the configured task period */
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(cfg.task_period_ms));
    }

    // Cleanup and exit the task
    closesocket(sock);
    ESP_ERROR_CHECK(safety_system_unregister_task());
    ESP_LOGI(TAG, "Network task finished.");
    vTaskDelete(NULL);
}
