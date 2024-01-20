#ifndef CORE_MODULE_H
#define CORE_MODULE_H


#define L2_DATA_DEV_PATH "/dev/cource_work_l2_data"
#define STATS_DEV_PATH "/dev/cource_work_stats"
#define CTRL_DEV_PATH "/dev/cource_work_ctrl"

enum command_type {
    UNDEFINED = 0,
    HIDE = 1,
    UNHIDE = 2,
};

struct command
{
    enum command_type type;
};

enum l2_property {
    L2_PROPERTY_UNDEFINED = 0,
    L2_PROPERTY_UNIQUE_SADDRS_COUNT = 1,
    L2_PROPERTY_CATCHED_PACKETS_COUNT = 2,
    L2_PROPERTY_TOT_LEN = 3,
    L2_PROPERTY_AVG_CATCHED_PACKETS = 4,
    L2_PROPERTY_MAX_CATCHED_PACKETS = 5,
    L2_PROPERTY_AVG_LEN = 6,
    L2_PROPERTY_MAX_LEN = 7,
    L2_PROPERTY_TOT_TCP = 8,
    L2_PROPERTY_TOT_SYN = 9,
};
enum l2_crit_behaviour_type {
    L2_CRIT_BEHAVIOUR_TYPE_UNDEFINED = 0,
    L2_CRIT_BEHAVIOUR_TYPE_INCREASE = 1,
    L2_CRIT_BEHAVIOUR_TYPE_FALL = 2,
};

struct l2_data_slice
{
    uint64_t from_time;
    uint32_t unique_saddr_count;
    uint32_t catched_packets_count;
    uint32_t tot_len;
    uint32_t avg_catched_packets;
    uint32_t max_catched_packets;
    uint32_t avg_len;
    uint32_t max_len;
    int crit_behaviour;
    enum l2_property property;
    enum l2_crit_behaviour_type type;
    uint32_t tot_tcp;
    uint32_t tot_syn;
};

struct module_stats
{
    uint32_t history_length;
    uint32_t current_history_length;
    uint64_t current_periud_ns;
    uint32_t crit_behaviour_count;
    uint64_t first_crit_behaviour_ns;
    uint64_t last_crit_behaviour_ns;
};

#endif //CORE_MODULE_H
