/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2013 Semtech-Cycleo

Description:
	Configure Lora concentrator and forward packets to a server

License: Revised BSD License, see LICENSE.TXT file include in the project
Maintainer: Sylvain Miermont
*/

#include "../headers/basic_pkt_fwd.h"


static int parse_gateway_configuration(const char * conf_file);
static int parse_actions_informations(JSON_Value *obj_value);
static int parse_mqtt_informations(JSON_Object *conf_obj);
/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */

static void sig_handler(int sigio) {
	if (sigio == SIGQUIT) {
		quit_sig = true;;
	} else if ((sigio == SIGINT) || (sigio == SIGTERM)) {
		exit_sig = true;
	}
	return;
}

static int parse_mqtt_informations(JSON_Object *conf_obj){
	const char* mqtt_host_name = json_object_get_string(conf_obj, "host_name");
	if (mqtt_host_name == NULL) {
		return -1;
	}
	MSG("[INFO] mqtt_informations host_name with value %s\n", mqtt_host_name);
	JSON_Value *val = json_object_get_value(conf_obj, "port");
	if (val == NULL) {
		return -1;
	}
	int serv_port_up = (int)json_value_get_number(val);
	MSG("[INFO] mqtt_informations serv_port_up with value %d\n", serv_port_up);
	const char* mqtt_client_id = json_object_get_string(conf_obj, "client_id");
	if (mqtt_client_id == NULL) {
		return -1;
	}
	MSG("[INFO] mqtt_informations mqtt_client_id with value %s\n", mqtt_client_id);
	const char* mqtt_username = json_object_get_string(conf_obj, "username");
	if (mqtt_username == NULL) {
		return -1;
	}
	MSG("[INFO] mqtt_informations mqtt_username with value %s\n", mqtt_username);
	const char* mqtt_password = json_object_get_string(conf_obj, "password");
	if (mqtt_password == NULL) {
		return -1;
	}
	MSG("[INFO] mqtt_informations mqtt_password with value %s\n", mqtt_password);
	initializeMQTT(strdup(mqtt_host_name), serv_port_up,strdup(mqtt_client_id), strdup(mqtt_username),strdup(mqtt_password));
	return 0;
}
static int parse_actions_informations(JSON_Value *obj_value){
	JSON_Array *actions_mapping = json_value_get_array(obj_value);
	JSON_Object *temp_obj;
	if(actions_mapping == NULL){
		return -1;
	}
	routes_Initialize();
	for(int i = 0; i < json_array_get_count(actions_mapping); i++){
		temp_obj = json_array_get_object(actions_mapping, i);
		if(temp_obj == NULL){
			return -1;
		}
		const char* temp_action = json_object_get_string(temp_obj, "action");
		if(temp_action == NULL){
			return -1;
		}
		const char* temp_topic = json_object_get_string(temp_obj, "topic");
		if(temp_topic == NULL){
			return -1;
		}
		routes_load_key_topic_map(strdup(temp_action), strdup(temp_topic));
		routes_load_key_topic_map(strdup(temp_topic), strdup(temp_action));
		MSG("Action: %s , topic: %s\n", temp_action, temp_topic);
		fflush(stdout);
	}
	return 0;
}

static int parse_gateway_configuration(const char * conf_file) {
	JSON_Value *root_val;
	JSON_Object *conf_obj = NULL;
	JSON_Value *val = NULL; /* needed to detect the absence of some fields */
	const char *str; /* pointer to sub-strings in the JSON data */
	unsigned long long ull = 0;
	
	/* try to parse JSON */
	root_val = json_parse_file_with_comments(conf_file);
	if (root_val == NULL) {
		MSG("ERROR: %s is not a valid JSON file\n", conf_file);
		exit(EXIT_FAILURE);
	}
	
	/* point to the gateway configuration object */
	JSON_Object *root_object = json_value_get_object(root_val);

	conf_obj = json_object_get_object(root_object, "mqtt_communication");
	if (conf_obj == NULL) {
		MSG("INFO: %s does not contain a JSON object named mqtt_communication\n", conf_file);
		return -1;
	} else {
		MSG("INFO: %s does contain a JSON object named mqtt_communication, parsing gateway parameters\n", conf_file);
	}
	if(parse_mqtt_informations(conf_obj) == -1){
		return -1;
	}

	val = json_object_get_value(root_object, "actions_mapping");
	if (val == NULL) {
		MSG("INFO: %s does not contain a JSON object named actions_mapping\n", conf_file);
		return -1;
	} else {
		MSG("INFO: %s does contain a JSON object named actions_mapping, parsing gateway parameters\n", conf_file);
	}
	if(parse_actions_informations(val) == -1){
		return -1;
	}
	conf_obj = NULL;
	conf_obj = json_object_get_object(root_object, "gateway_conf");
	if (conf_obj == NULL) {
		MSG("INFO: %s does not contain a JSON object named gateway_conf\n", conf_file);
		return -1;
	} else {
		MSG("INFO: %s does contain a JSON object named gateway_conf, parsing gateway parameters\n", conf_file);
	}
	
	/* gateway unique identifier (aka MAC address) (optional) */
	str = json_object_get_string(conf_obj, "gateway_ID");
	if (str != NULL) {
		sscanf(str, "%llx", &ull);
		lgwm = ull;
		MSG("INFO: gateway MAC address is configured to %016llX\n", ull);
	}
	
	/* server hostname or IP address (optional) */
	str = json_object_get_string(conf_obj, "server_address");
	if (str != NULL) {
		strncpy(serv_addr, str, sizeof serv_addr);
		MSG("INFO: server hostname or IP address is configured to \"%s\"\n", serv_addr);
	}
	
	/* get up and down ports (optional) */
	val = json_object_get_value(conf_obj, "serv_port_up");
	if (val != NULL) {
		snprintf(serv_port_up, sizeof serv_port_up, "%u", (uint16_t)json_value_get_number(val));
		MSG("INFO: upstream port is configured to \"%s\"\n", serv_port_up);
	}
	val = json_object_get_value(conf_obj, "serv_port_down");
	if (val != NULL) {
		snprintf(serv_port_down, sizeof serv_port_down, "%u", (uint16_t)json_value_get_number(val));
		MSG("INFO: downstream port is configured to \"%s\"\n", serv_port_down);
	}
	
	/* get keep-alive interval (in seconds) for downstream (optional) */
	val = json_object_get_value(conf_obj, "keepalive_interval");
	if (val != NULL) {
		keepalive_time = (int)json_value_get_number(val);
		MSG("INFO: downstream keep-alive interval is configured to %u seconds\n", keepalive_time);
	}
	
	/* get interval (in seconds) for statistics display (optional) */
	val = json_object_get_value(conf_obj, "stat_interval");
	if (val != NULL) {
		stat_interval = (unsigned)json_value_get_number(val);
		MSG("INFO: statistics display interval is configured to %u seconds\n", stat_interval);
	}
	
	/* get time-out value (in ms) for upstream datagrams (optional) */
	val = json_object_get_value(conf_obj, "push_timeout_ms");
	if (val != NULL) {
		push_timeout_half.tv_usec = 500 * (long int)json_value_get_number(val);
		MSG("INFO: upstream PUSH_DATA time-out is configured to %u ms\n", (unsigned)(push_timeout_half.tv_usec / 500));
	}
	
	/* packet filtering parameters */
	val = json_object_get_value(conf_obj, "forward_crc_valid");
	if (json_value_get_type(val) == JSONBoolean) {
		fwd_valid_pkt = (bool)json_value_get_boolean(val);
	}
	MSG("INFO: packets received with a valid CRC will%s be forwarded\n", (fwd_valid_pkt ? "" : " NOT"));
	val = json_object_get_value(conf_obj, "forward_crc_error");
	if (json_value_get_type(val) == JSONBoolean) {
		fwd_error_pkt = (bool)json_value_get_boolean(val);
	}
	MSG("INFO: packets received with a CRC error will%s be forwarded\n", (fwd_error_pkt ? "" : " NOT"));
	val = json_object_get_value(conf_obj, "forward_crc_disabled");
	if (json_value_get_type(val) == JSONBoolean) {
		fwd_nocrc_pkt = (bool)json_value_get_boolean(val);
	}
	MSG("INFO: packets received with no CRC will%s be forwarded\n", (fwd_nocrc_pkt ? "" : " NOT"));
	
	/* Auto-quit threshold (optional) */
	val = json_object_get_value(conf_obj, "autoquit_threshold");
	if (val != NULL) {
		autoquit_threshold = (uint32_t)json_value_get_number(val);
		MSG("INFO: Auto-quit after %u non-acknowledged PULL_DATA\n", autoquit_threshold);
	}
	
	/* free JSON parsing data structure */
	json_value_free(root_val);
	return 0;
}

