
#include "RISTSenderInterface.h"
#include <stdio.h>

#if HAVE_MBEDTLS
	FILE *srpfile = NULL;
#endif

#if defined(_WIN32) || defined(_WIN64)
# define strtok_r strtok_s
#define MSG_DONTWAIT (0)
#endif

#define RIST_MARK_UNUSED(unused_param) ((void)(unused_param))

#define RISTSENDER_VERSION "2"

#define MAX_INPUT_COUNT 20
#define MAX_OUTPUT_COUNT 20

static int signalReceived = 0;
static int peer_connected_count = 0;
static struct rist_logging_settings logging_settings = LOGGING_SETTINGS_INITIALIZER;

struct rist_callback_object {
	int sd;
	struct evsocket_ctx *evctx;
	struct rist_ctx *receiver_ctx;
	struct rist_ctx *sender_ctx;
	struct rist_udp_config *udp_config;
	uint8_t recv[RIST_MAX_PACKET_SIZE];
};

struct receive_thread_object {
	int sd;
	struct rist_ctx *ctx[MAX_OUTPUT_COUNT];
	struct rist_udp_config *udp_config;
	uint8_t recv[RIST_MAX_PACKET_SIZE];
};

struct rist_sender_args {
	struct rist_ctx *ctx;
	char* token;
	int profile;
	enum rist_log_level loglevel;
	int encryption_type;
	char* shared_secret;
	int buffer_size;
	int statsinterval;
	uint16_t stream_id;
};

JNIEXPORT jint JNICALL Java_com_example_ristsender_RIST_getTestValue(JNIEnv *env, jobject thiz)
{
    return 20;
}

