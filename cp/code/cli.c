#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>

#include <unistd.h>

#include "core_module.h"

#define PROPERTY_COUNT 10
enum l2_property properties[] = {L2_PROPERTY_UNDEFINED, L2_PROPERTY_UNIQUE_SADDRS_COUNT, L2_PROPERTY_CATCHED_PACKETS_COUNT, L2_PROPERTY_TOT_LEN, 
    L2_PROPERTY_AVG_CATCHED_PACKETS, L2_PROPERTY_MAX_CATCHED_PACKETS, L2_PROPERTY_AVG_LEN, L2_PROPERTY_MAX_LEN, L2_PROPERTY_TOT_TCP, L2_PROPERTY_TOT_SYN};
char *property_names[] = {"undefined", "saddr_count", "catched_packet_count", "tot_len", "avg_catched_packets",
 "max_catched_packets", "avg_len", "max_len", "tot_tcp", "tot_syn"};
char *get_property_name(enum l2_property prop) {
    for (int i = 0; i < PROPERTY_COUNT; i++) {
        if (properties[i] == prop) {
            return property_names[i];
        }
    }
    return property_names[0];
}

#define crit_behaviour_TYPE_COUNT 3
enum l2_crit_behaviour_type crit_behaviour_types[] = {L2_CRIT_BEHAVIOUR_TYPE_UNDEFINED, L2_CRIT_BEHAVIOUR_TYPE_INCREASE, L2_CRIT_BEHAVIOUR_TYPE_FALL};
char *crit_behaviour_type_names[] = {"undefined", "increase", "fall"};
char *get_crit_behaviour_type_name(enum l2_crit_behaviour_type at) {
    for (int i = 0; i < crit_behaviour_TYPE_COUNT; i++) {
        if (crit_behaviour_types[i] == at) {
            return crit_behaviour_type_names[i];
        }
    }
    return crit_behaviour_type_names[0];
}

void print_info(void) {
    printf(
        "\nby Gurova N.A.\n\n\n"
        "\tCOMMANDS:\n\n"
        "\t\thide       ---     hide\n"
        "\t\tunhide     ---     unhide\n\n"
        "\t\tinfo       ---     show info\n"
        "\t\tstats      ---     show stats\n"
        "\t\tl2         ---     show l2_data\n\n"
    );
}

void print_l2_data_slice(struct l2_data_slice *l2_ds) {
    printf("{"
    "\"from_time\": %ld,"
    "\"unique_saddr_count\": %d,"
    "\"catched_packets_count\": %d,"
    "\"tot_len\": %d,"
    "\"avg_catched_packets\": %d,"
    "\"max_catched_packets\": %d,"
    "\"avg_len\": %d,"
    "\"max_len\": %d,"
    "\"crit\": %d,"
    "\"property\": \"%s\","
    "\"crit_type\": \"%s\","
    "\"tot_tcp\": \"%d\","
    "\"tot_syn\": \"%d\""
    "}\n", l2_ds->from_time, l2_ds->unique_saddr_count, l2_ds->catched_packets_count,
    l2_ds->tot_len, l2_ds->avg_catched_packets, l2_ds->max_catched_packets, l2_ds->avg_len,
    l2_ds->max_len, l2_ds->crit_behaviour, get_property_name(l2_ds->property), get_crit_behaviour_type_name(l2_ds->type),
    l2_ds->tot_tcp, l2_ds->tot_syn);
}

int show_l2_data()
{
    struct l2_data_slice ds;
    int rb;

    int fd = open(L2_DATA_DEV_PATH, O_RDONLY);
    if (fd < 0) return 1;

    size_t l2_ds_size = sizeof(struct l2_data_slice);

    while ((rb = read(fd, &ds, l2_ds_size)) > 0) {
        if (rb != l2_ds_size) break;

        print_l2_data_slice(&ds);
	}

    close(fd);

    return 0;
}

int show_stats()
{
    struct module_stats stats;
    int rb;

    int fd = open(STATS_DEV_PATH, O_RDONLY);
    if (fd < 0) return 1;

    size_t stats_size = sizeof(struct module_stats);

    if (read(fd, &stats, stats_size) != stats_size) {
        close(fd);
        return 1;
    }

    close(fd);

    printf(
        "MODULE STATS:\n\n"
        "\tmax history length\t\t\t\t%d\n"
        "\tactual history length\t\t\t\t%d\n"
        "\tcurrent history time periud\t\t\t%ld\n"
        "\tcritical activity periuds\t\t\t%d\n"
        "\tfirst critical activity timestamp\t\t%ld\n"
        "\tlast critical activity timestamp\t\t%ld\n",
        stats.history_length, stats.current_history_length, stats.current_periud_ns,
        stats.crit_behaviour_count, stats.first_crit_behaviour_ns, stats.last_crit_behaviour_ns
    );

    return 0;
}

void hide_unhide(int make_hidden) {
    struct command cmd;
    if (make_hidden == 1) {
        cmd.type = HIDE;
    } else {
        cmd.type = UNHIDE;
    }

    int fd = open(CTRL_DEV_PATH, O_WRONLY | O_APPEND);
    if (fd < 0) return;

    write(fd, &cmd, sizeof(struct command));

    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        print_info();
        return 1;
    }

    if (!strcmp(argv[1], "info")) {
        print_info();
        return 0;
    }

    if (!strcmp(argv[1], "stats")) {
        return show_stats();
    }

    if (!strcmp(argv[1], "l2")) {
        return show_l2_data();
    }

    if (!strcmp(argv[1], "hide")) {
        hide_unhide(1);
        return 0;
    }

    if (!strcmp(argv[1], "unhide")) {
        hide_unhide(0);
        return 0;
    }

    print_info();
    return 1;
}