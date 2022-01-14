//
// Created by limyj7783 on 2022-01-14.
//

#ifndef RISTSENDER_RISTFUNCTION_H
#define RISTSENDER_RISTFUNCTION_H

#include "RISTDefine.h"

//RIST Receiver Func
int rist_receiver_create(struct rist_ctx **_ctx, enum rist_profile profile, struct rist_logging_settings *logging_settings);
int rist_receiver_nack_type_set(struct rist_ctx *rist_ctx, enum rist_nack_type nack_type);
static struct rist_flow *rist_get_longest_flow(struct rist_receiver *ctx, ssize_t *num);
int rist_receiver_data_read(struct rist_ctx *ctx, const struct rist_data_block **data_block, int timeout);
int rist_receiver_data_read2(struct rist_ctx *rist_ctx, struct rist_data_block **data_buffer, int timeout);
void rist_receiver_data_block_free(struct rist_data_block **const block);
void rist_receiver_data_block_free2(struct rist_data_block **block);
uint32_t rist_flow_id_create();
int rist_receiver_data_notify_fd_set(struct rist_ctx *rist_ctx, int fd);
int rist_connection_status_callback_set(struct rist_ctx *ctx, connection_status_callback_t connection_status_callback, void *arg);
int rist_receiver_data_callback_set(struct rist_ctx *rist_ctx,receiver_data_callback_t data_callback, void *arg);
int rist_receiver_data_callback_set2(struct rist_ctx *rist_ctx, receiver_data_callback2_t data_callback, void *arg);

//RIST Sender Func
int rist_sender_create(struct rist_ctx **_ctx, enum rist_profile profile, uint32_t flow_id, struct rist_logging_settings *logging_settings);
int rist_sender_flow_id_get(struct rist_ctx *rist_ctx, uint32_t *flow_id);
int rist_sender_npd_enable(struct rist_ctx *rist_ctx);
int rist_sender_npd_disable(struct rist_ctx *rist_ctx);
int rist_sender_flow_id_set(struct rist_ctx *rist_ctx, uint32_t flow_id);
int rist_sender_data_write(struct rist_ctx *rist_ctx, const struct rist_data_block *data_block);

//Shared OOB Func
int rist_oob_read(struct rist_ctx *ctx, const struct rist_oob_block **oob_block);
int rist_oob_write(struct rist_ctx *ctx, const struct rist_oob_block *oob_block);
int rist_oob_callback_set(struct rist_ctx *ctx, int (*oob_callback)(void *arg, const struct rist_oob_block *oob_block), void *arg);

//Shared Func
int rist_jitter_max_set(struct rist_ctx *ctx, int t);
int rist_auth_handler_set(struct rist_ctx *ctx, int (*conn_cb)(void *arg, const char *connecting_ip, uint16_t connecting_port, const char *local_ip, uint16_t local_port, struct rist_peer *peer),
						  int (*disconn_cb)(void *arg, struct rist_peer *peer), void *arg);
int rist_stats_free(const struct rist_stats *stats_container);
int rist_peer_config_free(const struct rist_peer_config **peer_config);
int rist_peer_config_free2(struct rist_peer_config **peer_config);
int rist_logging_settings_free(const struct rist_logging_settings **logging_settings);
int rist_logging_settings_free2(struct rist_logging_settings **logging_settings);
int rist_udp_config_free(const struct rist_udp_config **udp_config);
int rist_udp_config_free2(struct rist_udp_config **udp_config);
int rist_stats_callback_set(struct rist_ctx *ctx, int statsinterval, int (*stats_cb)(void *arg, const struct rist_stats *stats_container), void *arg);

//Utility Func
const char *librist_version(void);
const char *librist_api_version(void);
int rist_parse_udp_address(const char *url, const struct rist_udp_config **udp_config);
int rist_parse_udp_address2(const char *url, struct rist_udp_config **udp_config);
int rist_peer_config_defaults_set(struct rist_peer_config *peer_config);
int rist_parse_address(const char *url, const struct rist_peer_config **peer_config);
int rist_parse_address2(const char *url, struct rist_peer_config **peer_config);

//Shared functions with specific implementations
static int rist_receiver_peer_create(struct rist_receiver *ctx, struct rist_peer **peer, const struct rist_peer_config *config);
static int rist_sender_peer_create(struct rist_sender *ctx, struct rist_peer **peer, const struct rist_peer_config *config);
int rist_peer_create(struct rist_ctx *ctx, struct rist_peer **peer, const struct rist_peer_config *config);
int rist_peer_weight_set(struct rist_ctx *ctx, struct rist_peer *peer, const uint32_t weight);
int rist_peer_destroy(struct rist_ctx *ctx, struct rist_peer *peer);
static int rist_sender_start(struct rist_sender *ctx);
static int rist_receiver_start(struct rist_receiver *ctx);
int rist_start(struct rist_ctx *ctx);
static int rist_sender_destroy(struct rist_sender *ctx);
static int rist_receiver_destroy(struct rist_receiver *ctx);
int rist_destroy(struct rist_ctx *ctx);
int rist_receiver_set_output_fifo_size(struct rist_ctx *ctx, uint32_t desired_size);
int rist_set_opt(struct rist_ctx *ctx, enum rist_opt opt, void* optval1, void* optval2, void* optval3);

#endif //RISTSENDER_RISTFUNCTION_H