JNIEXPORT jint JNICALL Java_com_example_ristsender_RIST_SendStart(JNIEnv *env, jobject thiz, jstring url, jint port)
{
	int c;
	int option_index;
	struct rist_callback_object callback_object[MAX_INPUT_COUNT] = { {0} };
	struct evsocket_event *event[MAX_INPUT_COUNT];
	char *inputurl = "udp://@127.0.0.1:8192";
	char *outputurl = "rist://123.123.123.123:8200?cname=SENDER01&bandwidth=2560000";
	char *oobtun = NULL;
	char *shared_secret = NULL;
	int buffer_size = 0;
	int encryption_type = 0;
	int statsinterval = 1000;
	enum rist_profile profile = 0;
	enum rist_log_level loglevel = 4;
	bool npd = false;
	int faststart = 0;
	struct rist_sender_args peer_args;
	char *remote_log_address = NULL;
	bool thread_started[MAX_INPUT_COUNT +1] = {false};
	pthread_t thread_main_loop[MAX_INPUT_COUNT+1] = { 0 };

	for (size_t i = 0; i < MAX_INPUT_COUNT; i++)
		event[i] = NULL;

	// Default log settings
    struct rist_logging_settings *log_ptr = &logging_settings;
    if (rist_logging_set(&log_ptr, loglevel, NULL, NULL, NULL, stderr) != 0) {
      fprintf(stderr, "Failed to setup default logging!\n");
      exit(1);
	}

    __android_log_print(ANDROID_LOG_INFO, "RIST" , "Starting ristsender version: %s libRIST library: %s API version: %s", LIBRIST_VERSION, librist_version(), librist_api_version());

	if (profile == RIST_PROFILE_SIMPLE || faststart > 0)
    	peer_connected_count = 1;

    // Update log settings with custom loglevel and remote address if necessary
    if (rist_logging_set(&log_ptr, loglevel, NULL, NULL, remote_log_address, stderr) != 0) {
    	fprintf(stderr,"Failed to setup logging\n");
    	exit(1);
    }

    peer_args.loglevel = loglevel;
    peer_args.profile = profile;
    peer_args.encryption_type = encryption_type;
    peer_args.shared_secret = shared_secret;
    peer_args.buffer_size = buffer_size;
    peer_args.statsinterval = statsinterval;

	// Setup the input udp/rist objects: listen to the given address(es)
	int32_t stream_id_check[MAX_INPUT_COUNT];
	for (size_t j = 0; j < MAX_INPUT_COUNT; j++)
		stream_id_check[j] = -1;
	struct evsocket_ctx *evctx = NULL;
	bool atleast_one_socket_opened = false;
	char *saveptrinput;
	char *inputtoken = strtok_r(inputurl, ",", &saveptrinput);
	struct rist_udp_config *udp_config = NULL;
	for (size_t i = 0; i < MAX_INPUT_COUNT; i++) {
		if (!inputtoken)
			break;

		// First parse extra url and parameters
		if (rist_parse_udp_address2(inputtoken, &udp_config)) {
			__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not parse inputurl %s", inputtoken);
		}
        else
        {
		    // Check for duplicate stream-ids and reject the entire config if we have any dups
		    bool found_empty = false;

		    for (size_t j = 0; j < MAX_INPUT_COUNT; j++) {
		    	if (stream_id_check[j] == -1 && !found_empty) {
		    		stream_id_check[j] = (int32_t)udp_config->stream_id;
		    		__android_log_print(ANDROID_LOG_INFO, "RIST", "Assigning stream-id %d to this input", udp_config->stream_id);
		    		found_empty = true;
		    	} else if ((uint16_t)stream_id_check[j] == udp_config->stream_id) {
		    		__android_log_print(ANDROID_LOG_ERROR, "RIST", "Every input must have a unique stream-id (%d) when you multiplex", udp_config->stream_id);
		    		goto shutdown;
		    	}
		    }

		    // Setup the output rist objects (a brand new instance per receiver)
		    char *saveptroutput;
		    char *tmpoutputurl = malloc(strlen(outputurl) +1);
		    strcpy(tmpoutputurl, outputurl);
		    char *outputtoken = strtok_r(tmpoutputurl, ",", &saveptroutput);

		    // All output peers should be on the same context per receiver
		    if (rist_sender_create(&callback_object[i].sender_ctx, peer_args.profile, 0, &logging_settings) != 0) {
		    	__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not create rist sender context");
		    	break;
		    }
		    if (npd) {
		    	if (profile == RIST_PROFILE_SIMPLE)
		    	{
    		    	__android_log_print(ANDROID_LOG_INFO, "RIST", "NULL packet deletion enabled on SIMPLE profile. This is non-compliant but might work if receiver supports it (librist does)");
		    	}
		    	else
		    	{
		    		__android_log_print(ANDROID_LOG_INFO, "RIST", "NULL packet deletion enabled. Support for this feature is not guaranteed to be present on receivers. Please make sure the receiver supports it (librist does)");
		    	}

		    	if (rist_sender_npd_enable(callback_object[i].sender_ctx) != 0) {
		    		__android_log_print(ANDROID_LOG_ERROR, "RIST", "Failed to enable null packet deletion");
		    	}
		    }

		    for (size_t j = 0; j < MAX_OUTPUT_COUNT; j++) {
		    	peer_args.token = outputtoken;
		    	__android_log_print(ANDROID_LOG_DEBUG, "RIST", "peer_args.token = %s", outputtoken);
		    	peer_args.ctx = callback_object[i].sender_ctx;
		    	peer_args.stream_id = udp_config->stream_id;
		    	struct rist_peer *peer = setup_rist_peer(&peer_args);
		    	if (peer == NULL)
		    	{
                    goto shutdown;
		    	}

		    	outputtoken = strtok_r(NULL, ",", &saveptroutput);
		    	if (!outputtoken)
		    		break;
		    }
		    free(tmpoutputurl);

		    if (strcmp(udp_config->prefix, "rist") == 0) {
		    	// This is a rist input (new context for each listener)
		    	if (rist_receiver_create(&callback_object[i].receiver_ctx, peer_args.profile, &logging_settings) != 0) {
		    		__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not create rist receiver context");
		    		goto next;
		    	}
		    	peer_args.token = inputtoken;
		    	peer_args.ctx = callback_object[i].receiver_ctx;
		    	struct rist_peer *peer = setup_rist_peer(&peer_args);
		    	if (peer == NULL)
		    		atleast_one_socket_opened = true;
		    	rist_udp_config_free2(&udp_config);
		    	udp_config = NULL;
		    }
		    else {
		    	if(!evctx)
		    		evctx = evsocket_create();
		    	// This is a udp input, i.e. 127.0.0.1:5000
		    	char hostname[200] = {0};
		    	int inputlisten;
		    	uint16_t inputport;
		    	if (udpsocket_parse_url((void *)udp_config->address, hostname, 200, &inputport, &inputlisten) || !inputport || strlen(hostname) == 0) {
		    		__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not parse input url %s", inputtoken);
		    		goto next;
		    	}
		    	__android_log_print(ANDROID_LOG_INFO, "RIST", "URL parsed successfully: Host %s, Port %d", (char *) hostname, inputport);

		    	callback_object[i].sd = udpsocket_open_bind(hostname, inputport, udp_config->miface);
		    	if (callback_object[i].sd < 0) {
		    		__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not bind to: Host %s, Port %d, miface %s.", (char *) hostname, inputport, udp_config->miface);
		    		goto next;
		    	} else {
		    		udpsocket_set_nonblocking(callback_object[i].sd);
		    		__android_log_print(ANDROID_LOG_INFO, "RIST", "Input socket is open and bound %s:%d", (char *) hostname, inputport);
		    		atleast_one_socket_opened = true;
		    	}
		    	callback_object[i].udp_config = udp_config;
		    	udp_config = NULL;
		    	callback_object[i].evctx = evctx;
		    	event[i] = evsocket_addevent(callback_object[i].evctx, callback_object[i].sd, EVSOCKET_EV_READ, input_udp_recv, input_udp_sockerr,
		    		(void *)&callback_object[i]);
		    }
		}
next:
		inputtoken = strtok_r(NULL, ",", &saveptrinput);
	}

	if (!atleast_one_socket_opened) {
		goto shutdown;
	}

	if (evctx && pthread_create(&thread_main_loop[0], NULL, input_loop, (void *)callback_object) != 0)
	{
		fprintf(stderr, "Could not start udp receiver thread\n");
		__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not start udp receiver thread");
		goto shutdown;
	}
	thread_started[0] = true;

	for (size_t i = 0; i < MAX_INPUT_COUNT; i++) {
		if (callback_object[i].sender_ctx && rist_start(callback_object[i].sender_ctx) == -1) {
			__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not start rist sender");
			goto shutdown;
		}
		if (callback_object[i].receiver_ctx && rist_start(callback_object[i].receiver_ctx) == -1) {
			__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not start rist receiver");
			goto shutdown;
		}
		if (callback_object[i].receiver_ctx && pthread_create(&thread_main_loop[i+1], NULL, input_loop, (void *)callback_object) != 0)
		{
			fprintf(stderr, "Could not start send rist receiver thread\n");
			__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not start send rist receiver thread");
			goto shutdown;
		}
		thread_started[i+1] = true;
	}

#ifdef _WIN32
		system("pause");
#else
		pause();
#endif

shutdown:

	if (udp_config) {
		rist_udp_config_free2(&udp_config);
	}
	for (size_t i = 0; i < MAX_INPUT_COUNT; i++) {
		// Remove socket events
		if (event[i])
			evsocket_delevent(callback_object[i].evctx, event[i]);
		// Free udp_config object
		if ((void *)callback_object[i].udp_config)
			rist_udp_config_free2(&callback_object[i].udp_config);
		// Cleanup rist listeners
		if (callback_object[i].receiver_ctx)
			rist_destroy(callback_object[i].receiver_ctx);
		// Cleanup rist sender and their peers
		if (callback_object[i].sender_ctx)
			rist_destroy(callback_object[i].sender_ctx);
	}

	for (size_t i = 0; i <= MAX_INPUT_COUNT; i++) {
		if (thread_started[i])
			pthread_join(thread_main_loop[i], NULL);
	}

	rist_logging_unset_global();
	if (inputurl)
		free(inputurl);
	if (outputurl)
		free(outputurl);
	if (oobtun)
		free(oobtun);
	if (shared_secret)
		free(shared_secret);

	return 0;
}

