#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h> 
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/cdev.h> 
#include <linux/device.h>
#include <linux/types.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#include <linux/fcntl.h>
#include <linux/delay.h>
#include <linux/syscalls.h>

#include <linux/miscdevice.h>
#include <linux/stat.h>

#include <linux/string.h>
#include <linux/timekeeping.h>


#include "core_module.h"


#define MODULE_DMESG_PREFIX ">>>--------> "
#define DEVICE_L2_DATA_FNAME "cource_work_l2_data"
#define DEVICE_STATS_FNAME "cource_work_stats"
#define DEVICE_CTRL_FNAME "cource_work_ctrl"


#define IP_POS(ip, pos) (ip >> ((8 * (3 - pos))) & 0xFF)
#define SAME_ADDR(ip1, ip2) ((ip1 ^ ip2) == 0)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gurova N.A.");

uint64_t dump_ns_periud = 10000000000;

#define FIRST_LAYER_TABLE_LENGTH 64
struct first_layer_schema {
    uint32_t unique_saddr;
    uint32_t catched_packets_count;
    uint32_t tot_len;
    uint32_t catched_tcp;
    uint32_t catched_syn;
};
struct first_layer_schema first_layer_table[FIRST_LAYER_TABLE_LENGTH];
size_t first_layer_table_next_index = 0;
uint64_t from_time = 0; // ns

#define SECOND_LAYER_TABLE_LENGTH 512
enum crit_behaviour_property {
    CRIT_BEHAVIOUR_PROPERTY_UNDEFINED = 0,
    CRIT_BEHAVIOUR_PROPERTY_UNIQUE_SADDRS_COUNT = 1,
    CRIT_BEHAVIOUR_PROPERTY_CATCHED_PACKETS_COUNT = 2,
    CRIT_BEHAVIOUR_PROPERTY_TOT_LEN = 3,
    CRIT_BEHAVIOUR_PROPERTY_AVG_CATCHED_PACKETS = 4,
    CRIT_BEHAVIOUR_PROPERTY_MAX_CATCHED_PACKETS = 5,
    CRIT_BEHAVIOUR_PROPERTY_AVG_LEN = 6,
    CRIT_BEHAVIOUR_PROPERTY_MAX_LEN = 7,
    CRIT_BEHAVIOUR_PROPERTY_TOT_TCP = 8,
    CRIT_BEHAVIOUR_PROPERTY_TOT_SYN = 9,
};
enum crit_behaviour_type {
    CRIT_BEHAVIOUR_TYPE_UNDEFINED = 0,
    CRIT_BEHAVIOUR_TYPE_INCREASE = 1,
    CRIT_BEHAVIOUR_TYPE_FALL = 2,
};
struct second_layer_schema {
    uint64_t from_time;
    uint32_t unique_saddr_count;
    uint32_t catched_packets_count;
    uint32_t tot_len;
    uint32_t avg_catched_packets;
    uint32_t max_catched_packets;
    uint32_t avg_len;
    uint32_t max_len;
    int crit_behaviour;
    enum crit_behaviour_property property;
    enum crit_behaviour_type type;
    uint32_t tot_tcp;
    uint32_t tot_syn;
};
struct second_layer_schema second_layer_table[SECOND_LAYER_TABLE_LENGTH];
size_t second_layer_table_next_index = 0;
size_t second_layer_table_read_index = 0;

struct second_layer_schema initial_vec = {
    .unique_saddr_count = 10,
    .catched_packets_count = 100,
    .tot_len = 1000000,
    .avg_catched_packets = 50,
    .max_catched_packets = 50,
    .avg_len = 300000,
    .max_len = 300000,
    .tot_tcp = 100,
    .tot_syn = 100,
};

int eps_percent = 30;

