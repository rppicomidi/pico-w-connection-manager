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
#include <algorithm>
#include <cstring>
#include "pico_w_connection_manager.h"
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/assert.h"
rppicomidi::Pico_w_connection_manager::Pico_w_connection_manager() :
    country_code{CYW43_COUNTRY_WORLDWIDE}, state{DEINITIALIZED}, 
    scan_test{nil_time}, link_up_callback{nullptr,0},
    link_down_callback{nullptr,0},
    link_error_callback{nullptr,0},
    scan_complete_callback{nullptr, 0},
    settings_saved_state{UNKNOWN}
{
    countries.insert({CYW43_COUNTRY_WORLDWIDE, "Worldwide"});
    countries.insert({CYW43_COUNTRY_AUSTRALIA, "Australia"});
    countries.insert({CYW43_COUNTRY_BELGIUM, "Belgium"});
    countries.insert({CYW43_COUNTRY_BRAZIL, "Brazil"});
    countries.insert({CYW43_COUNTRY_CANADA, "Canada"});
    countries.insert({CYW43_COUNTRY_CHILE, "Chile"});
    countries.insert({CYW43_COUNTRY_CHINA, "China"});
    countries.insert({CYW43_COUNTRY_COLOMBIA, "Columbia"});
    countries.insert({CYW43_COUNTRY_CZECH_REPUBLIC, "Czech Republic"});
    countries.insert({CYW43_COUNTRY_DENMARK, "Denmark"});
    countries.insert({CYW43_COUNTRY_ESTONIA, "Estonia"});
    countries.insert({CYW43_COUNTRY_FINLAND, "Finland"});
    countries.insert({CYW43_COUNTRY_FRANCE, "France"});
    countries.insert({CYW43_COUNTRY_GERMANY, "Germany"});
    countries.insert({CYW43_COUNTRY_GREECE, "Greece" });
    countries.insert({CYW43_COUNTRY_HONG_KONG, "Honk Kong"});
    countries.insert({CYW43_COUNTRY_HUNGARY, "Hungary"});
    countries.insert({CYW43_COUNTRY_ICELAND, "Iceland"});
    countries.insert({CYW43_COUNTRY_INDIA, "India"});
    countries.insert({CYW43_COUNTRY_ISRAEL, "Israel"});
    countries.insert({CYW43_COUNTRY_ITALY, "Italy"});
    countries.insert({CYW43_COUNTRY_JAPAN, "Japan"});
    countries.insert({CYW43_COUNTRY_KENYA, "Kenya"});
    countries.insert({CYW43_COUNTRY_LATVIA, "Latvia" });
    countries.insert({CYW43_COUNTRY_LIECHTENSTEIN, "Liechtenstein"});
    countries.insert({CYW43_COUNTRY_LITHUANIA, "Lithuania"});
    countries.insert({CYW43_COUNTRY_LUXEMBOURG, "Luxembourg"});
    countries.insert({CYW43_COUNTRY_MALAYSIA, "Malaysia"});
    countries.insert({CYW43_COUNTRY_MALTA, "Malta"});
    countries.insert({CYW43_COUNTRY_MEXICO, "Mexico"});
    countries.insert({CYW43_COUNTRY_NETHERLANDS, "Netherlands"});
    countries.insert({CYW43_COUNTRY_NEW_ZEALAND, "New Zealand"});
    countries.insert({CYW43_COUNTRY_NIGERIA, "Nigeria"});
    countries.insert({CYW43_COUNTRY_NORWAY, "Norway"});
    countries.insert({CYW43_COUNTRY_PERU, "Peru"});
    countries.insert({CYW43_COUNTRY_PHILIPPINES, "Philippines"});
    countries.insert({CYW43_COUNTRY_POLAND, "Poland"});
    countries.insert({CYW43_COUNTRY_PORTUGAL, "Portugal"});
    countries.insert({CYW43_COUNTRY_SINGAPORE, "Singapore"});
    countries.insert({CYW43_COUNTRY_SLOVAKIA, "Slovakia"});
    countries.insert({CYW43_COUNTRY_SLOVENIA, "Slovenia"});
    countries.insert({CYW43_COUNTRY_SOUTH_AFRICA, "South Africa"});
    countries.insert({CYW43_COUNTRY_SOUTH_KOREA, "South Korea"});
    countries.insert({CYW43_COUNTRY_SPAIN, "Spain"});
    countries.insert({CYW43_COUNTRY_SWEDEN, "Sweden"});
    countries.insert({CYW43_COUNTRY_SWITZERLAND, "Switzerland"});
    countries.insert({CYW43_COUNTRY_TAIWAN, "Taiwan"});
    countries.insert({CYW43_COUNTRY_THAILAND, "Thailand"});
    countries.insert({CYW43_COUNTRY_TURKEY, "Turkey"});
    countries.insert({CYW43_COUNTRY_UK, "UK"});
    countries.insert({CYW43_COUNTRY_USA, "USA"});
    current_ssid.security = 0;
    // Attempt to load settings; if it fails, save defaults
    // It is important to have settings consistent with internal
    // data structures.
    if (!load_settings()) {
        assert(save_settings());
    }
}