static struct rist_peer* setup_rist_peer(struct rist_sender_args *setup)
{
	if (rist_stats_callback_set(setup->ctx, setup->statsinterval, cb_stats, NULL) == -1) {
		__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not enable stats callback");
		return NULL;
	}

	if (rist_auth_handler_set(setup->ctx, cb_auth_connect, cb_auth_disconnect, setup->ctx) < 0) {
	    __android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not initialize rist auth handler");
		return NULL;
	}

	if (rist_connection_status_callback_set(setup->ctx, connection_status_callback, NULL) == -1) {
		rist_log(&logging_settings, RIST_LOG_ERROR, "Could not initialize rist connection status callback\n");
		__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not initialize rist connection status callback");
		return NULL;
	}

	if (setup->profile != RIST_PROFILE_SIMPLE) {
		if (rist_oob_callback_set(setup->ctx, cb_recv_oob, setup->ctx) == -1) {
			__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not enable out-of-band data");
			return NULL;
		}
	}

	if (rist_stats_callback_set(setup->ctx, setup->statsinterval, cb_stats, NULL) == -1) {
		__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not enable stats callback");
		return NULL;
	}

	// Rely on the library to parse the url
	struct rist_peer_config *peer_config_link = NULL;
	if (rist_parse_address2(setup->token, (void *)&peer_config_link))
	{
		__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not parse peer options for sender: %s", setup->token);
		return NULL;
	}