void inspect_last(void) {
    if (second_layer_table_next_index < 2) {
        return;
    }

    struct second_layer_schema avg_values = {
        .unique_saddr_count = 0,
        .catched_packets_count = 0,
        .tot_len = 0,
        .avg_catched_packets = 0,
        .max_catched_packets = 0,
        .avg_len = 0,
        .max_len = 0,
        .tot_tcp = 0,
        .tot_syn = 0,
    };

    for (int i = 0; i < second_layer_table_next_index - 1; i++) {
        avg_values.unique_saddr_count += second_layer_table[i].unique_saddr_count;
        avg_values.catched_packets_count += second_layer_table[i].catched_packets_count;
        avg_values.tot_len += second_layer_table[i].tot_len;
        avg_values.avg_catched_packets += second_layer_table[i].avg_catched_packets;
        avg_values.max_catched_packets += second_layer_table[i].max_catched_packets;
        avg_values.avg_len += second_layer_table[i].avg_len;
        avg_values.max_len += second_layer_table[i].max_len;
        avg_values.tot_tcp += second_layer_table[i].tot_tcp;
        avg_values.tot_syn += second_layer_table[i].tot_syn;
    }

    avg_values.unique_saddr_count /= (second_layer_table_next_index - 1);
    avg_values.catched_packets_count /= (second_layer_table_next_index - 1);
    avg_values.tot_len /= (second_layer_table_next_index - 1);
    avg_values.avg_catched_packets /= (second_layer_table_next_index - 1);
    avg_values.max_catched_packets /= (second_layer_table_next_index - 1);
    avg_values.avg_len /= (second_layer_table_next_index - 1);
    avg_values.max_len /= (second_layer_table_next_index - 1);
    avg_values.tot_tcp /= (second_layer_table_next_index - 1);
    avg_values.tot_syn /= (second_layer_table_next_index - 1);

    struct second_layer_schema last = second_layer_table[second_layer_table_next_index - 1];

    uint64_t limit;

    limit = avg_values.unique_saddr_count * (100 + eps_percent) / 100 + initial_vec.unique_saddr_count;
    if (last.unique_saddr_count > limit) {
        second_layer_table[second_layer_table_next_index - 1].crit_behaviour = 1;
        second_layer_table[second_layer_table_next_index - 1].property = CRIT_BEHAVIOUR_PROPERTY_UNIQUE_SADDRS_COUNT;
        second_layer_table[second_layer_table_next_index - 1].type = CRIT_BEHAVIOUR_TYPE_INCREASE;
        return;
    }

    limit = avg_values.catched_packets_count * (100 + eps_percent) / 100 + initial_vec.catched_packets_count;
    if (last.catched_packets_count > limit) {
        second_layer_table[second_layer_table_next_index - 1].crit_behaviour = 1;
        second_layer_table[second_layer_table_next_index - 1].property = CRIT_BEHAVIOUR_PROPERTY_CATCHED_PACKETS_COUNT;
        second_layer_table[second_layer_table_next_index - 1].type = CRIT_BEHAVIOUR_TYPE_INCREASE;
        return;
    }

    limit = avg_values.tot_len * (100 + eps_percent) / 100 + initial_vec.tot_len;
    if (last.tot_len > limit) {
        second_layer_table[second_layer_table_next_index - 1].crit_behaviour = 1;
        second_layer_table[second_layer_table_next_index - 1].property = CRIT_BEHAVIOUR_PROPERTY_TOT_LEN;
        second_layer_table[second_layer_table_next_index - 1].type = CRIT_BEHAVIOUR_TYPE_INCREASE;
        return;
    }

    limit = avg_values.avg_catched_packets * (100 + eps_percent) / 100 + initial_vec.avg_catched_packets;
    if (last.avg_catched_packets > limit) {
        second_layer_table[second_layer_table_next_index - 1].crit_behaviour = 1;
        second_layer_table[second_layer_table_next_index - 1].property = CRIT_BEHAVIOUR_PROPERTY_AVG_CATCHED_PACKETS;
        second_layer_table[second_layer_table_next_index - 1].type = CRIT_BEHAVIOUR_TYPE_INCREASE;
        return;
    }

    limit = avg_values.max_catched_packets * (100 + eps_percent) / 100 + initial_vec.max_catched_packets;
    if (last.max_catched_packets > limit) {
        second_layer_table[second_layer_table_next_index - 1].crit_behaviour = 1;
        second_layer_table[second_layer_table_next_index - 1].property = CRIT_BEHAVIOUR_PROPERTY_MAX_CATCHED_PACKETS;
        second_layer_table[second_layer_table_next_index - 1].type = CRIT_BEHAVIOUR_TYPE_INCREASE;
        return;
    }

    limit = avg_values.avg_len * (100 + eps_percent) / 100 + initial_vec.avg_len;
    if (last.avg_len > limit) {
        second_layer_table[second_layer_table_next_index - 1].crit_behaviour = 1;
        second_layer_table[second_layer_table_next_index - 1].property = CRIT_BEHAVIOUR_PROPERTY_AVG_LEN;
        second_layer_table[second_layer_table_next_index - 1].type = CRIT_BEHAVIOUR_TYPE_INCREASE;
        return;
    }

    limit = avg_values.max_len * (100 + eps_percent) / 100 + initial_vec.max_len;
    if (last.max_len > limit) {
        second_layer_table[second_layer_table_next_index - 1].crit_behaviour = 1;
        second_layer_table[second_layer_table_next_index - 1].property = CRIT_BEHAVIOUR_PROPERTY_MAX_LEN;
        second_layer_table[second_layer_table_next_index - 1].type = CRIT_BEHAVIOUR_TYPE_INCREASE;
        return;
    }

    limit = avg_values.tot_tcp * (100 + eps_percent) / 100 + initial_vec.tot_tcp;
    if (last.tot_tcp > limit) {
        second_layer_table[second_layer_table_next_index - 1].crit_behaviour = 1;
        second_layer_table[second_layer_table_next_index - 1].property = CRIT_BEHAVIOUR_PROPERTY_TOT_TCP;
        second_layer_table[second_layer_table_next_index - 1].type = CRIT_BEHAVIOUR_TYPE_INCREASE;
        return;
    }

    limit = avg_values.tot_syn * (100 + eps_percent) / 100 + initial_vec.tot_syn;
    if (last.tot_syn > limit) {
        second_layer_table[second_layer_table_next_index - 1].crit_behaviour = 1;
        second_layer_table[second_layer_table_next_index - 1].property = CRIT_BEHAVIOUR_PROPERTY_TOT_SYN;
        second_layer_table[second_layer_table_next_index - 1].type = CRIT_BEHAVIOUR_TYPE_INCREASE;
        return;
    }
}

