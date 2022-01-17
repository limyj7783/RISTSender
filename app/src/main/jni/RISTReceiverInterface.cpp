//
// Created by limyj7783 on 2022-01-17.
//

JNIEXPORT jint JNICALL Java_com_example_ristsender_RIST_ReceiveStart(JNIEnv *env, jobject thiz)
{
	int option_index;
	int c;
	int data_read_mode = DATA_READ_MODE_CALLBACK;
	const struct rist_peer_config *peer_input_config[MAX_INPUT_COUNT];
	char *inputurl = NULL;
	char *outputurl = NULL;
	char *oobtun = NULL;
	char *shared_secret = NULL;
	int buffer = 0;
	int encryption_type = 0;
	struct rist_callback_object callback_object;
	enum rist_log_level loglevel = RIST_LOG_INFO;
	int statsinterval = 1000;
	char *remote_log_address = NULL;
	if (pthread_mutex_init(&signal_lock, NULL) != 0)
	{
		fprintf(stderr, "Could not initialize signal lock\n");
		exit(1);
	}

#if HAVE_MBEDTLS
	FILE *srpfile = NULL;
#endif

	for (size_t i = 0; i < MAX_OUTPUT_COUNT; i++)
	{
		callback_object.mpeg[i] = 0;
		callback_object.udp_config[i] = NULL;
	}
	// Default log settings
    struct rist_logging_settings *log_ptr = &logging_settings;
    if (rist_logging_set(&log_ptr, loglevel, NULL, NULL, NULL,
                         stderr) != 0) {
      fprintf(stderr, "Failed to setup default logging!\n");
      exit(1);
	}

	rist_log(&logging_settings, RIST_LOG_INFO, "Starting ristreceiver version: %s libRIST library: %s API version: %s\n", LIBRIST_VERSION, librist_version(), librist_api_version());

	// Update log settings with custom loglevel and remote address if necessary
	if (rist_logging_set(&log_ptr, loglevel, NULL, NULL, remote_log_address, stderr) != 0) {
		fprintf(stderr,"Failed to setup logging!\n");
		exit(1);
	}

	/* rist side */

	struct rist_ctx *ctx;
	if (rist_receiver_create(&ctx, profile, &logging_settings) != 0) {
		rist_log(&logging_settings, RIST_LOG_ERROR, "Could not create rist receiver context\n");
		exit(1);
	}

	if (rist_auth_handler_set(ctx, cb_auth_connect, cb_auth_disconnect, ctx) != 0) {
		rist_log(&logging_settings, RIST_LOG_ERROR, "Could not init rist auth handler\n");
		exit(1);
	}

	if (rist_connection_status_callback_set(ctx, connection_status_callback, NULL) == -1) {
		rist_log(&logging_settings, RIST_LOG_ERROR, "Could not initialize rist connection status callback\n");
		exit(1);
	}

	if (profile != RIST_PROFILE_SIMPLE) {
		if (rist_oob_callback_set(ctx, cb_recv_oob, ctx) == -1) {
			rist_log(&logging_settings, RIST_LOG_ERROR, "Could not add enable out-of-band data\n");
			exit(1);
		}
	}

	if (rist_stats_callback_set(ctx, statsinterval, cb_stats, NULL) == -1) {
		rist_log(&logging_settings, RIST_LOG_ERROR, "Could not enable stats callback\n");
		exit(1);
	}

	char *saveptr1;
	char *inputtoken = strtok_r(inputurl, ",", &saveptr1);
	for (size_t i = 0; i < MAX_INPUT_COUNT; i++) {
		if (!inputtoken)
			break;

		// Rely on the library to parse the url
		struct rist_peer_config *peer_config = NULL;
		if (rist_parse_address2(inputtoken, &peer_config))
		{
			rist_log(&logging_settings, RIST_LOG_ERROR, "Could not parse peer options for receiver #%d\n", (int)(i + 1));
			exit(1);
		}

		/* Process overrides */
		struct rist_peer_config *overrides_peer_config = peer_config;
		if (shared_secret && peer_config->secret[0] == 0) {
			strncpy(overrides_peer_config->secret, shared_secret, RIST_MAX_STRING_SHORT -1);
			if (encryption_type)
				overrides_peer_config->key_size = encryption_type;
			else if (!overrides_peer_config->key_size)
				overrides_peer_config->key_size = 128;
		}
		if (buffer) {
			overrides_peer_config->recovery_length_min = buffer;
			overrides_peer_config->recovery_length_max = buffer;
		}

		/* Print config */
		rist_log(&logging_settings, RIST_LOG_INFO, "Link configured with maxrate=%d bufmin=%d bufmax=%d reorder=%d rttmin=%d rttmax=%d congestion_control=%d min_retries=%d max_retries=%d\n",
			peer_config->recovery_maxbitrate, peer_config->recovery_length_min, peer_config->recovery_length_max,
			peer_config->recovery_reorder_buffer, peer_config->recovery_rtt_min,peer_config->recovery_rtt_max,
			peer_config->congestion_control_mode, peer_config->min_retries, peer_config->max_retries);

		peer_input_config[i] = peer_config;

		struct rist_peer *peer;
		if (rist_peer_create(ctx, &peer, peer_input_config[i]) == -1) {
			rist_log(&logging_settings, RIST_LOG_ERROR, "Could not add peer connector to receiver #%i\n", (int)(i + 1));
			exit(1);
		}
#if HAVE_MBEDTLS
		int srp_error = 0;
		if (profile != RIST_PROFILE_SIMPLE) {
			if (strlen(peer_config->srp_username) > 0 && strlen(peer_config->srp_password) > 0)
			{
				srp_error = rist_enable_eap_srp(peer, peer_config->srp_username, peer_config->srp_password, NULL, NULL);
				if (srp_error)
					rist_log(&logging_settings, RIST_LOG_WARN, "Error %d trying to enable SRP for peer\n", srp_error);
			}
			if (srpfile)
			{
				srp_error = rist_enable_eap_srp(peer, NULL, NULL, user_verifier_lookup, srpfile);
				if (srp_error)
					rist_log(&logging_settings, RIST_LOG_WARN, "Error %d trying to enable SRP global authenticator, file %s\n", srp_error, srpfile);
			}
		}
		else
			rist_log(&logging_settings, RIST_LOG_WARN, "SRP Authentication is not available for Rist Simple Profile\n");
#endif

		rist_peer_config_free2(&peer_config);
		inputtoken = strtok_r(NULL, ",", &saveptr1);
	}

	/* Mpeg side */
	bool atleast_one_socket_opened = false;
	char *saveptr2;
	char *outputtoken = strtok_r(outputurl, ",", &saveptr2);
	for (size_t i = 0; i < MAX_OUTPUT_COUNT; i++) {

		if (!outputtoken)
			break;

		// First parse extra parameters (?miface=lo&stream-id=1971) and separate the address
		// We are using the rist_parse_address function to create a config object that does not really
		// belong to the udp output. We do this only to avoid writing another parser for the two url
		// parameters available to the udp input/output url
		struct rist_udp_config *udp_config = NULL;
		if (rist_parse_udp_address2(outputtoken, &udp_config)) {
			rist_log(&logging_settings, RIST_LOG_ERROR, "Could not parse outputurl %s\n", outputtoken);
			goto next;
		}

		// Now parse the address 127.0.0.1:5000
		char hostname[200] = {0};
		int outputlisten;
		uint16_t outputport;
		if (udpsocket_parse_url((void *)udp_config->address, hostname, 200, &outputport, &outputlisten) || !outputport || strlen(hostname) == 0) {
			rist_log(&logging_settings, RIST_LOG_ERROR, "Could not parse output url %s\n", outputtoken);
			goto next;
		}
		rist_log(&logging_settings, RIST_LOG_INFO, "URL parsed successfully: Host %s, Port %d\n", (char *) hostname, outputport);

		// Open the output socket
		callback_object.mpeg[i] = udpsocket_open_connect(hostname, outputport, udp_config->miface);
		if (callback_object.mpeg[i] < 0) {
			rist_log(&logging_settings, RIST_LOG_ERROR, "Could not connect to: Host %s, Port %d\n", (char *) hostname, outputport);
			goto next;
		} else {
			rist_log(&logging_settings, RIST_LOG_INFO, "Output socket is open and bound %s:%d\n", (char *) hostname, outputport);
			atleast_one_socket_opened = true;
		}
		callback_object.udp_config[i] = udp_config;

next:
		outputtoken = strtok_r(NULL, ",", &saveptr2);
	}

	if (!atleast_one_socket_opened) {
		exit(1);
	}

	if (data_read_mode == DATA_READ_MODE_CALLBACK) {
		if (rist_receiver_data_callback_set2(ctx, cb_recv, &callback_object))
		{
			rist_log(&logging_settings, RIST_LOG_ERROR, "Could not set data_callback pointer\n");
			exit(1);
		}
	}
#ifndef _WIN32
	else if (data_read_mode == DATA_READ_MODE_POLL) {
		if (pipe(receiver_pipe))
		{
			rist_log(&logging_settings, RIST_LOG_ERROR, "Could not create pipe for file descriptor channel\n");
			exit(1);
		}
		if (fcntl(receiver_pipe[WriteEnd], F_SETFL, O_NONBLOCK) < 0)
		{
			rist_log(&logging_settings, RIST_LOG_ERROR, "Could not set pipe to non blocking mode\n");
 			exit(1);
 		}
		if (fcntl(receiver_pipe[ReadEnd], F_SETFL, O_NONBLOCK) < 0)
		{
			rist_log(&logging_settings, RIST_LOG_ERROR, "Could not set pipe to non blocking mode\n");
 			exit(1);
 		}
		if (rist_receiver_data_notify_fd_set(ctx, receiver_pipe[WriteEnd]))
		{
			rist_log(&logging_settings, RIST_LOG_ERROR, "Could not set file descriptor channel\n");
			exit(1);
		}
	}
#endif

	if (rist_start(ctx)) {
		rist_log(&logging_settings, RIST_LOG_ERROR, "Could not start rist receiver\n");
		exit(1);
	}
	/* Start the rist protocol thread */
	if (data_read_mode == DATA_READ_MODE_CALLBACK) {
#ifdef _WIN32
		system("pause");
#else
		pause();
#endif
	}
	else if (data_read_mode == DATA_READ_MODE_API) {
#ifndef _WIN32
		int prio_max = sched_get_priority_max(SCHED_RR);
		struct sched_param param = { 0 };
		param.sched_priority = prio_max;
		if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) != 0)
			rist_log(&logging_settings, RIST_LOG_WARN, "Failed to set data output thread to RR scheduler with prio of %i\n", prio_max);