	/* Process overrides */
	struct rist_peer_config *overrides_peer_config = (void *)peer_config_link;
	if (setup->shared_secret && peer_config_link->secret[0] == 0) {
		strncpy(overrides_peer_config->secret, setup->shared_secret, RIST_MAX_STRING_SHORT -1);
		if (setup->encryption_type)
			overrides_peer_config->key_size = setup->encryption_type;
		else if (!overrides_peer_config->key_size)
			overrides_peer_config->key_size = 128;
	}
	if (setup->buffer_size) {
		overrides_peer_config->recovery_length_min = setup->buffer_size;
		overrides_peer_config->recovery_length_max = setup->buffer_size;
	}
	if (setup->stream_id) {
		if (setup->stream_id % 2 != 0) {
		    __android_log_print(ANDROID_LOG_ERROR, "RIST", "Error parsing peer options for sender: %s, stream-id (%d) must be even!", setup->token, setup->stream_id);
			return NULL;
		}
		else {
			overrides_peer_config->virt_dst_port = setup->stream_id;
		}
	}

	/* Print config */
    __android_log_print(ANDROID_LOG_INFO, "RIST", "Link configured with maxrate=%d bufmin=%d bufmax=%d reorder=%d rttmin=%d rttmax=%d congestion_control=%d min_retries=%d max_retries=%d",
                                                  		peer_config_link->recovery_maxbitrate, peer_config_link->recovery_length_min, peer_config_link->recovery_length_max,
                                                  		peer_config_link->recovery_reorder_buffer, peer_config_link->recovery_rtt_min, peer_config_link->recovery_rtt_max,
                                                  		peer_config_link->congestion_control_mode, peer_config_link->min_retries, peer_config_link->max_retries);

	struct rist_peer *peer;
	if (rist_peer_create(setup->ctx, &peer, peer_config_link) == -1) {
		__android_log_print(ANDROID_LOG_ERROR, "RIST", "Could not add peer connector to %s", peer_config_link->address);
		free((void *)peer_config_link);
		return NULL;
	}

#if HAVE_MBEDTLS
	int srp_error = 0;
	if (setup->profile != RIST_PROFILE_SIMPLE) {
		if (strlen(peer_config_link->srp_username) > 0 && strlen(peer_config_link->srp_password) > 0)
		{
			srp_error = rist_enable_eap_srp(peer, peer_config_link->srp_username, peer_config_link->srp_password, NULL, NULL);
			if (srp_error)
			{
				__android_log_print(ANDROID_LOG_WARN, "RIST", "Error %d trying to enable SRP for peer", srp_error);
			}
		}
		if (srpfile)
		{
			srp_error = rist_enable_eap_srp(peer, NULL, NULL, user_verifier_lookup, srpfile);
			if (srp_error)
			{
				__android_log_print(ANDROID_LOG_WARN, "RIST", "Error %d trying to enable SRP global authenticator, file %s", srp_error, srpfile);
			}
		}
	}
	else
	{
		__android_log_print(ANDROID_LOG_WARN, "RIST", "SRP Authentication is not available for Rist Simple Profile\n");
	}
#endif