void rppicomidi::Pico_w_connection_manager::Ssid_info::serialize(JSON_Object *ssid_object)
{
    json_object_set_string(ssid_object, "ssid", ssid.c_str());
    json_object_set_string(ssid_object, "pw", passphrase.c_str());
    json_object_set_number(ssid_object, "auth", security);
}

bool rppicomidi::Pico_w_connection_manager::Ssid_info::deserialize(JSON_Object* root_object)
{
    const char* ptr = json_object_get_string(root_object, "ssid");
    if (ptr != nullptr) {
        ssid = std::string(ptr);
        ptr = json_object_get_string(root_object, "pw");
        if (ptr != nullptr) {
            passphrase = std::string(ptr);
            JSON_Value* val = json_object_get_value(root_object, "auth");
            if (json_value_get_type(val) == JSONNumber) {
                security = json_value_get_number(val);
                return true;
            }
        }
    }
    return false;
}

void rppicomidi::Pico_w_connection_manager::get_country_code(std::string& code_)
{
    uint32_t icode = (state != DEINITIALIZED) ? cyw43_arch_get_country_code() : country_code;
    char code_str[3] = {static_cast<char>(icode & 0xff),
        static_cast<char>((icode >> 8) & 0xff),
        '\0'};
    code_ = std::string(code_str);
}

bool rppicomidi::Pico_w_connection_manager::get_country_from_code(const std::string& code_, std::string& country_)
{
    bool result = false;
    auto it = countries.find(CYW43_COUNTRY(code_.c_str()[0], code_.c_str()[1], 0));
    if (it != countries.end()) {
        country_ = it->second;
        result = true;
    }
    return result;
}

const char* rppicomidi::Pico_w_connection_manager::get_country_from_code(const std::string&code_)
{
    const char* result = nullptr;
    int32_t icode = CYW43_COUNTRY(code_.c_str()[0], code_.c_str()[1], 0);
    auto it = countries.find(icode);

    if (it != countries.end()) {
        result = it->second.c_str();
    }
    return result;
}

void rppicomidi::Pico_w_connection_manager::get_all_country_codes(std::vector<std::string>& all_codes_)
{
    char code[3]={'X', 'X', '\0'};
    for(auto& country: countries) {
        code[0] = country.first & 0xff;
        code[1] = (country.first >> 8) & 0xff;
        all_codes_.push_back(country.second + ":" + std::string(code));
    }
    std::sort(all_codes_.begin(), all_codes_.end());
}

bool rppicomidi::Pico_w_connection_manager::initialize()
{
    if (state == DEINITIALIZED) {
        if (cyw43_arch_init_with_country(country_code) == 0) {
            state = INITIALIZED;
            cyw43_arch_enable_sta_mode();
        }
    }
    return state != DEINITIALIZED;
}

bool rppicomidi::Pico_w_connection_manager::deinitialize()
{
    if (state != DEINITIALIZED) {
        cyw43_arch_deinit();
        state = DEINITIALIZED;
    }
    return true;
}

int rppicomidi::Pico_w_connection_manager::static_scan_result(void *env, const cyw43_ev_scan_result_t *result)
{
    auto me = reinterpret_cast<Pico_w_connection_manager*>(env);
    if (result) {
        for(auto it: me->discovered_ssids) {
            if (it.bssid[0] == result->bssid[0] && it.bssid[1] == result->bssid[1] && it.bssid[2] == result->bssid[2] &&
                it.bssid[3] == result->bssid[3] && it.bssid[4] == result->bssid[4] && it.bssid[5] == result->bssid[5])
                return 0; // already in the list
        }
        // otherwise, it's new. Add it to the list
        me->discovered_ssids.push_back(*result);
    }
    return 0;
}

bool rppicomidi::Pico_w_connection_manager::start_scan()
{
    if (state == SCAN_REQUESTED || state == SCANNING)
        return false;
    else if (state == DEINITIALIZED) {
        initialize();
    }
    else if (state == CONNECTED) {
        disconnect();
        deinitialize();
        initialize();
    }
    else if (state == CONNECTION_REQUESTED) {
        deinitialize();
        initialize();
    }
    // nothing to do for SCAN_COMPLETE or INITIALIZED
    discovered_ssids.clear();
    state = SCAN_REQUESTED;
    return true;
}