#else
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif
		// Master loop
		while (true)
		{
			struct rist_data_block *b = NULL;
			int queue_size = rist_receiver_data_read2(ctx, &b, 5);
			if (queue_size > 0) {
				if (queue_size % 10 == 0 || queue_size > 50) {
					// We need a better way to report on this
					uint32_t flow_id = b ? b->flow_id : 0;
					rist_log(&logging_settings, RIST_LOG_WARN, "Falling behind on rist_receiver_data_read: count %d, flow id %u\n", queue_size, flow_id);
				}
				if (b && b->payload) cb_recv(&callback_object, b);
			}
			pthread_mutex_lock(&signal_lock);
			if (signalReceived)
			{
				rist_log(&logging_settings, RIST_LOG_INFO, "Signal %d received\n", signal);
				break;
			}
			pthread_mutex_unlock(&signal_lock);
		}
	}
#ifndef _WIN32
	else if (data_read_mode == DATA_READ_MODE_POLL) {
		char pipebuffer[256];
		fd_set readfds;
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 5000;
		while (true) {
			FD_ZERO(&readfds);
			FD_SET(receiver_pipe[ReadEnd], &readfds);
			int ret = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout);
			if (ret == -1 && errno != EINTR) {
				fprintf(stderr, "Pipe read error %d, exiting\n", errno);
				break;
			}
			else if (ret == 0) {
				// Normal timeout (loop and wait)
				continue;
			}
			/* Consume bytes from pipe (irrelevant data) */
			for (;;) {
				if (read(receiver_pipe[ReadEnd], &pipebuffer, sizeof(pipebuffer)) <= 0) {
					if (errno != EAGAIN)
						fprintf(stderr, "Error reading data from pipe: %d\n", errno);
					break;
				}
			}
			/* Consume data from library */
			struct rist_data_block *b = NULL;
			int queue_size = 0;
			for (;;) {
				queue_size = rist_receiver_data_read2(ctx, &b, 0);
				if (queue_size > 0) {
					if (queue_size % 10 == 0 || queue_size > 50) {
						// We need a better way to report on this
						uint32_t flow_id = b ? b->flow_id : 0;
						rist_log(&logging_settings, RIST_LOG_WARN, "Falling behind on rist_receiver_data_read: count %d, flow id %u\n", queue_size, flow_id);
					}
					if (b && b->payload) cb_recv(&callback_object, b);
				}
				else
					break;
			}
			pthread_mutex_lock(&signal_lock);
			if (signalReceived)
			{
				rist_log(&logging_settings, RIST_LOG_INFO, "Signal %d received\n", signal);
				break;
			}
			pthread_mutex_unlock(&signal_lock);
		}
	}
#endif
	fprintf(stderr, "DESTROY\n");
	rist_destroy(ctx);

	for (size_t i = 0; i < MAX_OUTPUT_COUNT; i++) {
		// Free udp_config object
		if ((void *)callback_object.udp_config[i])
			rist_udp_config_free2(&callback_object.udp_config[i]);
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

	struct ristreceiver_flow_cumulative_stats *stats, *next;
	stats = stats_list;
	while (stats)
	{
		next = stats->next;
		free(stats);
		stats = next;
	}
	return 0;
}