	rist_peer_config_free2(&peer_config_link);

	return peer;
}

static void input_udp_recv(struct evsocket_ctx *evctx, int fd, short revents, void *arg)
{
	struct rist_callback_object *callback_object = (void *) arg;
	RIST_MARK_UNUSED(evctx);
	RIST_MARK_UNUSED(revents);
	RIST_MARK_UNUSED(fd);

	ssize_t recv_bufsize = -1;
	struct sockaddr_in addr4 = {0};
	struct sockaddr_in6 addr6 = {0};
	//struct sockaddr *addr;
	uint8_t *recv_buf = callback_object->recv;

	uint16_t address_family = (uint16_t)callback_object->udp_config->address_family;
	if (address_family == AF_INET6) {
		socklen_t addrlen = sizeof(struct sockaddr_in6);
		recv_bufsize = udpsocket_recvfrom(callback_object->sd, recv_buf, RIST_MAX_PACKET_SIZE, MSG_DONTWAIT, (struct sockaddr *) &addr6, &addrlen);
		__android_log_print(ANDROID_LOG_DEBUG, "RIST_DEBUG", "INET6 buf_size : %d", recv_bufsize);
		//addr = (struct sockaddr *) &addr6;
	} else {
		socklen_t addrlen = sizeof(struct sockaddr_in);
		recv_bufsize = udpsocket_recvfrom(callback_object->sd, recv_buf, RIST_MAX_PACKET_SIZE, MSG_DONTWAIT, (struct sockaddr *) &addr4, &addrlen);
		__android_log_print(ANDROID_LOG_DEBUG, "RIST_DEBUG", "buf_size : %d", recv_bufsize);
		//addr = (struct sockaddr *) &addr4;
	}

	if (recv_bufsize > 0) {
		ssize_t offset = 0;
		struct rist_data_block data_block = { 0 };
		// Delegate ts_ntp to the library by default.
		// If we wanted to be more accurate, we could use the kernel nic capture timestamp (linux)
		data_block.ts_ntp = 0;
		data_block.flags = 0;
		if (callback_object->udp_config->rtp_timestamp && recv_bufsize > 12)
		{
			// Extract timestamp from rtp header
			//uint32_t rtp_time = (recv_buf[4] << 24) | (recv_buf[5] << 16) | (recv_buf[6] << 8) | recv_buf[7];
			// Convert to NTP (assumes 90Khz)
			//data_block.ts_ntp = risttools_convertRTPtoNTP(rtp_time);
			// TODO: Figure out why this does not work (commenting out for now)
		}
		if (callback_object->udp_config->rtp_sequence && recv_bufsize > 12)
		{
			// Extract sequence number from rtp header
			//data_block.seq = (uint64_t)((recv_buf[2] << 8) | recv_buf[3]);
			//data_block.flags = RIST_DATA_FLAGS_USE_SEQ;
			// TODO: Figure out why this does not work (commenting out for now)
		}
		if (callback_object->udp_config->rtp && recv_bufsize > 12)
			offset = 12; // TODO: check for header extensions and remove them as well
		data_block.payload = recv_buf + offset;
		data_block.payload_len = recv_bufsize - offset;
		if (peer_connected_count) {
		    __android_log_print(ANDROID_LOG_DEBUG, "RIST_DEBUG", "RECV_BUF : %s", recv_buf);
			int w = rist_sender_data_write(callback_object->sender_ctx, &data_block);
			// TODO: report error?
			(void) w;
		}
	}
	else
	{
		// EWOULDBLOCK = EAGAIN = 11 would be the most common recoverable error (if any)
		if (errno != EWOULDBLOCK)
		{
			__android_log_print(ANDROID_LOG_ERROR, "RIST", "Input receive failed: errno=%d, ret=%d, socket=%d", errno, recv_bufsize, callback_object->sd);
		}
	}
}

