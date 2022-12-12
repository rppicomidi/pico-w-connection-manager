/**
 * MIT License
 *
 * Copyright (c) 2022 rppicomidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */
#pragma once
#include <string>
#include <vector>
#include <map>
#include "pico/cyw43_arch.h"
#include "pico_hal.h"
#include "parson.h"

namespace rppicomidi
{
class Pico_w_connection_manager
{
public:

    /**
     * @brief The possible states of the Wi-Fi system
     * 
     */
    enum Wifi_state {
        DEINITIALIZED,  //!< initial state of the Wi-Fi hardware
        INITIALIZED,    //!< Wi-Fi radio is ready to be accessed
        SCAN_REQUESTED, //!< A scan for available SSIDs has been issued
        SCANNING,       //!< A Wi-Fi scan is in progress
        SCAN_COMPLETE,  //!< A Wi-Fi scan is complete
        CONNECTION_REQUESTED,   //!< the Wi-Fi radio is attempting to connect to the current_ssid
        CONNECTED,      //!< The Wi-Fi radio has connected to the current_ssid and an IP address has been assigned
    };

    enum Settings_saved_state {
        UNKNOWN,
        NOT_SAVED,
        SAVED
    };
    /**
     * @brief Describes the information required to connected an SSID
     * 
     */
    struct Ssid_info {
        std::string ssid; //!< The SSID name
        std::string passphrase; //!< The password or passphrase; may be empty if security is 0
        int security; //!< 
        /**
         * @brief Serialize the fields in this struct to the the given root_object
         *
         * @param root_object the object to receive the value
         */
        void serialize(JSON_Object* root_object);

        /**
         * @brief Deserialize from the root_object the 3 fields
         * in this struct
         *
         * @param root_object the object containing the values
         * @return true if deserialization is successful, false otherwise
         */
        bool deserialize(JSON_Object* root_object);
    };

    static const int OPEN=0;                //!< security will be 0 if the SSID requires no passphrase
    static const int WEP=1;                 //!< scan ORs this value to security if SSID supports WEP; not supported
    static const int WPA=2;                 //!< scan ORs this value to security if SSID supports WPA-PSK
    static const int WPA2=4;                //!< scan ORs this value to security if SSID supports WPA2-PSK
    static const int MIXED = (WPA | WPA2);  //!< scan ORs this value to security if SSID supports both WPA & WPA2

    Pico_w_connection_manager(Pico_w_connection_manager const&) = delete;
    void operator=(Pico_w_connection_manager const&) = delete;
    ~Pico_w_connection_manager()=default;
    /**
     * @brief Construct a new Pico_w_connection_manager object
     * 
     */
    Pico_w_connection_manager();

    /**
     * @brief Initialize the Wi-Fi hardware
     * 
     * @return true if successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Disconnect and shut down the Wi-Fi hardware
     * 
     * @return true if successful, false otherwise
     */
    bool deinitialize();

    /**
     * @brief Set the Wi-Fi radio country code 
     * 
     * @param code_ the 2-letter country code
     * @return true if successful, false otherwise
     */
    bool set_country_code(const std::string& code_);

    /**
     * @brief set code_ to contain the 2-letter country code
     * 
     * @param code_ receives the 2-letter country code
     */
    void get_country_code(std::string& code_);

    /**
     * @brief Get the current state of the Wi-Fi system
     * 
     * @return wifi_state the current Wi-Fi system state
     */
    Wifi_state get_state() {return state; }

    /**
     * @brief set the codes_ vector to contain a list of all supported countries
     * and correspoinding 2-letter codes
     *
     * @param codes_ is a vector of entries of the format country:2-letter
     * code sorted by country.
     */
    void get_all_country_codes(std::vector<std::string>& codes_);

    /**
     * @brief set country string to the country name that corresponds
     * to the given 2-letter code.
     * 
     * @param code the 2-letter country code
     * @param country the corresponding country
     * @return true if the code was valid, false if not
     */
    bool get_country_from_code(const std::string&code, std::string& country);

    /**
     * @brief save the current country code, last SSID for which a connection
     * attempt was made (with corresponding security configuration and password),
     * and a list of all previously connected SSIDs and security information.
     *
     * Data is stored in JSON format to the LittleFS file system
     * @return true if save is successful, false otherwise
     */
    bool save_settings();

    /**
     * @brief recall all previously saved settings
     * 
     * @return true if successful, false otherwise
     */
    bool load_settings();

    /**
     * @brief populate the discovered_ssids vector with all
     * detected SSIDs
     *
     * @return true if scan successfully started; false otherwise
     * @note discovered_ssids is not fully populated until is_scan_in_progress returns false
     */
    bool start_scan();

    /**
     * @brief 
     * 
     * @return true 
     * @return false 
     */
    bool is_scan_in_progress() const {return state == SCANNING; }

    /**
     * @brief Request a connection to the SSID stored in current_ssid
     *
     * Use the passphrase and security information stored in current_ssid
     * if required. 
     *
     * @return true if connection request was successful, false otherwise
     * @note connection is not complete until is_link_up() returns true
     */
    bool connect();

    /**
     * @brief disconnect a link-up connection, or stop trying to reconnect
     * if the link is down.
     *
     * @return true if successful, false otherwise
     */
    bool disconnect();

    /**
     * @brief return true if the Wi-Fi radio is connected to the current_ssid
     * and the Wi-Fi connection has an IP address
     * 
     * @return true if the link is fully up, false otherwise
     */
    bool is_link_up();