bool rppicomidi::Pico_w_connection_manager::set_country_code(const std::string& code_)
{
    bool result = false;
    if (code_.size() == 2) {
        char c0 = std::toupper(code_.c_str()[0]);
        char c1 = std::toupper(code_.c_str()[1]);
        int32_t icode = CYW43_COUNTRY(c0, c1, 0);
        auto it = countries.find(icode);
        if (it != countries.end()) {
            country_code = icode;
            result = true;
            printf("new country code %s=%s\r\n", code_.c_str(), it->second.c_str());
        }
        else {
            std::string oldcode;
            get_country_code(oldcode);
            printf("invalid country code %s; using previous code %s=%s\r\n", code_.c_str(),
                oldcode.c_str(), countries.at(country_code).c_str());
        }
    }
    return result;
}

bool rppicomidi::Pico_w_connection_manager::save_settings()
{
    int error_code = pico_mount(false);
    if (error_code != LFS_ERR_OK) {
        error_code = pico_mount(true);
        if (error_code != LFS_ERR_OK) {
            return false;
        }
    }
    lfs_dir_t dir;
    error_code = lfs_dir_open(&dir, wifi_info_dir);
    if (error_code == LFS_ERR_OK) {
        lfs_dir_close(&dir);
    }
    else if (error_code == LFS_ERR_NOENT) {
        error_code = lfs_mkdir(wifi_info_dir);
        if (error_code != LFS_ERR_OK) {
            pico_unmount();
            return false;
        }
    }
    else {
        pico_unmount();
        return false;
    }
    lfs_file_t file;
    error_code = lfs_file_open(&file, wifi_info_file, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if (error_code != LFS_ERR_OK) {
        pico_unmount();
        return false;
    }
    // file is open for writing.

    // Serialize the data to json
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    std::string code;
    get_country_code(code);
    json_object_set_string(root_object, "cc", code.c_str());
    JSON_Value *ssid_value = json_value_init_object();
    JSON_Object *ssid_object = json_value_get_object(ssid_value);

    current_ssid.serialize(ssid_object);
    json_object_set_value(root_object, "last_ssid", ssid_value);

    JSON_Value* known_array_value = json_value_init_array();
    JSON_Array* known_array = json_value_get_array(known_array_value);

    for (auto known: known_ssids) {
        ssid_value = json_value_init_object();
        ssid_object = json_value_get_object(ssid_value);
        known.serialize(ssid_object);
        json_array_append_value(known_array, ssid_value);
    }
    json_object_set_value(root_object, "known_ssids", known_array_value);
    json_set_float_serialization_format("%.0f");
    char* serialized_string = json_serialize_to_string(root_value);
    json_value_free(root_value);
    error_code = lfs_file_write(&file, serialized_string, strlen(serialized_string));
    bool result = true;
    if (error_code < 0) {
        result = false;
    }
    lfs_file_close(&file);
    pico_unmount();
    json_free_serialized_string(serialized_string);
    settings_saved_state = result ? SAVED:NOT_SAVED;
    return result;
}

bool rppicomidi::Pico_w_connection_manager::load_settings()
{
    int error_code = pico_mount(false);
    if (error_code != LFS_ERR_OK) {
        return false;
    }
    lfs_file_t file;
    error_code = lfs_file_open(&file, wifi_info_file, LFS_O_RDONLY);
    if (error_code != LFS_ERR_OK) {
        pico_unmount();
        return false;
    }
    auto sz = lfs_file_size(&file);
    if (sz < 0) {
        lfs_file_close(&file);
        pico_unmount();
        return false;
    }
    char* serialized_string = new char[sz+1];
    error_code = lfs_file_read(&file, serialized_string, sz);
    lfs_file_close(&file);
    pico_unmount();
    if (error_code <= 0) {
        delete[] serialized_string;
        return false;
    }
    serialized_string[sz] = '\0';
    JSON_Value* root_value = json_parse_string(serialized_string);
    delete[] serialized_string;
    bool result = false;
    if (root_value != nullptr) {
        JSON_Object* root_object = json_value_get_object(root_value);
        const char* val = json_object_get_string(root_object, "cc");
        if (val != nullptr) {
            if (set_country_code(std::string(val))) {
                JSON_Value* prev_ssid_value = json_object_get_value(root_object, "last_ssid");
                if (prev_ssid_value != nullptr) {
                    JSON_Object* prev_ssid_object = json_value_get_object(prev_ssid_value);
                    if (current_ssid.deserialize(prev_ssid_object)) {
                        // Now deserialize all known ssids
                        JSON_Value* known_ssids_value = json_object_get_value(root_object, "known_ssids");
                        if (known_ssids_value != nullptr) {
                            JSON_Array* known_array = json_value_get_array(known_ssids_value);
                            auto n_known = json_array_get_count(known_array);
                            size_t idx = 0;
                            known_ssids.clear();
                            for (; idx < n_known; idx++) {
                                Ssid_info info;
                                JSON_Object* item_object = json_array_get_object(known_array, idx);
                                if (item_object != nullptr) {
                                    if (info.deserialize(item_object)) {
                                        known_ssids.push_back(info);
                                    }
                                    else {
                                        break;
                                    }
                                }
                            }
                            result = idx == n_known;
                        }
                    }
                }
            }
        }
        json_value_free(root_value);
    }
    settings_saved_state = result ? SAVED:NOT_SAVED;
    return result;
}


void rppicomidi::Pico_w_connection_manager::set_current_passphrase(const std::string& pw)
{
    if (pw != current_ssid.passphrase) {
        current_ssid.passphrase = pw;
        settings_saved_state = NOT_SAVED;
    }
}


void rppicomidi::Pico_w_connection_manager::add_known_ssid(const Ssid_info& info)
{
    for (auto& known: known_ssids) {
        if (known.ssid == info.ssid) {
            settings_saved_state = (known.passphrase == info.passphrase && known.security == info.security) ? SAVED:NOT_SAVED;
            known.passphrase = info.passphrase;
            known.security = info.security;
            return;
        }
    }
    known_ssids.push_back(info);
    settings_saved_state = NOT_SAVED;
}

void rppicomidi::Pico_w_connection_manager::task()
{
    if (state != DEINITIALIZED) {
        if ((state == SCAN_REQUESTED || state == SCANNING) && absolute_time_diff_us(get_absolute_time(), scan_test) < 0) {
            last_link_error = "";
            if (state == SCAN_REQUESTED) {
                cyw43_wifi_scan_options_t scan_options;
                memset(&scan_options, 0, sizeof(scan_options));
                int err = cyw43_wifi_scan(&cyw43_state, &scan_options, this, static_scan_result);
                if (err == 0) {
                    printf("\nPerforming wifi scan\n");
                    state = SCANNING;
                } else {
                    printf("Failed to start scan: %d\n", err);
                    scan_test = make_timeout_time_ms(10000); // wait 10s and scan again
                }
            } 
            else if (!cyw43_wifi_scan_active(&cyw43_state)) {
                scan_test = make_timeout_time_ms(10000); // wait 10s before can scan again 
                state = is_link_up() ? CONNECTED : SCAN_COMPLETE;
                if (scan_complete_callback.cb != nullptr) {
                    scan_complete_callback.cb(scan_complete_callback.context);
                }
            }
        }
        if (state == SCAN_COMPLETE && is_link_up()) {
            state = CONNECTED;
            last_link_error = "";
        }
        else if (state == CONNECTION_REQUESTED || state == CONNECTED) {
            int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
            if (status < 0) {
                switch(status) {
                    case CYW43_LINK_BADAUTH:
                        last_link_error = "not authorized";
                        break;
                    case CYW43_LINK_NONET:
                        last_link_error = "cannot find SSID";
                        break;
                    case CYW43_LINK_FAIL:
                        last_link_error = "link failure";
                        break;
                    default:
                        last_link_error = "unknown error";
                        break;
                }
                printf("Connection error %s\r\n", last_link_error.c_str());
                // clear the error? I am not sure why I have to toggle Wi-Fi off and on
                deinitialize();
                initialize();
                if (link_error_callback.cb) {
                    link_error_callback.cb(link_error_callback.context, last_link_error.c_str());
                }
            }
            else if (status == CYW43_LINK_UP && state == CONNECTION_REQUESTED) {
                state = CONNECTED;
                last_link_error = "";
                if (link_up_callback.cb != nullptr) {
                    link_up_callback.cb(link_up_callback.context);
                }
                add_known_ssid(current_ssid);
                if (settings_saved_state != SAVED) {
                    save_settings();
                }
            }
            else if (status != CYW43_LINK_UP && state == CONNECTED) {
                if (status != CYW43_LINK_DOWN && status >= 0) {
                    state = CONNECTION_REQUESTED;
                    printf("Attempting to reconnect\r\n");
                }
                else {
                    state = INITIALIZED;
                }
                if (link_down_callback.cb != nullptr) {
                    link_down_callback.cb(link_down_callback.context);
                }
            }
        }
    }
}

bool rppicomidi::Pico_w_connection_manager::is_link_up()
{
    int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    return status == CYW43_LINK_UP;
}

bool rppicomidi::Pico_w_connection_manager::connect()
{
#if 0
    if (state == DEINITIALIZED) {
        printf("Wifi not initialized\r\n");
        return false;
    }
    if (is_scan_in_progress()) {
        printf("Cannot connect while scan is in progress\r\n");
        return false;
    }
#endif
    if (current_ssid.ssid.size() == 0) {
        printf("No SSID specified\r\n");
        return false;
    }
    else if (current_ssid.passphrase.size() == 0 && current_ssid.security != 0) {
        printf("No password specified\r\n");
        return false;
    }
    uint32_t auth = CYW43_AUTH_OPEN;
    if ((current_ssid.security & MIXED) == MIXED) {
        auth = CYW43_AUTH_WPA2_MIXED_PSK;
    }
    else if ((current_ssid.security & WPA2) == WPA2) {
        auth = CYW43_AUTH_WPA2_AES_PSK;
    }
    else if ((current_ssid.security & WPA) == WPA) {
        auth = CYW43_AUTH_WPA_TKIP_PSK;
    }
    const char* pw = (auth == CYW43_AUTH_OPEN) ? nullptr : current_ssid.passphrase.c_str();

    // Make sure the hardware will let us make a connection
    if (state == DEINITIALIZED) {
        initialize();
    }
    else if (state == CONNECTED) {
        disconnect();
        deinitialize();
        initialize();
    }
    else if (state == CONNECTION_REQUESTED ||
            state == SCANNING ||
            state == SCAN_REQUESTED) {
        deinitialize();
        initialize();
    }

    if (cyw43_arch_wifi_connect_async(current_ssid.ssid.c_str(), pw, auth) != 0) {
        return false;
    }
    else {
        state = CONNECTION_REQUESTED;
        last_link_error = "";
    }
    return true;
}

bool rppicomidi::Pico_w_connection_manager::disconnect()
{
    bool result = false;
    if (state == CONNECTED) {
        result = cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA) == 0;
    }
    else if (state == CONNECTION_REQUESTED) {
        // stop trying to reconnect
        if (deinitialize()) {
            result = initialize();
        }
    }

    return result;
}