void dump_first_layer_table(void) {
    if (second_layer_table_next_index >= SECOND_LAYER_TABLE_LENGTH) {
        for (int i = 0; i < SECOND_LAYER_TABLE_LENGTH - 1; i++) {
            second_layer_table[i] = second_layer_table[i + 1];
        }

        second_layer_table_next_index -= 1;
    }

    if (second_layer_table_next_index < SECOND_LAYER_TABLE_LENGTH) {
        uint32_t catched_packets_count = 0;
        uint32_t tot_len = 0;
        uint32_t avg_catched_packets = 0;
        uint32_t max_catched_packets = 0;
        uint32_t avg_len = 0;
        uint32_t max_len = 0;
        uint32_t tot_tcp = 0;
        uint32_t tot_syn = 0;

        for (int i = 0; i < first_layer_table_next_index; i++) {
            catched_packets_count += first_layer_table[i].catched_packets_count;
            tot_len += first_layer_table[i].tot_len;
            if (max_catched_packets < first_layer_table[i].catched_packets_count) { max_catched_packets = first_layer_table[i].catched_packets_count; }
            if (max_len < first_layer_table[i].tot_len) { max_len = first_layer_table[i].tot_len; }

            tot_tcp += first_layer_table[i].catched_tcp;
            tot_syn += first_layer_table[i].catched_syn;
        }

        avg_catched_packets = catched_packets_count / first_layer_table_next_index;
        avg_len = tot_len / first_layer_table_next_index;

        second_layer_table[second_layer_table_next_index].from_time = ktime_get_seconds();
        second_layer_table[second_layer_table_next_index].unique_saddr_count = first_layer_table_next_index;
        second_layer_table[second_layer_table_next_index].catched_packets_count = catched_packets_count;
        second_layer_table[second_layer_table_next_index].tot_len = tot_len;
        second_layer_table[second_layer_table_next_index].avg_catched_packets = avg_catched_packets;
        second_layer_table[second_layer_table_next_index].max_catched_packets = max_catched_packets;
        second_layer_table[second_layer_table_next_index].avg_len = avg_len;
        second_layer_table[second_layer_table_next_index].max_len = max_len;

        second_layer_table[second_layer_table_next_index].crit_behaviour = 0;
        second_layer_table[second_layer_table_next_index].property = CRIT_BEHAVIOUR_PROPERTY_UNDEFINED;
        second_layer_table[second_layer_table_next_index].type = CRIT_BEHAVIOUR_TYPE_UNDEFINED;

        second_layer_table[second_layer_table_next_index].tot_tcp = tot_tcp;
        second_layer_table[second_layer_table_next_index].tot_syn = tot_syn;        

        first_layer_table_next_index = 0;
        second_layer_table_next_index += 1;

        inspect_last();
    }
}