static double difftimespec(struct timespec end, struct timespec beginning) {
	double x;
	
	x = 1E-9 * (double)(end.tv_nsec - beginning.tv_nsec);
	x += (double)(end.tv_sec - beginning.tv_sec);
	
	return x;
}

/* -------------------------------------------------------------------------- */
/* --- MAIN FUNCTION -------------------------------------------------------- */

int main(int argc, char *argv[])
{
	struct sigaction sigact; /* SIGQUIT&SIGINT&SIGTERM signal handling */
	int i; /* loop variable and temporary variable for return value */
	
	/* configuration file related */
	char *global_cfg_path= "/cfg-data/config.json"; /* contain global (typ. network-wide) configuration */
	
	/* threads */
	pthread_t thrid_up;
	pthread_t thrid_down;
	
	/* network socket creation */
	struct addrinfo hints;
	struct addrinfo *result; /* store result of getaddrinfo */
	struct addrinfo *q; /* pointer to move into *result data */
	char host_name[64];
	char port_name[64];
	
	/* variables to get local copies of measurements */
	uint32_t cp_nb_rx_rcv;
	uint32_t cp_nb_rx_ok;
	uint32_t cp_nb_rx_bad;
	uint32_t cp_nb_rx_nocrc;
	uint32_t cp_up_pkt_fwd;
	uint32_t cp_up_network_byte;
	uint32_t cp_up_payload_byte;
	uint32_t cp_up_dgram_sent;
	uint32_t cp_up_ack_rcv;
	uint32_t cp_dw_pull_sent;
	uint32_t cp_dw_ack_rcv;
	uint32_t cp_dw_dgram_rcv;
	uint32_t cp_dw_network_byte;
	uint32_t cp_dw_payload_byte;
	uint32_t cp_nb_tx_ok;
	uint32_t cp_nb_tx_fail;
	
	/* statistics variable */
	time_t t;
	char stat_timestamp[24];
	float rx_ok_ratio;
	float rx_bad_ratio;
	float rx_nocrc_ratio;
	float up_ack_ratio;
	float dw_ack_ratio;
	
	/* display version informations */
	MSG("*** Basic Packet Forwarder for Lora Gateway ***\nVersion: " VERSION_STRING "\n");
	MSG("*** Lora concentrator HAL library version info ***\n%s\n***\n", "5.0");
	
	/* display host endianness */
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		MSG("INFO: Little endian host\n");
	#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		MSG("INFO: Big endian host\n");
	#else
		MSG("INFO: Host endianness unknown\n");
	#endif
	
	/* load configuration file */
	if (access(global_cfg_path, R_OK) == 0) {
		MSG("INFO: found global configuration file config.json, parsing it\n");
		if(parse_gateway_configuration(global_cfg_path) == -1){
			return -1;
		}
	} else {
		MSG("ERROR: [main] failed to find any configuration file named config.json\n");
		exit(EXIT_FAILURE);
	}
	
	/* sanity check on configuration variables */
	// TODO
	
	/* process some of the configuration variables */
	net_mac_h = htonl((uint32_t)(0xFFFFFFFF & (lgwm>>32)));
	net_mac_l = htonl((uint32_t)(0xFFFFFFFF &  lgwm  ));
	
	/* prepare hints to open network sockets */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; /* should handle IP v4 or v6 automatically */
	hints.ai_socktype = SOCK_DGRAM;
	
	/* look for server address w/ upstream port */
	i = getaddrinfo(serv_addr, serv_port_up, &hints, &result);
	if (i != 0) {
		MSG("ERROR: [up] getaddrinfo on address %s (PORT %s) returned %s\n", serv_addr, serv_port_up, gai_strerror(i));
		exit(EXIT_FAILURE);
	}
	
	/* try to open socket for upstream traffic */
	for (q=result; q!=NULL; q=q->ai_next) {
		sock_up = socket(q->ai_family, q->ai_socktype,q->ai_protocol);
		if (sock_up == -1) continue; /* try next field */
		else break; /* success, get out of loop */
	}
	if (q == NULL) {
		MSG("ERROR: [up] failed to open socket to any of server %s addresses (port %s)\n", serv_addr, serv_port_up);
		i = 1;
		for (q=result; q!=NULL; q=q->ai_next) {
			getnameinfo(q->ai_addr, q->ai_addrlen, host_name, sizeof host_name, port_name, sizeof port_name, NI_NUMERICHOST);
			MSG("INFO: [up] result %i host:%s service:%s\n", i, host_name, port_name);
			++i;
		}
		exit(EXIT_FAILURE);
	}
	
	/* connect so we can send/receive packet with the server only */
	i = connect(sock_up, q->ai_addr, q->ai_addrlen);
	if (i != 0) {
		MSG("ERROR: [up] connect returned %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result);

	/* look for server address w/ downstream port */
	i = getaddrinfo(serv_addr, serv_port_down, &hints, &result);
	if (i != 0) {
		MSG("ERROR: [down] getaddrinfo on address %s (port %s) returned %s\n", serv_addr, serv_port_up, gai_strerror(i));
		exit(EXIT_FAILURE);
	}
	
	/* try to open socket for downstream traffic */
	for (q=result; q!=NULL; q=q->ai_next) {
		sock_down = socket(q->ai_family, q->ai_socktype,q->ai_protocol);
		if (sock_down == -1) continue; /* try next field */
		else break; /* success, get out of loop */
	}
	if (q == NULL) {
		MSG("ERROR: [down] failed to open socket to any of server %s addresses (port %s)\n", serv_addr, serv_port_up);
		i = 1;
		for (q=result; q!=NULL; q=q->ai_next) {
			getnameinfo(q->ai_addr, q->ai_addrlen, host_name, sizeof host_name, port_name, sizeof port_name, NI_NUMERICHOST);
			MSG("INFO: [down] result %i host:%s service:%s\n", i, host_name, port_name);
			++i;
		}
		exit(EXIT_FAILURE);
	}
	
	/* connect so we can send/receive packet with the server only */
	i = connect(sock_down, q->ai_addr, q->ai_addrlen);
	if (i != 0) {
		MSG("ERROR: [down] connect returned %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result);
	
	/* starting the concentrator */
	i = lgw_start();
	if (i == LGW_HAL_SUCCESS) {
		MSG("INFO: [main] concentrator started, packet can now be received\n");
	} else {
		MSG("ERROR: [main] failed to start the concentrator\n");
		exit(EXIT_FAILURE);
	}
	
	/* spawn threads to manage upstream and downstream */
	i = pthread_create( &thrid_up, NULL, (void * (*)(void *))thread_up, NULL);
	if (i != 0) {
		MSG("ERROR: [main] impossible to create upstream thread\n");
		exit(EXIT_FAILURE);
	}
	i = pthread_create( &thrid_down, NULL, (void * (*)(void *))thread_down, NULL);
	if (i != 0) {
		MSG("ERROR: [main] impossible to create downstream thread\n");
		exit(EXIT_FAILURE);
	}
	
	/* configure signal handling */
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigact.sa_handler = sig_handler;
	sigaction(SIGQUIT, &sigact, NULL); /* Ctrl-\ */
	sigaction(SIGINT, &sigact, NULL); /* Ctrl-C */
	sigaction(SIGTERM, &sigact, NULL); /* default "kill" command */
	
	//INIT RANDOM
	srand(time(NULL));
	/* main loop task : statistics collection */
	while (!exit_sig && !quit_sig) {
		/* wait for next reporting interval */
		sleep(stat_interval);
		
		/* get timestamp for statistics */
		t = time(NULL);
		strftime(stat_timestamp, sizeof stat_timestamp, "%F %T %Z", gmtime(&t));
		
		/* access upstream statistics, copy and reset them */
		pthread_mutex_lock(&mx_meas_up);
		cp_nb_rx_rcv       = meas_nb_rx_rcv;
		cp_nb_rx_ok        = meas_nb_rx_ok;
		cp_nb_rx_bad       = meas_nb_rx_bad;
		cp_nb_rx_nocrc     = meas_nb_rx_nocrc;
		cp_up_pkt_fwd      = meas_up_pkt_fwd;
		cp_up_network_byte = meas_up_network_byte;
		cp_up_payload_byte = meas_up_payload_byte;
		cp_up_dgram_sent   = meas_up_dgram_sent;
		cp_up_ack_rcv      = meas_up_ack_rcv;
		meas_nb_rx_rcv = 0;
		meas_nb_rx_ok = 0;
		meas_nb_rx_bad = 0;
		meas_nb_rx_nocrc = 0;
		meas_up_pkt_fwd = 0;
		meas_up_network_byte = 0;
		meas_up_payload_byte = 0;
		meas_up_dgram_sent = 0;
		meas_up_ack_rcv = 0;
		pthread_mutex_unlock(&mx_meas_up);
		if (cp_nb_rx_rcv > 0) {
			rx_ok_ratio = (float)cp_nb_rx_ok / (float)cp_nb_rx_rcv;
			rx_bad_ratio = (float)cp_nb_rx_bad / (float)cp_nb_rx_rcv;
			rx_nocrc_ratio = (float)cp_nb_rx_nocrc / (float)cp_nb_rx_rcv;
		} else {
			rx_ok_ratio = 0.0;
			rx_bad_ratio = 0.0;
			rx_nocrc_ratio = 0.0;
		}
		if (cp_up_dgram_sent > 0) {
			up_ack_ratio = (float)cp_up_ack_rcv / (float)cp_up_dgram_sent;
		} else {
			up_ack_ratio = 0.0;
		}
		
		/* access downstream statistics, copy and reset them */
		pthread_mutex_lock(&mx_meas_dw);
		cp_dw_pull_sent    =  meas_dw_pull_sent;
		cp_dw_ack_rcv      =  meas_dw_ack_rcv;
		cp_dw_dgram_rcv    =  meas_dw_dgram_rcv;
		cp_dw_network_byte =  meas_dw_network_byte;
		cp_dw_payload_byte =  meas_dw_payload_byte;
		cp_nb_tx_ok        =  meas_nb_tx_ok;
		cp_nb_tx_fail      =  meas_nb_tx_fail;
		meas_dw_pull_sent = 0;
		meas_dw_ack_rcv = 0;
		meas_dw_dgram_rcv = 0;
		meas_dw_network_byte = 0;
		meas_dw_payload_byte = 0;
		meas_nb_tx_ok = 0;
		meas_nb_tx_fail = 0;
		pthread_mutex_unlock(&mx_meas_dw);
		if (cp_dw_pull_sent > 0) {
			dw_ack_ratio = (float)cp_dw_ack_rcv / (float)cp_dw_pull_sent;
		} else {
			dw_ack_ratio = 0.0;
		}
		/*
		//display a report 
		printf("\n##### %s #####\n", stat_timestamp);
		printf("### [UPSTREAM] ###\n");
		printf("# RF packets received by concentrator: %u\n", cp_nb_rx_rcv);
		printf("# CRC_OK: %.2f%%, CRC_FAIL: %.2f%%, NO_CRC: %.2f%%\n", 100.0 * rx_ok_ratio, 100.0 * rx_bad_ratio, 100.0 * rx_nocrc_ratio);
		printf("# RF packets forwarded: %u (%u bytes)\n", cp_up_pkt_fwd, cp_up_payload_byte);
		printf("# PUSH_DATA datagrams sent: %u (%u bytes)\n", cp_up_dgram_sent, cp_up_network_byte);
		printf("# PUSH_DATA acknowledged: %.2f%%\n", 100.0 * up_ack_ratio);
		printf("### [DOWNSTREAM] ###\n");
		printf("# PULL_DATA sent: %u (%.2f%% acknowledged)\n", cp_dw_pull_sent, 100.0 * dw_ack_ratio);
		printf("# PULL_RESP(onse) datagrams received: %u (%u bytes)\n", cp_dw_dgram_rcv, cp_dw_network_byte);
		printf("# RF packets sent to concentrator: %u (%u bytes)\n", (cp_nb_tx_ok+cp_nb_tx_fail), cp_dw_payload_byte);
		printf("# TX errors: %u\n", cp_nb_tx_fail);
		printf("##### END #####\n");*/
	}
	
	/* wait for upstream thread to finish (1 fetch cycle max) */
	pthread_join(thrid_up, NULL);
	pthread_cancel(thrid_down); /* don't wait for downstream thread */
	
	/* if an exit signal was received, try to quit properly */
	if (exit_sig) {
		/* shut down network sockets */
		shutdown(sock_up, SHUT_RDWR);
		shutdown(sock_down, SHUT_RDWR);
		/* stop the hardware */
		i = lgw_stop();
		if (i == LGW_HAL_SUCCESS) {
			MSG("INFO: concentrator stopped successfully\n");
		} else {
			MSG("WARNING: failed to stop concentrator successfully\n");
		}
	}
	mqtt_stop_listen();
    freeMQTT();
	MSG("INFO: Exiting packet forwarder program\n");
	exit(EXIT_SUCCESS);
}

/* -------------------------------------------------------------------------- */
/* --- THREAD 1: RECEIVING PACKETS AND FORWARDING THEM ---------------------- */

void thread_up(void) {
	int i, j; /* loop variables */
	unsigned pkt_in_dgram; /* nb on Lora packet in the current datagram */
	
	/* allocate memory for packet fetching and processing */
	struct lgw_pkt_rx_s rxpkt[NB_PKT_MAX]; /* array containing inbound packets + metadata */
	struct lgw_pkt_rx_s *p; /* pointer on a RX packet */
	int nb_pkt;
	
	/* local timestamp variables until we get accurate GPS time */
	struct timespec fetch_time;
	struct tm * x;
	char fetch_timestamp[28]; /* timestamp as a text string */
	
	/* data buffers */
	uint8_t buff_up[TX_BUFF_SIZE]; /* buffer to compose the upstream packet */
	int buff_index;
	uint8_t buff_ack[32]; /* buffer to receive acknowledges */
	
	/* protocol variables */
	uint8_t token_h; /* random token for acknowledgement matching */
	uint8_t token_l; /* random token for acknowledgement matching */
	
	/* ping measurement variables */
	struct timespec send_time;
	struct timespec recv_time;
	
	/* set upstream socket RX timeout */
	i = setsockopt(sock_up, SOL_SOCKET, SO_RCVTIMEO, (void *)&push_timeout_half, sizeof push_timeout_half);
	if (i != 0) {
		MSG("ERROR: [up] setsockopt returned %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	/* pre-fill the data buffer with fixed fields */
	buff_up[0] = PROTOCOL_VERSION;
	buff_up[3] = PKT_PUSH_DATA;
	*(uint32_t *)(buff_up + 4) = net_mac_h;
	*(uint32_t *)(buff_up + 8) = net_mac_l;
	
	while (!exit_sig && !quit_sig) {
	
		/* fetch packets */
		pthread_mutex_lock(&mx_concent);
		nb_pkt = lgw_receive(NB_PKT_MAX, rxpkt);
		pthread_mutex_unlock(&mx_concent);

		if (nb_pkt == LGW_HAL_ERROR) {
			MSG("ERROR: [up] failed packet fetch, exiting\n");
			exit(EXIT_FAILURE);
		} else if (nb_pkt == 0) {
			usleep(FETCH_SLEEP_MS*1000); /* wait a short time if no packets */
			continue;
		}
		
		/* local timestamp generation until we get accurate GPS time */
		clock_gettime(CLOCK_REALTIME, &fetch_time);
		x = gmtime(&(fetch_time.tv_sec)); /* split the UNIX timestamp to its calendar components */
		snprintf(fetch_timestamp, sizeof fetch_timestamp, "%04i-%02i-%02iT%02i:%02i:%02i.%06liZ", (x->tm_year)+1900, (x->tm_mon)+1, x->tm_mday, x->tm_hour, x->tm_min, x->tm_sec, (fetch_time.tv_nsec)/1000); /* ISO 8601 format */
		
		/* start composing datagram with the header */
		token_h = (uint8_t)rand(); /* random token */
		token_l = (uint8_t)rand(); /* random token */
		buff_up[1] = token_h;
		buff_up[2] = token_l;
		buff_index = 12; /* 12-byte header */
		
		/* start of JSON structure */
		memcpy((void *)(buff_up + buff_index), (void *)"{\"rxpk\":[", 9);
		buff_index += 9;
		
		/* serialize Lora packets metadata and payload */
		pkt_in_dgram = 0;
		for (i=0; i < nb_pkt; ++i) {
			p = &rxpkt[i];
			
			struct timeval msg_arrived;
    		gettimeofday(&msg_arrived, NULL);
            char str_to_print[300];
            sprintf(str_to_print, "[INFO-GW] Uplink Received, timestamp (s.us) %ld.%ld",
						msg_arrived.tv_sec, msg_arrived.tv_usec);
            mqtt_publish(str_to_print,"/DbALT", 0);
			printf("%s",str_to_print);

			/* basic packet filtering */
			pthread_mutex_lock(&mx_meas_up);
			meas_nb_rx_rcv += 1;
			switch(p->status) {
				case STAT_CRC_OK:
					meas_nb_rx_ok += 1;
					if (!fwd_valid_pkt) {
						pthread_mutex_unlock(&mx_meas_up);
						continue; /* skip that packet */
					}
					break;
				case STAT_CRC_BAD:
					meas_nb_rx_bad += 1;
					if (!fwd_error_pkt) {
						pthread_mutex_unlock(&mx_meas_up);
						continue; /* skip that packet */
					}
					break;
				case STAT_NO_CRC:
					meas_nb_rx_nocrc += 1;
					if (!fwd_nocrc_pkt) {
						pthread_mutex_unlock(&mx_meas_up);
						continue; /* skip that packet */
					}
					break;
				default:
					MSG("WARNING: [up] received packet with unknown status %u (size %u, modulation %u, BW %u, DR %u, RSSI %.1f)\n", p->status, p->size, p->modulation, p->bandwidth, p->datarate, p->rssi);
					pthread_mutex_unlock(&mx_meas_up);
					continue; /* skip that packet */
					// exit(EXIT_FAILURE);
			}
			meas_up_pkt_fwd += 1;
			meas_up_payload_byte += p->size;
			pthread_mutex_unlock(&mx_meas_up);
			
			/* Start of packet, add inter-packet separator if necessary */
			if (pkt_in_dgram == 0) {
				buff_up[buff_index] = '{';
				++buff_index;
			} else {
				buff_up[buff_index] = ',';
				buff_up[buff_index+1] = '{';
				buff_index += 2;
			}
			
			/* RAW timestamp, 8-17 useful chars */
			j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, "\"tmst\":%u", p->count_us);
			if (j > 0) {
				buff_index += j;
			} else {
				MSG("ERROR: [up] snprintf failed line %u\n", (__LINE__ - 4));
				exit(EXIT_FAILURE);
			}
			
			/* Packet RX time (system time based), 37 useful chars */
			memcpy((void *)(buff_up + buff_index), (void *)",\"time\":\"???????????????????????????\"", 37);
			memcpy((void *)(buff_up + buff_index + 9), (void *)fetch_timestamp, 27);
			buff_index += 37;
			
			/* Packet concentrator channel, RF chain & RX frequency, 34-36 useful chars */
			j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, ",\"chan\":%1u,\"rfch\":%1u,\"freq\":%.2lf", p->if_chain, p->rf_chain, p->freq_hz);
			if (j > 0) {
				buff_index += j;
			} else {
				MSG("ERROR: [up] snprintf failed line %u\n", (__LINE__ - 4));
				exit(EXIT_FAILURE);
			}
			
			/* Packet status, 9-10 useful chars */
			switch (p->status) {
				case STAT_CRC_OK:
					memcpy((void *)(buff_up + buff_index), (void *)",\"stat\":1", 9);
					buff_index += 9;
					break;
				case STAT_CRC_BAD:
					memcpy((void *)(buff_up + buff_index), (void *)",\"stat\":-1", 10);
					buff_index += 10;
					break;
				case STAT_NO_CRC:
					memcpy((void *)(buff_up + buff_index), (void *)",\"stat\":0", 9);
					buff_index += 9;
					break;
				default:
					MSG("ERROR: [up] received packet with unknown status\n");
					memcpy((void *)(buff_up + buff_index), (void *)",\"stat\":?", 9);
					buff_index += 9;
					exit(EXIT_FAILURE);
			}
			
			/* Packet modulation, 13-14 useful chars */
			if (p->modulation == MOD_LORA) {
				memcpy((void *)(buff_up + buff_index), (void *)",\"modu\":\"LORA\"", 14);
				buff_index += 14;
				
				/* Lora datarate & bandwidth, 16-19 useful chars */
				switch (p->datarate) {
					case DR_LORA_SF7:
						memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF7", 12);
						buff_index += 12;
						break;
					case DR_LORA_SF8:
						memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF8", 12);
						buff_index += 12;
						break;
					case DR_LORA_SF9:
						memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF9", 12);
						buff_index += 12;
						break;
					case DR_LORA_SF10:
						memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF10", 13);
						buff_index += 13;
						break;
					case DR_LORA_SF11:
						memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF11", 13);
						buff_index += 13;
						break;
					case DR_LORA_SF12:
						memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF12", 13);
						buff_index += 13;
						break;
					default:
						MSG("ERROR: [up] lora packet with unknown datarate\n");
						memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF?", 12);
						buff_index += 12;
						exit(EXIT_FAILURE);
				}
				switch (p->bandwidth) {
					case BW_125KHZ:
						memcpy((void *)(buff_up + buff_index), (void *)"BW125\"", 6);
						buff_index += 6;
						break;
					case BW_250KHZ:
						memcpy((void *)(buff_up + buff_index), (void *)"BW250\"", 6);
						buff_index += 6;
						break;
					case BW_500KHZ:
						memcpy((void *)(buff_up + buff_index), (void *)"BW500\"", 6);
						buff_index += 6;
						break;
					default:
						MSG("ERROR: [up] lora packet with unknown bandwidth\n");
						memcpy((void *)(buff_up + buff_index), (void *)"BW?\"", 4);
						buff_index += 4;
						exit(EXIT_FAILURE);
				}
				
				/* Packet ECC coding rate, 11-13 useful chars */
				switch (p->coderate) {
					case CR_LORA_4_5:
						memcpy((void *)(buff_up + buff_index), (void *)",\"codr\":\"4/5\"", 13);
						buff_index += 13;
						break;
					case CR_LORA_4_6:
						memcpy((void *)(buff_up + buff_index), (void *)",\"codr\":\"4/6\"", 13);
						buff_index += 13;
						break;
					case CR_LORA_4_7:
						memcpy((void *)(buff_up + buff_index), (void *)",\"codr\":\"4/7\"", 13);
						buff_index += 13;
						break;
					case CR_LORA_4_8:
						memcpy((void *)(buff_up + buff_index), (void *)",\"codr\":\"4/8\"", 13);
						buff_index += 13;
						break;
					case 0: /* treat the CR0 case (mostly false sync) */
						memcpy((void *)(buff_up + buff_index), (void *)",\"codr\":\"OFF\"", 13);
						buff_index += 13;
						break;
					default:
						MSG("ERROR: [up] lora packet with unknown coderate\n");
						memcpy((void *)(buff_up + buff_index), (void *)",\"codr\":\"?\"", 11);
						buff_index += 11;
						exit(EXIT_FAILURE);
				}
				
				/* Lora SNR, 11-13 useful chars */
				j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, ",\"lsnr\":%.1f", p->snr);
				if (j > 0) {
					buff_index += j;
				} else {
					MSG("ERROR: [up] snprintf failed line %u\n", (__LINE__ - 4));
					exit(EXIT_FAILURE);
				}
			} else if (p->modulation == MOD_FSK) {
				memcpy((void *)(buff_up + buff_index), (void *)",\"modu\":\"FSK\"", 13);
				buff_index += 13;
				
				/* FSK datarate, 11-14 useful chars */
				j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, ",\"datr\":%u", p->datarate);
				if (j > 0) {
					buff_index += j;
				} else {
					MSG("ERROR: [up] snprintf failed line %u\n", (__LINE__ - 4));
					exit(EXIT_FAILURE);
				}
			} else {
				MSG("ERROR: [up] received packet with unknown modulation\n");
				exit(EXIT_FAILURE);
			}
			
			/* Packet RSSI, payload size, 18-23 useful chars */
			j = snprintf((char *)(buff_up + buff_index), TX_BUFF_SIZE-buff_index, ",\"rssi\":%.0f,\"size\":%u", p->rssi, p->size);
			if (j > 0) {
				buff_index += j;
			} else {
				MSG("ERROR: [up] snprintf failed line %u\n", (__LINE__ - 4));
				exit(EXIT_FAILURE);
			}
			
			//\"MACVersion\":\"1.0.4\",
			/* Packet base64-encoded payload, 14-350 useful chars */
			memcpy((void *)(buff_up + buff_index), (void *)",\"data\":\"", 9);
			buff_index += 9;
			j = bin_to_b64(p->payload, p->size, (char *)(buff_up + buff_index), 341); // 255 bytes = 340 chars in b64 + null char 
			if (j>=0) {
				buff_index += j;
			} else {
				MSG("ERROR: [up] bin_to_b64 failed line %u\n", (__LINE__ - 5));
				exit(EXIT_FAILURE);
			}

			buff_up[buff_index] = '"';
			++buff_index;
			/* End of packet serialization */
			buff_up[buff_index] = '}';
			++buff_index;
			++pkt_in_dgram;
		}
		
		/* restart fetch sequence without sending empty JSON if all packets have been filtered out */
		if (pkt_in_dgram == 0) {
			continue;
		}
		
		/* end of packet array */
		buff_up[buff_index] = ']';
		++buff_index;
		
		/* end of JSON datagram payload */
		buff_up[buff_index] = '}';
		++buff_index;
		buff_up[buff_index] = 0; /* add string terminator, for safety */
		
		// printf("\nJSON up: %s\n", (char *)(buff_up + 12)); /* DEBUG: display JSON payload */
		
		/* send datagram to server */
		
		printf("-------------------SEND--------DG----------TO-------------SERVER----------\n");

		int s = send(sock_up, (void *)buff_up, buff_index, 0);

		struct timeval msg_arrived;
		gettimeofday(&msg_arrived, NULL);
		char str_to_print[300];
		sprintf(str_to_print, "[INFO-GW] Uplink Sent, timestamp (s.us) %ld.%ld",
					msg_arrived.tv_sec, msg_arrived.tv_usec);
		mqtt_publish(str_to_print,"/DbALT", 0);
		printf("%s",str_to_print);

		if(s == -1){
			printf("Send error\n");
		}else{
			printf("Send ok %d\n",s);
		}
		printf("MSG sent :\n");
		for(int ko = 0; ko < buff_index; ko++){
			if(ko < 12) {
				printf("%d ", buff_up[ko]);
			}else{
				printf("%c", buff_up[ko]);
			}
		}
		printf("\n.\n");


		

		clock_gettime(CLOCK_MONOTONIC, &send_time);
		pthread_mutex_lock(&mx_meas_up);
		meas_up_dgram_sent += 1;
		meas_up_network_byte += buff_index;
		
		/* wait for acknowledge (in 2 times, to catch extra packets) */
		for (i=0; i<2; ++i) {
			j = recv(sock_up, (void *)buff_ack, sizeof buff_ack, 0);
			clock_gettime(CLOCK_MONOTONIC, &recv_time);
			if (j == -1) {
				if (errno == EAGAIN) { /* timeout */
					continue;
				} else { /* server connection error */
					break;
				}
			} else if ((j < 4) || (buff_ack[0] != PROTOCOL_VERSION) || (buff_ack[3] != PKT_PUSH_ACK)) {
				//MSG("WARNING: [up] ignored invalid non-ACL packet\n");
				continue;
			} else if ((buff_ack[1] != token_h) || (buff_ack[2] != token_l)) {
				//MSG("WARNING: [up] ignored out-of sync ACK packet\n");
				continue;
			} else {
				MSG("INFO: [up] PUSH_ACK received in %i ms\n", (int)(1000 * difftimespec(recv_time, send_time)));
				meas_up_ack_rcv += 1;
				break;
			}
		}
		pthread_mutex_unlock(&mx_meas_up);

	}
	MSG("\nINFO: End of upstream thread\n");
}

/* -------------------------------------------------------------------------- */
/* --- THREAD 2: POLLING SERVER AND EMITTING PACKETS ------------------------ */

void thread_down(void) {
	int i; /* loop variables */
	
	/* configuration and metadata for an outbound packet */
	struct lgw_pkt_tx_s txpkt;
	bool sent_immediate = false; /* option to sent the packet immediately */
	
	/* local timekeeping variables */
	struct timespec send_time; /* time of the pull request */
	struct timespec recv_time; /* time of return from recv socket call */
	
	/* data buffers */
	uint8_t buff_down[1000]; /* buffer to receive downstream packets */
	uint8_t buff_req[12]; /* buffer to compose pull requests */
	int msg_len;
	
	/* protocol variables */
	uint8_t token_h; /* random token for acknowledgement matching */
	uint8_t token_l; /* random token for acknowledgement matching */
	bool req_ack = false; /* keep track of whether PULL_DATA was acknowledged or not */
	
	/* JSON parsing variables */
	JSON_Value *root_val = NULL;
	JSON_Object *txpk_obj = NULL;
	JSON_Value *val = NULL; /* needed to detect the absence of some fields */
	const char *str; /* pointer to sub-strings in the JSON data */
	short x0, x1;
	
	/* auto-quit variable */
	uint32_t autoquit_cnt = 0; /* count the number of PULL_DATA sent since the latest PULL_ACK */
	
	/* set downstream socket RX timeout */
	i = setsockopt(sock_down, SOL_SOCKET, SO_RCVTIMEO, (void *)&pull_timeout, sizeof pull_timeout);
	if (i != 0) {
		MSG("ERROR: [down] setsockopt returned %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	/* pre-fill the pull request buffer with fixed fields */
	buff_req[0] = PROTOCOL_VERSION;
	buff_req[3] = PKT_PULL_DATA;
	*(uint32_t *)(buff_req + 4) = net_mac_h;
	*(uint32_t *)(buff_req + 8) = net_mac_l;
	
	while (!exit_sig && !quit_sig) {
		
		/* auto-quit if the threshold is crossed */
		if ((autoquit_threshold > 0) && (autoquit_cnt >= autoquit_threshold)) {
			exit_sig = true;
			MSG("INFO: [down] the last %u PULL_DATA were not ACKed, exiting application\n", autoquit_threshold);
			break;
		}
		
		/* generate random token for request */
		token_h = (uint8_t)rand(); /* random token */
		token_l = (uint8_t)rand(); /* random token */
		buff_req[1] = token_h;
		buff_req[2] = token_l;
		
		/* send PULL request and record time */
		send(sock_down, (void *)buff_req, sizeof buff_req, 0);
		clock_gettime(CLOCK_MONOTONIC, &send_time);
		pthread_mutex_lock(&mx_meas_dw);
		meas_dw_pull_sent += 1;
		pthread_mutex_unlock(&mx_meas_dw);
		req_ack = false;
		autoquit_cnt++;
		
		/* listen to packets and process them until a new PULL request must be sent */
		recv_time = send_time;
		while ((int)difftimespec(recv_time, send_time) < keepalive_time) {
			
			/* try to receive a datagram */
			msg_len = recv(sock_down, (void *)buff_down, (sizeof buff_down)-1, 0);
			//printf("msg_len : %d\n" , msg_len);
			clock_gettime(CLOCK_MONOTONIC, &recv_time);
			
			/* if no network message was received, got back to listening sock_down socket */
			if (msg_len == -1) {
				//MSG("WARNING: [down] recv returned %s\n", strerror(errno)); /* too verbose */
				MSG("NO PACKETS\n");
				continue;
			}
			
			/* if the datagram does not respect protocol, just ignore it */
			if ((msg_len < 4) || (buff_down[0] != PROTOCOL_VERSION) || ((buff_down[3] != PKT_PULL_RESP) && (buff_down[3] != PKT_PULL_ACK))) {
				MSG("WARNING: [down] ignoring invalid packet\n");
				continue;
			}
			
			/* if the datagram is an ACK, check token */
			if (buff_down[3] == PKT_PULL_ACK) {
				if ((buff_down[1] == token_h) && (buff_down[2] == token_l)) {
					if (req_ack) {
						MSG("INFO: [down] duplicate ACK received :)\n");
					} else { /* if that packet was not already acknowledged */
						req_ack = true;
						autoquit_cnt = 0;
						pthread_mutex_lock(&mx_meas_dw);
						meas_dw_ack_rcv += 1;
						pthread_mutex_unlock(&mx_meas_dw);
						MSG("msg_len : %d\n", msg_len);
						MSG("INFO: [down] PULL_ACK received in %i ms\n", (int)(1000 * difftimespec(recv_time, send_time)));
						
					}
				} else { /* out-of-sync token */
					MSG("INFO: [down] received out-of-sync ACK\n");
				}
				continue;
			}
			
			/* the datagram is a PULL_RESP */
			buff_down[msg_len] = 0; /* add string terminator, just to be safe */
			MSG("INFO: [down] PULL_RESP received :)\n"); /* very verbose */
			printf("\nJSON down: %s\n", (char *)(buff_down + 4)); /* DEBUG: display JSON payload */
			
			/* initialize TX struct and try to parse JSON */
			memset(&txpkt, 0, sizeof txpkt);
			root_val = json_parse_string_with_comments((const char *)(buff_down + 4)); /* JSON offset */
			if (root_val == NULL) {
				MSG("WARNING: [down] invalid JSON, TX aborted\n");
				continue;
			}
			
			/* look for JSON sub-object 'txpk' */
			txpk_obj = json_object_get_object(json_value_get_object(root_val), "txpk");
			if (txpk_obj == NULL) {
				MSG("WARNING: [down] no \"txpk\" object in JSON, TX aborted\n");
				json_value_free(root_val);
				continue;
			}
			
			/* Parse "immediate" tag, or target timestamp, or UTC time to be converted by GPS (mandatory) */
			i = json_object_get_boolean(txpk_obj,"imme"); /* can be 1 if true, 0 if false, or -1 if not a JSON boolean */
			if (i == 1) {
				/* TX procedure: send immediately */
				sent_immediate = true;
				MSG("INFO: [down] a packet will be sent in \"immediate\" mode\n");
			} else {
				sent_immediate = false;
				val = json_object_get_value(txpk_obj,"tmst");
				if (val != NULL) {
					/* TX procedure: send on timestamp value */
					txpkt.count_us = (uint32_t)json_value_get_number(val);
					MSG("INFO: [down] a packet will be sent on timestamp value %u\n", txpkt.count_us);
				
					struct timeval msg_arrived;
    				gettimeofday(&msg_arrived, NULL);
					char str_to_print[300];
					sprintf(str_to_print, "[INFO-GW] Downlink Received, timestamp (s.us) %ld.%ld",
						msg_arrived.tv_sec, msg_arrived.tv_usec);
					mqtt_publish(str_to_print,"/DbALT", 0);


				} else {
					MSG("WARNING: [down] only \"immediate\" and \"timestamp\" modes supported, TX aborted\n");
					json_value_free(root_val);
					continue;
				}
			}
			
			/* Parse "No CRC" flag (optional field) */
			val = json_object_get_value(txpk_obj,"ncrc");
			if (val != NULL) {
				txpkt.no_crc = (bool)json_value_get_boolean(val);
			}
			
			/* parse target frequency (mandatory) */
			val = json_object_get_value(txpk_obj,"freq");
			if (val == NULL) {
				MSG("WARNING: [down] no mandatory \"txpk.freq\" object in JSON, TX aborted\n");
				json_value_free(root_val);
				continue;
			}
			txpkt.freq_hz = (float)json_value_get_number(val);
			
			/* parse RF chain used for TX (mandatory) */
			val = json_object_get_value(txpk_obj,"rfch");
			if (val == NULL) {
				MSG("WARNING: [down] no mandatory \"txpk.rfch\" object in JSON, TX aborted\n");
				json_value_free(root_val);
				continue;
			}
			txpkt.rf_chain = (uint8_t)json_value_get_number(val);
			
			/* parse TX power (optional field) */
			val = json_object_get_value(txpk_obj,"powe");
			if (val != NULL) {
				txpkt.rf_power = (int8_t)json_value_get_number(val);
			}
			
			/* Parse modulation (mandatory) */
			str = json_object_get_string(txpk_obj, "modu");
			if (str == NULL) {
				MSG("WARNING: [down] no mandatory \"txpk.modu\" object in JSON, TX aborted\n");
				json_value_free(root_val);
				continue;
			}
			if (strcmp(str, "LORA") == 0) {
				/* Lora modulation */
				txpkt.modulation = MOD_LORA;
				
				/* Parse Lora spreading-factor and modulation bandwidth (mandatory) */
				str = json_object_get_string(txpk_obj, "datr");
				if (str == NULL) {
					MSG("WARNING: [down] no mandatory \"txpk.datr\" object in JSON, TX aborted\n");
					json_value_free(root_val);
					continue;
				}
				i = sscanf(str, "SF%2hdBW%3hd", &x0, &x1);
				if (i != 2) {
					MSG("WARNING: [down] format error in \"txpk.datr\", TX aborted\n");
					json_value_free(root_val);
					continue;
				}
				switch (x0) {
					case  7: txpkt.datarate = DR_LORA_SF7;  break;
					case  8: txpkt.datarate = DR_LORA_SF8;  break;
					case  9: txpkt.datarate = DR_LORA_SF9;  break;
					case 10: txpkt.datarate = DR_LORA_SF10; break;
					case 11: txpkt.datarate = DR_LORA_SF11; break;
					case 12: txpkt.datarate = DR_LORA_SF12; break;
					default:
						MSG("WARNING: [down] format error in \"txpk.datr\", invalid SF, TX aborted\n");
						json_value_free(root_val);
						continue;
				}
				switch (x1) {
					case 125: txpkt.bandwidth = BW_125KHZ; break;
					case 250: txpkt.bandwidth = BW_250KHZ; break;
					case 500: txpkt.bandwidth = BW_500KHZ; break;
					default:
						MSG("WARNING: [down] format error in \"txpk.datr\", invalid BW, TX aborted\n");
						json_value_free(root_val);
						continue;
				}
				
				/* Parse ECC coding rate (optional field) */
				str = json_object_get_string(txpk_obj, "codr");
				if (str == NULL) {
					MSG("WARNING: [down] no mandatory \"txpk.codr\" object in json, TX aborted\n");
					json_value_free(root_val);
					continue;
				}
				if      (strcmp(str, "4/5") == 0) txpkt.coderate = CR_LORA_4_5;
				else if (strcmp(str, "4/6") == 0) txpkt.coderate = CR_LORA_4_6;
				else if (strcmp(str, "2/3") == 0) txpkt.coderate = CR_LORA_4_6;
				else if (strcmp(str, "4/7") == 0) txpkt.coderate = CR_LORA_4_7;
				else if (strcmp(str, "4/8") == 0) txpkt.coderate = CR_LORA_4_8;
				else if (strcmp(str, "1/2") == 0) txpkt.coderate = CR_LORA_4_8;
				else {
					MSG("WARNING: [down] format error in \"txpk.codr\", TX aborted\n");
					json_value_free(root_val);
					continue;
				}
				
				/* Parse signal polarity switch (optional field) */
				val = json_object_get_value(txpk_obj,"ipol");
				if (val != NULL) {
					txpkt.invert_pol = (bool)json_value_get_boolean(val);
				}
				
				/* parse Lora preamble length (optional field, optimum min value enforced) */
				val = json_object_get_value(txpk_obj,"prea");
				if (val != NULL) {
					i = (int)json_value_get_number(val);
					if (i >= MIN_LORA_PREAMB) {
						txpkt.preamble = (uint16_t)i;
					} else {
						txpkt.preamble = (uint16_t)MIN_LORA_PREAMB;
					}
				} else {
					txpkt.preamble = (uint16_t)STD_LORA_PREAMB;
				}
				
			} else if (strcmp(str, "FSK") == 0) {
				/* FSK modulation */
				txpkt.modulation = MOD_FSK;
				
				/* parse FSK bitrate (mandatory) */
				val = json_object_get_value(txpk_obj,"datr");
				if (val == NULL) {
					MSG("WARNING: [down] no mandatory \"txpk.datr\" object in JSON, TX aborted\n");
					json_value_free(root_val);
					continue;
				}
				txpkt.datarate = (uint32_t)(json_value_get_number(val));
				
				/* parse frequency deviation (mandatory) */
				val = json_object_get_value(txpk_obj,"fdev");
				if (val == NULL) {
					MSG("WARNING: [down] no mandatory \"txpk.fdev\" object in JSON, TX aborted\n");
					json_value_free(root_val);
					continue;
				}
				txpkt.f_dev = (uint8_t)(json_value_get_number(val) / 1000.0); /* JSON value in Hz, txpkt.f_dev in kHz */
					
				/* parse FSK preamble length (optional field, optimum min value enforced) */
				val = json_object_get_value(txpk_obj,"prea");
				if (val != NULL) {
					i = (int)json_value_get_number(val);
					if (i >= MIN_FSK_PREAMB) {
						txpkt.preamble = (uint16_t)i;
					} else {
						txpkt.preamble = (uint16_t)MIN_FSK_PREAMB;
					}
				} else {
					txpkt.preamble = (uint16_t)STD_FSK_PREAMB;
				}
			
			} else {
				MSG("WARNING: [down] invalid modulation in \"txpk.modu\", TX aborted\n");
				json_value_free(root_val);
				continue;
			}
			
			/* Parse payload length (mandatory) */
			val = json_object_get_value(txpk_obj,"size");
			if (val == NULL) {
				MSG("WARNING: [down] no mandatory \"txpk.size\" object in JSON, TX aborted\n");
				json_value_free(root_val);
				continue;
			}
			txpkt.size = (uint16_t)json_value_get_number(val);
			
			/* Parse payload data (mandatory) */
			str = json_object_get_string(txpk_obj, "data");
			if (str == NULL) {
				MSG("WARNING: [down] no mandatory \"txpk.data\" object in JSON, TX aborted\n");
				json_value_free(root_val);
				continue;
			}
			i = b64_to_bin(str, strlen(str), txpkt.payload, sizeof txpkt.payload);
			if (i != txpkt.size) {
				MSG("WARNING: [down] mismatch between .size and .data size once converter to binary\n");
			}
			
			/* free the JSON parse tree from memory */
			json_value_free(root_val);
			
			/* select TX mode */
			if (sent_immediate) {
				txpkt.tx_mode = IMMEDIATE;
			} else {
				txpkt.tx_mode = TIMESTAMPED;
			}
			
			/* record measurement data */
			pthread_mutex_lock(&mx_meas_dw);
			meas_dw_dgram_rcv += 1; /* count only datagrams with no JSON errors */
			meas_dw_network_byte += msg_len; /* meas_dw_network_byte */
			meas_dw_payload_byte += txpkt.size;
			
			/* transfer data and metadata to the concentrator, and schedule TX */
			pthread_mutex_lock(&mx_concent); /* may have to wait for a fetch to finish */
			i = lgw_send(txpkt);
			pthread_mutex_unlock(&mx_concent); /* free concentrator ASAP */

			struct timeval msg_arrived;
			gettimeofday(&msg_arrived, NULL);
			char str_to_print[300];
			sprintf(str_to_print, "[INFO-GW] Downlink Sent, timestamp (s.us) %ld.%ld",
				msg_arrived.tv_sec, msg_arrived.tv_usec);
			mqtt_publish(str_to_print,"/DbALT", 0);

			if (i == LGW_HAL_ERROR) {
				meas_nb_tx_fail += 1;
				pthread_mutex_unlock(&mx_meas_dw);
				MSG("WARNING: [down] lgw_send failed\n");
				continue;
			} else {
				meas_nb_tx_ok += 1;
				pthread_mutex_unlock(&mx_meas_dw);
			}
		}
	}
	MSG("\nINFO: End of downstream thread\n");
}