uint32_t rppicomidi::Pico_w_connection_manager::get_ip_address()
{
    return is_link_up() ? cyw43_state.netif->ip_addr.addr : 0;
}

void rppicomidi::Pico_w_connection_manager::get_ip_address_string(std::string& addr_str)
{
    uint32_t addr = get_ip_address();
    addr_str =  std::to_string((addr >> 0) & 0xFF) + "." + 
                std::to_string((addr >> 8) & 0xFF) + "." +
                std::to_string((addr >> 16) & 0xFF) + "." +
                std::to_string((addr >> 24) & 0xFF);
}

int rppicomidi::Pico_w_connection_manager::get_rssi()
{
    // See https://forums.raspberrypi.com/viewtopic.php?t=341774
    int rssi = INT_MIN;
    if (state == CONNECTED) {
        // RSSI is only valid if the link is up
        if (cyw43_ioctl(&cyw43_state, 254, sizeof(rssi), (uint8_t *)&rssi, CYW43_ITF_STA) != 0) {
            rssi = INT_MIN;
        }
    }
    return rssi;
}

bool rppicomidi::Pico_w_connection_manager::autoconnect()
{
    if (state != DEINITIALIZED) {
        // de-initialize so can initialize with the correct country code
        if (!deinitialize())
            return false;
    }
    bool success = false;
    if (load_settings()) {
        std::string ssid;
        get_current_ssid(ssid);
        if (initialize()) {
            if (connect()) {
                printf("Requesting connection to %s\r\n", ssid.c_str());
                success = true;
            }
            else {
                printf("failed to connect to %s\r\n", ssid.c_str());
            }
        }
        else {
            printf("initialze failed\r\n");
        }
    }
    else {
        printf("load settings failed\r\n");
    }
    return success;
}

bool rppicomidi::Pico_w_connection_manager::erase_known_ssid_by_idx(size_t idx)
{
    bool success = false;
    if (idx < known_ssids.size()) {
        if (state == CONNECTED || state == CONNECTION_REQUESTED) {
            if (known_ssids[idx].ssid == current_ssid.ssid) {
                disconnect();
                current_ssid.ssid.clear();
                current_ssid.passphrase.clear();
                current_ssid.security = 0;
            }
        }
        known_ssids.erase(known_ssids.begin() + idx);
        success = save_settings();
    }
    return success;
}