void add_packet_info(uint32_t saddr, uint32_t tot_len, uint32_t is_tcp, uint32_t is_syn) {

    if (ktime_get_ns() - from_time > dump_ns_periud) {
        if (from_time != 0) dump_first_layer_table();

        from_time = ktime_get_ns();
    }

    int records_found = 0;

    for (int i = 0; i < first_layer_table_next_index; i++) {
        if (first_layer_table[i].unique_saddr == saddr) {
            records_found = 1;

            first_layer_table[i].catched_packets_count += 1;
            first_layer_table[i].tot_len += tot_len;

            if (is_tcp) first_layer_table[i].catched_tcp += 1;
            if (is_syn) first_layer_table[i].catched_syn += 1;

            break;
        }
    }

    if (records_found != 1 && first_layer_table_next_index < FIRST_LAYER_TABLE_LENGTH) {
        first_layer_table[first_layer_table_next_index].unique_saddr = saddr;
        first_layer_table[first_layer_table_next_index].catched_packets_count = 1;
        first_layer_table[first_layer_table_next_index].tot_len = tot_len;

        if (is_tcp) {
            first_layer_table[first_layer_table_next_index].catched_tcp = 1;
        } else {
            first_layer_table[first_layer_table_next_index].catched_tcp = 0;
        }

        if (is_syn) {
            first_layer_table[first_layer_table_next_index].catched_syn= 1;
        } else {
            first_layer_table[first_layer_table_next_index].catched_syn= 0;
        }

        first_layer_table_next_index += 1;
    }
}


static unsigned int catch_traffic(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;  /* An IPv4 packet header */
    struct tcphdr *tcph; /* An TCP packet header */
    uint32_t saddr;
    uint32_t tot_len;
    uint32_t is_tcp = 0;
    uint32_t is_syn = 0;

    if (!skb) return NF_ACCEPT;

    iph = (struct iphdr *)skb_network_header(skb);
    if (iph == NULL) return NF_ACCEPT;

    saddr = iph->saddr;
    tot_len = iph->tot_len;

    if (iph->protocol == IPPROTO_TCP) {
        tcph = (struct tcphdr *)(skb_transport_header(skb));
        is_tcp = 1;
        if (tcph->syn) is_syn = 1;
    }

    add_packet_info(saddr, tot_len, is_tcp, is_syn);

    return NF_ACCEPT;   /* the packet passes, continue iterations */
}

