#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/if_ether.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David");
MODULE_DESCRIPTION("Netfilter Kernel Module for packet inspection");

static unsigned int packet_hook(void *priv,
				struct sk_buff *skb,
				const struct nf_hook_state *state) {
	struct iphdr *ip_header;

	if (skb->protocol == htons(ETH_P_IP)) {
		ip_header = ip_hdr(skb);

		if (ip_header) { 
			// Filter out 127.x.x.x  addresses
            		if ((ip_header->saddr & htonl(0xFF000000)) != htonl(0x7F000000))

			printk(KERN_INFO "Netfilter Module: Packet intercepted - Source: %pI4, Destination: %pI4\n", &ip_header->saddr, &ip_header->daddr);
		} else
			printk(KERN_INFO "Netfilter Module: Failed to get IP header\n");
	}

	return NF_ACCEPT;
}

static struct nf_hook_ops nfho = {
	.hook = packet_hook,
	.pf = PF_INET,
	.hooknum = NF_INET_PRE_ROUTING,
	.priority = NF_IP_PRI_FIRST,
};

static int __init netfilter_module_init(void) {
	nf_register_net_hook(&init_net, &nfho); // Register hook
	printk(KERN_INFO "Netfilter Module: Loaded and hook registered\n");
	return 0;
}

static void __exit netfilter_module_exit(void) {
	nf_unregister_net_hook(&init_net, &nfho); // Unregister hook
	printk(KERN_INFO "Netfilter Module: Unloaded and hook unregistered.\n");
}

module_init(netfilter_module_init);
module_exit(netfilter_module_exit);
