#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David");
MODULE_DESCRIPTION("Netfilter Kernel Module");

static unsigned int packet_hook(void *priv,
				struct sk_buff *skb,
				const struct nf_hook_state *state) {
	printk(KERN_INFO "Netfilter Module: Packet intercepted.\n");
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