static struct nf_hook_ops module_hook_ops = 
{
    .hook = catch_traffic,
    .pf = PF_INET,
    .hooknum = NF_INET_LOCAL_IN,
    .priority = NF_IP_PRI_FIRST
};

// hide && unhide

struct list_head *module_prev;
int flag_hidden = 0;

void hide(void) {
    if (flag_hidden)
        return;

    module_prev = THIS_MODULE->list.prev;
    list_del(&THIS_MODULE->list);
    flag_hidden = 1;

    printk("module was hidden");
}

void unhide(void) {
    if (!flag_hidden)
        return;

    list_add(&THIS_MODULE->list, module_prev);
    flag_hidden = 0;

    printk("module was exposed");
} 

// misc devices description

ssize_t write_stub(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos) { return 0; }
ssize_t read_stub(struct file *filp, char __user *buff, size_t count, loff_t *f_pos) { return 0; }
int open_stub(struct inode *inode, struct file *file) { return 0; }
int release_stub(struct inode *inode, struct file *file) { return 0; }

ssize_t read_l2_data(struct file *filp, char __user *buff, size_t count, loff_t *f_pos) {
    if (count != sizeof(struct l2_data_slice)) {
        printk(MODULE_DMESG_PREFIX "[ERROR] invalid count");
        return 0;
    }
    
    if (second_layer_table_read_index >= second_layer_table_next_index) {
        second_layer_table_read_index = 0;
        return 0;
    }

    struct l2_data_slice l2_ds = {
        .from_time = second_layer_table[second_layer_table_read_index].from_time,
        .unique_saddr_count = second_layer_table[second_layer_table_read_index].unique_saddr_count,
        .catched_packets_count = second_layer_table[second_layer_table_read_index].catched_packets_count,
        .tot_len = second_layer_table[second_layer_table_read_index].tot_len,
        .avg_catched_packets = second_layer_table[second_layer_table_read_index].avg_catched_packets,
        .max_catched_packets = second_layer_table[second_layer_table_read_index].max_catched_packets,
        .avg_len = second_layer_table[second_layer_table_read_index].avg_len,
        .max_len = second_layer_table[second_layer_table_read_index].max_len,
        .crit_behaviour = second_layer_table[second_layer_table_read_index].crit_behaviour,
        .property = second_layer_table[second_layer_table_read_index].property,
        .type = second_layer_table[second_layer_table_read_index].type,
        .tot_tcp = second_layer_table[second_layer_table_read_index].tot_tcp,
        .tot_syn = second_layer_table[second_layer_table_read_index].tot_syn,
    };

    if (copy_to_user(buff, (char *) &l2_ds, sizeof(struct l2_data_slice)))
    {
        printk(MODULE_DMESG_PREFIX "[ERROR] copy_to_user error");
        return 0;
    }

    second_layer_table_read_index += 1;

    return sizeof(struct l2_data_slice);
}

