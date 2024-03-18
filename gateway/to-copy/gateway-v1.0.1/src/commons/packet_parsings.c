#include "../../headers/commons/packet_parsings.h"

char* ngw_rx_to_json(struct lgw_pkt_rx_s *obj){
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_number(root_object, "freq_hz", obj->freq_hz);
    json_object_set_number(root_object, "if_chain", obj->if_chain);
    json_object_set_number(root_object, "status", obj->status);
    json_object_set_number(root_object, "count_us", obj->count_us);
    json_object_set_number(root_object, "rf_chain", obj->rf_chain);
    json_object_set_number(root_object, "modulation", obj->modulation);
    json_object_set_number(root_object, "bandwidth", obj->bandwidth);
    json_object_set_number(root_object, "datarate", obj->datarate);
    json_object_set_number(root_object, "coderate", obj->coderate);
    json_object_set_number(root_object, "rssi", obj->rssi);
    json_object_set_number(root_object, "snr", obj->snr);
    json_object_set_number(root_object, "snr_min", obj->snr_min);
    json_object_set_number(root_object, "snr_max", obj->snr_max);
    json_object_set_number(root_object, "crc", obj->crc);
    json_object_set_number(root_object, "size", obj->size);
    int new_size = 256*2;
    char payload_str[new_size];
    int new_char_size = bin_to_b64(obj->payload, obj->size, payload_str, new_size);
    json_object_set_string_with_len(root_object, "payload", payload_str, new_char_size);

    char* serialized_string = json_serialize_to_string(root_value);
    json_value_free(root_value);
    return serialized_string;
}

struct lgw_pkt_rx_s *ngw_json_to_rx(char* json){
    struct lgw_pkt_rx_s *obj = malloc(sizeof(struct lgw_pkt_rx_s));
    JSON_Value *root_val = json_parse_string(json);
    JSON_Object *root_obj = json_value_get_object(root_val);
    obj->freq_hz = (float)json_object_dotget_number(root_obj,"freq_hz");
    obj->if_chain = (uint8_t)json_object_dotget_number(root_obj,"if_chain");
    obj->status = (uint8_t)json_object_dotget_number(root_obj,"status");
    obj->count_us = (uint32_t)json_object_dotget_number(root_obj,"count_us");
    obj->rf_chain = (uint8_t)json_object_dotget_number(root_obj,"rf_chain");
    obj->modulation = (uint8_t)json_object_dotget_number(root_obj,"modulation");
    obj->bandwidth = (uint8_t)json_object_dotget_number(root_obj,"bandwidth");
    obj->datarate = (uint32_t)json_object_dotget_number(root_obj,"datarate");
    obj->coderate = (uint8_t)json_object_dotget_number(root_obj,"coderate");
    obj->rssi = (float)json_object_dotget_number(root_obj,"rssi");
    obj->snr = (float)json_object_dotget_number(root_obj,"snr");
    obj->snr_min = (float)json_object_dotget_number(root_obj,"snr_min");
    obj->snr_max = (float)json_object_dotget_number(root_obj,"snr_max");
    obj->crc = (uint16_t)json_object_dotget_number(root_obj,"crc");
    obj->size = (uint16_t)json_object_dotget_number(root_obj,"size");
    const char* payload_ch = json_object_dotget_string(root_obj,"payload");
    int size_payload = strlen(payload_ch);
    if(size_payload > 256){
        printf("PAYLOAD TOO LONG\n");
        fflush(stdout);
        obj->size = 0;
        json_value_free(root_val);
        return obj;
    }
    b64_to_bin(payload_ch, strlen(payload_ch), obj->payload, sizeof obj->payload);
    json_value_free(root_val);
    return obj;
}

char* ngw_tx_to_json(struct lgw_pkt_tx_s *obj){
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_number(root_object, "freq_hz", obj->freq_hz);
    json_object_set_number(root_object, "tx_mode", obj->tx_mode);
    json_object_set_number(root_object, "count_us", obj->count_us);
    json_object_set_number(root_object, "rf_chain", obj->rf_chain);
    json_object_set_number(root_object, "rf_power", obj->rf_power);
    json_object_set_number(root_object, "modulation", obj->modulation);
    json_object_set_number(root_object, "bandwidth", obj->bandwidth);
    json_object_set_number(root_object, "datarate", obj->datarate);
    json_object_set_number(root_object, "coderate", obj->coderate);
    json_object_set_number(root_object, "invert_pol", obj->invert_pol);
    json_object_set_number(root_object, "f_dev", obj->f_dev);
    json_object_set_number(root_object, "preamble", obj->preamble);
    json_object_set_number(root_object, "no_crc", obj->no_crc);
    json_object_set_number(root_object, "no_header", obj->no_header);
    json_object_set_number(root_object, "size", obj->size);
    int new_size = 256*2;
    char payload_str[new_size];
    int new_char_size = bin_to_b64(obj->payload, obj->size, payload_str, new_size);
    json_object_set_string_with_len(root_object, "payload", payload_str, new_char_size);
    char* serialized_string = json_serialize_to_string(root_value);
    json_value_free(root_value);
    return serialized_string;
}

struct lgw_pkt_tx_s *ngw_json_to_tx(char* json){
    struct lgw_pkt_tx_s *obj = malloc(sizeof(struct lgw_pkt_tx_s));
    JSON_Value *root_val = json_parse_string(json);
    JSON_Object *root_obj = json_value_get_object(root_val);
    obj->freq_hz = (float)json_object_dotget_number(root_obj,"freq_hz");
    obj->tx_mode = (uint8_t)json_object_dotget_number(root_obj,"tx_mode");
    obj->count_us = (uint32_t)json_object_dotget_number(root_obj,"count_us");
    obj->rf_chain = (uint8_t)json_object_dotget_number(root_obj,"rf_chain");
    obj->rf_power = (int8_t)json_object_dotget_number(root_obj,"rf_power");
    obj->modulation = (uint8_t)json_object_dotget_number(root_obj,"modulation");
    obj->bandwidth = (uint8_t)json_object_dotget_number(root_obj,"bandwidth");
    obj->datarate = (uint32_t)json_object_dotget_number(root_obj,"datarate");
    obj->coderate = (uint8_t)json_object_dotget_number(root_obj,"coderate");
    obj->invert_pol = (json_object_dotget_number(root_obj,"invert_pol")==1.0?true:false);
    obj->f_dev = (uint8_t)json_object_dotget_number(root_obj,"f_dev");
    obj->preamble = (uint16_t)json_object_dotget_number(root_obj,"preamble");
    obj->no_crc = (json_object_dotget_number(root_obj,"no_crc")==1.0?true:false);
    obj->no_header = (json_object_dotget_number(root_obj,"no_header")==1.0?true:false);
    obj->size = (uint16_t)json_object_dotget_number(root_obj,"size");
    const char* payload_ch = json_object_dotget_string(root_obj,"payload");
    int size_payload = strlen(payload_ch);
    if(size_payload > 256){
        printf("PAYLOAD TOO LONG\n");
        obj->size = 0;
        json_value_free(root_val);
        return obj;
    }
    b64_to_bin(payload_ch, strlen(payload_ch), obj->payload, sizeof obj->payload);
    json_value_free(root_val);
    return obj;
}