    /**
     * @brief update the Wi-Fi state based on the current Wi-Fi hardware status
     *
     * You must call this function periodically in your main "super-loop"
     */
    void task();

    /**
     * @brief Return a pointer to the list of discovered SSIDs
     * 
     * @return const std::vector<cyw43_ev_scan_result_t>* 
     */
    const std::vector<cyw43_ev_scan_result_t>* get_discovered_ssids() {return &discovered_ssids; }

    /**
     * @brief Get the SSID of the AP to which the Wi-Fi has last attempted to connect
     * 
     * @param ssid_ contains the SSID on exit
     */
    void get_current_ssid(std::string& ssid_) { ssid_ = current_ssid.ssid; }

    /**
     * @brief Get the stored passphrase for the SSID returned by get_current_ssid()
     * 
     * @param pw_ contains the passphrase
     */
    void get_current_passphrase(std::string& pw_) { pw_ = current_ssid.ssid; }

    /**
     * @brief Get the current security object
     * 
     * @return int 
     */
    int get_current_security() {return current_ssid.security; }

    /**
     * @brief Set the SSID for the next connection attempt
     * 
     * @param ssid contains the SSID to which you would like to connect next
     */
    void set_current_ssid(const std::string& ssid)
    {
        if (ssid != current_ssid.ssid) {
            current_ssid.ssid = ssid;
            settings_saved_state = NOT_SAVED;
        }
    }

    /**
     * @brief Set the passphrase for the SSID set by set_current_ssid()
     * 
     * @param pw 
     */
    void set_current_passphrase(const std::string& pw)
    {
        if (pw != current_ssid.passphrase) {
            current_ssid.passphrase = pw;
            settings_saved_state = NOT_SAVED;
        }
    }

    /**
     * @brief Set the security value for the SSID set by set_current_ssid()
     * 
     * @param auth one of OPEN, WPA, WPA2, MIXED
     * @note WEP is not supported
     */
    void set_current_security(int auth)
    {
        if (current_ssid.security != auth) {
            current_ssid.security = auth;
            settings_saved_state = NOT_SAVED;
        }
    }

    /**
     * @brief Get the ip address if the link is up or 0 if it is not
     * 
     * @return uint32_t The IPv4 address packed into a 32-bit word, first
     * octet is in the lower 8 bits; 4th octet is in the upper 8 bits.
     */
    uint32_t get_ip_address();

    /**
     * @brief Get the ip address as a dot notation string, e.g. 192.168.1.2
     * 
     * @param addr will contain the IPv4 address in dot notation; will be
     * 0.0.0.0 if the link is not up
     */
    void get_ip_address_string(std::string& addr);

    /**
     * @brief Get the RSSI for the currently connected AP
     *
     * @return int the RSSI of the connection or INT_MIN if the link is not up
     * or the RSSI read request fails.
     */
    int get_rssi();

    /**
     * @brief register the callback function to call if the link is up
     *
     * To unregister the callback, call this function again with cb==nullptr
     * @param cb 
     * @param context 
     */
    void register_link_up_callback(void (*cb)(void*), void* context)
    {
        link_up_callback.cb = cb; link_up_callback.context = context;
    }

    /**
     * @brief register the callback function to call if the link is down
     *
     * To unregister the callback, call this function again with cb==nullptr
     * @param cb 
     * @param context 
     */
    void register_link_down_callback(void (*cb)(void*), void* context)
    {
        link_down_callback.cb = cb; link_down_callback.context = context;
    }

    /**
     * @brief register the callback function to call when scan completes
     *
     * To unregister the callback, call this function again with cb==nullptr
     * @param cb 
     * @param context 
     */
    void register_scan_complete_callback(void (*cb)(void*), void* context)
    {
        scan_complete_callback.cb = cb; scan_complete_callback.context = context;
    }

    /**
     * @brief Load settings, Initialize Wi-Fi with stored country code, and request
     * to connect to the last connected AP.
     *
     * @return true if successful, false otherwise
     */
    bool autoconnect();

    /**
     * @brief Get the known SSIDs vector
     * 
     * @return const std::vector<Ssid_info>& the known SSIDs
     */
    const std::vector<Ssid_info>& get_known_ssids() {return known_ssids; }

    /**
     * @brief delete item idx from the known SSIDs vector; store the settings in flash
     *
     * @note if connected to the SSID to be erased, will also disconnecte and clear current_ssid.
     * @param idx the index of the known SSID record to erase
     * @return true if idx is in range and the erase was successful, false otherwise
     */
    bool erase_known_ssid_by_idx(size_t idx);

    Settings_saved_state get_settings_saved_state() { return settings_saved_state; }
private:
    struct wifi_callback {
        void (*cb)(void*);
        void* context;
    };
    static int static_scan_result(void *env, const cyw43_ev_scan_result_t *result);
    
    void add_known_ssid(const Ssid_info& info);
    uint32_t country_code;
    std::map<uint32_t, std::string> countries;
    Wifi_state state;
    Ssid_info current_ssid;
    std::vector<Ssid_info> known_ssids;
    absolute_time_t scan_test;
    std::vector<cyw43_ev_scan_result_t> discovered_ssids;
    wifi_callback link_up_callback;
    wifi_callback link_down_callback;
    wifi_callback scan_complete_callback;
    Settings_saved_state settings_saved_state;
    static constexpr const char* wifi_info_dir{"/wifi_info"};
    static constexpr const char* wifi_info_file{"/wifi_info/wifi_info.json"};
};
}