ssize_t read_stats(struct file *filp, char __user *buff, size_t count, loff_t *f_pos) {
    if (count != sizeof(struct module_stats)) {
        printk(MODULE_DMESG_PREFIX "[ERROR] invalid count");
        return 0;
    }

    uint32_t history_length = SECOND_LAYER_TABLE_LENGTH;
    uint32_t current_history_length = second_layer_table_next_index;
    uint64_t current_periud_ns = 0;
    uint32_t crit_behaviour_count = 0;
    uint64_t first_crit_behaviour_ns = 0;
    uint64_t last_crit_behaviour_ns = 0;

    for (int i = 0; i < second_layer_table_next_index; i++) {
        if (second_layer_table[i].crit_behaviour > 0) {
            crit_behaviour_count += 1;
            if (first_crit_behaviour_ns == 0) {
                first_crit_behaviour_ns = second_layer_table[i].from_time;
            }
            last_crit_behaviour_ns = second_layer_table[i].from_time;
        }
    }

    uint64_t per_start = 0, per_finish = 0;
    if (second_layer_table_next_index > 0) {
        per_start = second_layer_table[0].from_time;
        per_finish = second_layer_table[second_layer_table_next_index - 1].from_time;
        // printk(MODULE_DMESG_PREFIX "%lld %lld", per_start, per_finish);
    }
    current_periud_ns = per_finish - per_start;

    struct module_stats stats = {
        .history_length = history_length,
        .current_history_length = current_history_length,
        .current_periud_ns = current_periud_ns,
        .crit_behaviour_count = crit_behaviour_count,
        .first_crit_behaviour_ns = first_crit_behaviour_ns,
        .last_crit_behaviour_ns = last_crit_behaviour_ns,
    };

    if (copy_to_user(buff, (char *) &stats, sizeof(struct module_stats)))
    {
        printk(MODULE_DMESG_PREFIX "[ERROR] copy_to_user error");
        return 0;
    }

    return sizeof(struct module_stats);
}

ssize_t write_command(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos) 
{
    struct command cmd;

    if (count != sizeof(struct command))
    {
        printk(MODULE_DMESG_PREFIX "[ERROR] incorrect command");
        return 0;
    }

    if (copy_from_user(&cmd, buff, sizeof(struct command)))
    {
        printk(MODULE_DMESG_PREFIX "[ERROR] copy_from_user error");
        return 0;
    }

    if (cmd.type == HIDE) {
        hide();
    }

    if (cmd.type == UNHIDE) {
        unhide();
    }

    return 0; 
}

static struct file_operations l2_data_fops = {
    .owner = THIS_MODULE,
    .read = read_l2_data,
    .write = write_stub,
    .open = open_stub,
    .release = release_stub,
};

static struct file_operations stats_fops = {
    .owner = THIS_MODULE,
    .read = read_stats,
    .write = write_stub,
    .open = open_stub,
    .release = release_stub,
};

static struct file_operations ctrl_fops = {
    .owner = THIS_MODULE,
    .read = read_stub,
    .write = write_command,
    .open = open_stub,
    .release = release_stub,
};

struct miscdevice dev_l2_data = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_L2_DATA_FNAME,
    .fops = &l2_data_fops,
    .mode = S_IRWXU | S_IWGRP | S_IWOTH | S_IROTH,
};

struct miscdevice dev_stats = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_STATS_FNAME,
    .fops = &stats_fops,
    .mode = S_IRWXU | S_IWGRP | S_IWOTH | S_IROTH,
};

struct miscdevice dev_ctrl = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_CTRL_FNAME,
    .fops = &ctrl_fops,
    .mode = S_IRWXU | S_IWGRP | S_IWOTH | S_IROTH,
};

static int __init my_module_init(void) {
    int rc = 0;

    nf_register_net_hook(&init_net, &module_hook_ops);

    rc = misc_register(&dev_l2_data);
    if (rc)
    {
        printk(MODULE_DMESG_PREFIX "[ERROR] registration was failed");
        return rc;
    }

    rc = misc_register(&dev_stats);
    if (rc)
    {
        printk(MODULE_DMESG_PREFIX "[ERROR] registration was failed");
        return rc;
    }

    rc = misc_register(&dev_ctrl);
    if (rc)
    {
        printk(MODULE_DMESG_PREFIX "[ERROR] registration was failed");
        return rc;
    }

    printk(MODULE_DMESG_PREFIX "module was loaded");

    return 0;
}

static void __exit my_module_exit(void) {

    nf_unregister_net_hook(&init_net, &module_hook_ops);

    misc_deregister(&dev_l2_data);
    misc_deregister(&dev_stats);
    misc_deregister(&dev_ctrl);

    printk(MODULE_DMESG_PREFIX "module was unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
