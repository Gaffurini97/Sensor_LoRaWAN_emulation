#include "../../headers/commons/packet_parsings.h"


char *arr_packet_ngw_tx_to_json[] = {
    "{\"freq_hz\":", NULL,
    ",\"tx_mode\":", NULL,
    ",\"status\":", NULL,
    ",\"rf_chain\":", NULL,
    ",\"rf_power\":", NULL,
    ",\"modulation\":", NULL,
    ",\"bandwidth\":", NULL,
    ",\"datarate\":", NULL,
    ",\"coderate\":",NULL,
    ",\"invert_pol\":", NULL,
    ",\"f_dev\":", NULL,
    ",\"preamble\":", NULL,
    ",\"no_crc\":",NULL,
    ",\"no_header\":", NULL,
    ",\"size\":", NULL,
    ",\"payload\":\"",NULL,
    "\"}"
};
//int arr_tx_string_len = 222;
int arr_packet_ngw_tx_to_json_size = 33;

char *arr_packet_ngw_rx_to_json[] = {
    "{\"freq_hz\":", NULL,
    ",\"if_chain\":", NULL,
    ",\"status\":", NULL,
    ",\"count_us\":", NULL,
    ",\"rf_chain\":", NULL,
    ",\"modulation\":", NULL,
    ",\"bandwidth\":", NULL,
    ",\"datarate\":", NULL,
    ",\"coderate\":",NULL,
    ",\"rssi\":", NULL,
    ",\"snr\":", NULL,
    ",\"snr_min\":", NULL,
    ",\"snr_max\":",NULL,
    ",\"crc\":", NULL,
    ",\"size\":", NULL,
    ",\"payload\":\"",NULL,
    "\"}"
};
//int arr_rx_string_len = 208;
int arr_packet_ngw_rx_to_json_size = 33;


char* float_to_str(float value){
    char *string_float = malloc(100);
    sprintf(string_float,"%.8f", value);
    return string_float;
}
char* int_to_str(int value){
    char *string_float = malloc(100);
    sprintf(string_float,"%d", value);
    return string_float;
}

void set_value_in_arr_pkt(int position, char* value){
    if(arr_packet_ngw_rx_to_json[position]!=NULL){
        free(arr_packet_ngw_rx_to_json[position]);
    }
    arr_packet_ngw_rx_to_json[position] = value;
}


char* ngw_rx_to_json(struct lgw_pkt_rx_s *obj){
    int arrpos = 1;
    set_value_in_arr_pkt(arrpos++, float_to_str(obj->freq_hz));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->if_chain));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->status));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->count_us));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->rf_chain));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->modulation));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->bandwidth));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->datarate));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->coderate));arrpos++;
    set_value_in_arr_pkt(arrpos++, float_to_str(obj->rssi));arrpos++;
    set_value_in_arr_pkt(arrpos++, float_to_str(obj->snr));arrpos++;
    set_value_in_arr_pkt(arrpos++, float_to_str(obj->snr_min));arrpos++;
    set_value_in_arr_pkt(arrpos++, float_to_str(obj->snr_max));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->crc));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->size));arrpos++;
    char *payload_str = malloc(1000); 
    int new_char_size = bin_to_b64(obj->payload,obj->size,payload_str,1000);
    set_value_in_arr_pkt(arrpos, payload_str);
    int outsizemax = 208 + 15*100 + 1000;//208 characters, 15 elem 100 char max + 1000 payload size
    char* out_str = malloc(outsizemax);
    memset(out_str, '\0', outsizemax);
    for(arrpos = 0; arrpos < arr_packet_ngw_rx_to_json_size; arrpos++){
        int elem_size = strlen(arr_packet_ngw_rx_to_json[arrpos]);
        strncat(out_str, arr_packet_ngw_rx_to_json[arrpos], elem_size);
    }
    printf("\n\n stringa: \n%s \n\n", out_str);
    return out_str;
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
        obj->size = 0;
        json_value_free(root_val);
        return obj;
    }
    b64_to_bin(payload_ch, strlen(payload_ch), obj->payload, sizeof obj->payload);
    json_value_free(root_val);
    return obj;
}

char* ngw_tx_to_json(struct lgw_pkt_tx_s *obj){
    int arrpos = 1;
    set_value_in_arr_pkt(arrpos++, float_to_str(obj->freq_hz));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->tx_mode));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->count_us));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->rf_chain));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->rf_power));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->modulation));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->bandwidth));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->datarate));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->coderate));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->invert_pol));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->f_dev));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->preamble));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->no_crc));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->no_header));arrpos++;
    set_value_in_arr_pkt(arrpos++, int_to_str(obj->size));arrpos++;
    char *payload_str = malloc(1000); 
    int new_char_size = bin_to_b64(obj->payload,obj->size,payload_str,1000);
    set_value_in_arr_pkt(arrpos, payload_str);
    int outsizemax = 193 + 15*100 + 1000;//193 characters, 15 elem 100 char max + 1000 payload size
    char* out_str = malloc(outsizemax);
    memset(out_str, '\0', outsizemax);
    for(arrpos = 0; arrpos < arr_packet_ngw_rx_to_json_size; arrpos++){
        int elem_size = strlen(arr_packet_ngw_rx_to_json[arrpos]);
        strncat(out_str, arr_packet_ngw_rx_to_json[arrpos], elem_size);
    }
    printf("\n\n stringa: \n%s \n\n", out_str);
    return out_str;
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
    obj->invert_pol = (json_object_dotget_number(root_obj,"rssi")==1.0?true:false);
    obj->f_dev = (uint8_t)json_object_dotget_number(root_obj,"snr");
    obj->preamble = (uint16_t)json_object_dotget_number(root_obj,"snr_min");
    obj->no_crc = (json_object_dotget_number(root_obj,"snr_max")==1.0?true:false);
    obj->no_header = (json_object_dotget_number(root_obj,"crc")==1.0?true:false);
    obj->size = (uint16_t)json_object_dotget_number(root_obj,"size");
    printf("OK5\n");fflush(stdout);
    const char* payload_ch = json_object_dotget_string(root_obj,"payload");
    printf("OK5\n");fflush(stdout);
    int size_payload = strlen(payload_ch);
    printf("%d\n",size_payload);fflush(stdout);
    if(size_payload > 256){
        printf("PAYLOAD TOO LONG\n");
        obj->size = 0;
        json_value_free(root_val);
        return obj;
    }
    printf("OK5\n");fflush(stdout);
    b64_to_bin(payload_ch, strlen(payload_ch), obj->payload, sizeof obj->payload);
    printf("OK5\n");fflush(stdout);
    json_value_free(root_val);
    return obj;
}