static void input_udp_sockerr(struct evsocket_ctx *evctx, int fd, short revents, void *arg)
{
	struct rist_callback_object *callback_object = (void *) arg;
	RIST_MARK_UNUSED(evctx);
	RIST_MARK_UNUSED(revents);
	RIST_MARK_UNUSED(fd);
	__android_log_print(ANDROID_LOG_ERROR, "RIST", "Socket error on sd=%d, stream-id=%d !\n", callback_object->sd, callback_object->udp_config->stream_id);
}

static PTHREAD_START_FUNC(input_loop, arg)
{
	struct rist_callback_object *callback_object = (void *) arg;
	// This is my main loop (one thread per receiver)
	while (!signalReceived) {
		if (callback_object->receiver_ctx)
		{
			// RIST receiver
			struct rist_data_block *b = NULL;
			int queue_size = rist_receiver_data_read2(callback_object->receiver_ctx, &b, 5);
			if (queue_size > 0) {
				if (queue_size % 10 == 0 || queue_size > 50)
				{
					__android_log_print(ANDROID_LOG_WARN, "RIST", "Falling behind on rist_receiver_data_read: %d", queue_size);
				}
				if (b && b->payload) {
					if (peer_connected_count) {
						int w = rist_sender_data_write(callback_object->sender_ctx, b);
						// TODO: report error?
						(void) w;
					}
					rist_receiver_data_block_free2(&b);
				}
			}
		}
		else
		{
			// UDP recevier. Infinite wait, 100 socket events
			evsocket_loop_single(callback_object->evctx, 5, 100);
		}
	}
	return 0;
}

static int cb_stats(void *arg, const struct rist_stats *stats_container)
{
	(void)arg;
	__android_log_print(ANDROID_LOG_INFO, "RIST", "%s", stats_container->stats_json);
	rist_stats_free(stats_container);
	return 0;
}

static int cb_auth_connect(void *arg, const char* connecting_ip, uint16_t connecting_port, const char* local_ip, uint16_t local_port, struct rist_peer *peer)
{
	struct rist_ctx *ctx = (struct rist_ctx *)arg;
	char buffer[500];
	char message[200];
	int message_len = snprintf(message, 200, "auth,%s:%d,%s:%d", connecting_ip, connecting_port, local_ip, local_port);
	// To be compliant with the spec, the message must have an ipv4 header
	int ret = oob_build_api_payload(buffer, (char *)connecting_ip, (char *)local_ip, message, message_len);
	//__andorid_log_print(ANDROID_LOG_INFO, "RIST", "Peer has been peer_connected_count, sending oob/api message: %s", message);
	struct rist_oob_block oob_block;
	oob_block.peer = peer;
	oob_block.payload = buffer;
	oob_block.payload_len = ret;
	rist_oob_write(ctx, &oob_block);
	return 0;
}

static int cb_auth_disconnect(void *arg, struct rist_peer *peer)
{
	(void)peer;
	struct rist_ctx *ctx = (struct rist_ctx *)arg;
	(void)ctx;
	return 0;
}

static void connection_status_callback(void *arg, struct rist_peer *peer, enum rist_connection_status peer_connection_status)
{
	(void)arg;
	if (peer_connection_status == RIST_CONNECTION_ESTABLISHED || peer_connection_status == RIST_CLIENT_CONNECTED)
		peer_connected_count++;
	else
		peer_connected_count--;
	rist_log(&logging_settings, RIST_LOG_INFO,"Connection Status changed for Peer %"PRIu64", new status is %d, peer connected count is %d\n",
				peer, peer_connection_status, peer_connected_count);
}

static int cb_recv_oob(void *arg, const struct rist_oob_block *oob_block)
{
	struct rist_ctx *ctx = (struct rist_ctx *)arg;
	(void)ctx;
	int message_len = 0;
	char *message = oob_process_api_message((int)oob_block->payload_len, (char *)oob_block->payload, &message_len);
	if (message) {
		__android_log_print(ANDROID_LOG_INFO, "RIST", "Out-of-band api data received: %.*s", message_len, message);
	}
	return 0;
}