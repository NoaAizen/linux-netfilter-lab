#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/if_ether.h>
#include <linux/jhash.h>
#include <linux/slab.h>  // For kmalloc and kfree

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David and Noa");
MODULE_DESCRIPTION("Rate limiting and anomaly detection and prevention");

// Define rate limiting parameters
#define MAX_PACKETS_PER_IP 100  // Max number of packets allowed per IP in time window
#define TIME_WINDOW_SECS 10     // Time window in seconds

// Struct to hold info for each source IP
struct ip_entry {
    __be32 ip;                // Source IP address
    unsigned long timestamp;  // Timestamp of the first packet
    int packet_count;         // Count of packets from this IP
    struct hlist_node hnode;  // Linked list node
};

// Define a hash table to store IP entries
#define IP_HASH_SIZE 256
static struct hlist_head ip_hash_table[IP_HASH_SIZE];

// Hash function to map IP to a bucket in the hash table
static inline int ip_hash(__be32 ip) {
    return jhash(&ip, sizeof(ip), 0) % IP_HASH_SIZE;
}

// Function to find an IP entry in the hash table
static struct ip_entry *find_ip_entry(__be32 ip) {
    int bucket = ip_hash(ip);
    struct ip_entry *entry;

    hlist_for_each_entry(entry, &ip_hash_table[bucket], hnode) {
        if (entry->ip == ip)
            return entry;
    }
    return NULL;  // Return NULL if the IP entry is not found
}

// Function to add a new IP entry
static struct ip_entry *add_ip_entry(__be32 ip) {
    struct ip_entry *new_entry;
    int bucket = ip_hash(ip);

    new_entry = kmalloc(sizeof(*new_entry), GFP_ATOMIC);
    if (!new_entry)
        return NULL;

    new_entry->ip = ip;
    new_entry->timestamp = jiffies;
    new_entry->packet_count = 1;

    INIT_HLIST_NODE(&new_entry->hnode);
    hlist_add_head(&new_entry->hnode, &ip_hash_table[bucket]);

    return new_entry;
}

// Hook function to inspect packets
static unsigned int packet_inspection_hook(void *priv,
                                           struct sk_buff *skb,
                                           const struct nf_hook_state *state) {
    struct iphdr *iph;
    struct ip_entry *entry;
    unsigned long current_time;

    iph = ip_hdr(skb);
    if (!iph)
        return NF_ACCEPT;  // Not an IP packet, accept it

    // Get current time
    current_time = jiffies;

    // Check if the source IP is already in the hash table
    entry = find_ip_entry(iph->saddr);
    if (entry) {
        // Check if the time window has expired
        if (time_after(current_time, entry->timestamp + msecs_to_jiffies(TIME_WINDOW_SECS * 1000))) {
            // Reset the timestamp and packet count
            entry->timestamp = current_time;
            entry->packet_count = 1;
        } else {
            // Increment the packet count
            entry->packet_count++;

            // If the rate limit is exceeded, drop the packet
            if (entry->packet_count > MAX_PACKETS_PER_IP) {
                printk(KERN_INFO "Rate limiting: Dropping packet from IP %pI4\n", &iph->saddr);
                return NF_DROP;  // Drop the packet
            }
        }
    } else {
        // Add new IP entry
        entry = add_ip_entry(iph->saddr);
        if (!entry) {
            printk(KERN_WARNING "Rate limiting: Failed to allocate memory for IP entry\n");
            return NF_ACCEPT;  // If we fail to allocate memory, accept the packet
        }
    }

    return NF_ACCEPT;  // Accept the packet if it does not exceed the rate limit
}

// Define the nf_hook_ops structure
static struct nf_hook_ops nfho = {
    .hook = packet_inspection_hook,
    .pf = PF_INET,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST,
};

// Module initialization function
static int __init rate_limit_module_init(void) {
    int i;

    // Initialize the hash table
    for (i = 0; i < IP_HASH_SIZE; i++) {
        INIT_HLIST_HEAD(&ip_hash_table[i]);
    }

    // Register the Netfilter hook
    nf_register_net_hook(&init_net, &nfho);
    printk(KERN_INFO "Rate limiting module loaded.\n");

    return 0;
}

// Module exit function
static void __exit rate_limit_module_exit(void) {
    int i;
    struct hlist_node *tmp;
    struct ip_entry *entry;

    // Unregister the Netfilter hook
    nf_unregister_net_hook(&init_net, &nfho);

    // Clean up the hash table
    for (i = 0; i < IP_HASH_SIZE; i++) {
        hlist_for_each_entry_safe(entry, tmp, &ip_hash_table[i], hnode) {
            hlist_del(&entry->hnode);
            kfree(entry);
        }
    }

    printk(KERN_INFO "Rate limiting module unloaded.\n");
}

module_init(rate_limit_module_init);
module_exit(rate_limit_module_exit);
