# Linux Netfilter Lab

Linux kernel networking exercises developed collaboratively by Noa Aizen and David Zaydenberg during the Excellenteam program.

The four exercises build on each other, starting with a minimal loadable kernel module and ending with a Netfilter hook that rate-limits IPv4 traffic per source address.

| Exercise | Topic | Module |
|---|---|---|
| [basic-kernel-module](basic-kernel-module/) | Basic Linux kernel module | `basic_module.ko` |
| [netfilter-hook](netfilter-hook/) | Netfilter `PRE_ROUTING` hook | `netfilter_module.ko` |
| [ipv4-packet-inspection](ipv4-packet-inspection/) | IPv4 packet inspection | `netfilter_module.ko` |
| [ip-rate-limiter](ip-rate-limiter/) | Per-source-IP rate limiting | `netfilter_module.ko` |

## Exercises

### basic-kernel-module: Basic Linux kernel module

A minimal loadable kernel module showing the module lifecycle:

- `module_init` / `module_exit` entry points
- Kernel logging with `printk(KERN_INFO ...)`
- Module metadata (`MODULE_LICENSE`, `MODULE_AUTHOR`, `MODULE_DESCRIPTION`)

It logs `Hello, Check Point!` on load and `Goodbye, Check Point!` on unload.

### netfilter-hook: Netfilter PRE_ROUTING hook

Registers a hook with the Netfilter framework:

- `struct nf_hook_ops` registered with `nf_register_net_hook` on `init_net`
- IPv4 (`PF_INET`) at `NF_INET_PRE_ROUTING` with `NF_IP_PRI_FIRST` priority
- Logs every intercepted packet and returns `NF_ACCEPT`

The hook only observes traffic. Every packet is accepted.

### ipv4-packet-inspection: IPv4 packet inspection

Extends the netfilter-hook module to read the IPv4 header:

- Checks `skb->protocol` for `ETH_P_IP`
- Reads the header with `ip_hdr(skb)`
- Logs source and destination addresses using the `%pI4` format specifier
- Skips logging for loopback sources (`127.0.0.0/8`)

All packets are still accepted. The loopback check only affects logging.

### ip-rate-limiter: Per-source-IP rate limiting

A Netfilter `PRE_ROUTING` hook that tracks IPv4 source addresses and drops traffic from sources that exceed a packet budget.

- **Netfilter:** the hook runs at `NF_INET_PRE_ROUTING` with `NF_IP_PRI_FIRST` priority.
- **IPv4 source tracking:** each source address (`iph->saddr`) has its own entry holding the address, the window start time and a packet count.
- **256-bucket hash table:** entries live in a static array of 256 `struct hlist_head` buckets.
- **jhash:** the bucket index is `jhash(&ip, sizeof(ip), 0) % 256`.
- **Kernel hlist:** entries are chained with `struct hlist_node` and handled with `hlist_add_head`, `hlist_for_each_entry` and `hlist_for_each_entry_safe`.
- **jiffies:** the window start is stored in `jiffies` and expiry is checked with `time_after`.
- **kmalloc/kfree:** new entries are allocated with `kmalloc(..., GFP_ATOMIC)` because the hook runs in atomic context. All entries are freed with `kfree` when the module unloads.
- **NF_ACCEPT/NF_DROP:** packets within the budget return `NF_ACCEPT`. Packets beyond it return `NF_DROP` and are logged. If an entry can't be allocated, the packet is accepted.
- **Limit:** 100 packets per source IP per 10-second fixed window (`MAX_PACKETS_PER_IP`, `TIME_WINDOW_SECS`). When the window expires, the count resets.

`ip-rate-limiter/test_rate_limit.sh` loads the module, sends 200 pings to `127.0.0.1`, prints matching kernel log lines and unloads the module.

## Building and running

These are out-of-tree modules. You need a Linux system with headers for the running kernel (for example `linux-headers-$(uname -r)` on Debian/Ubuntu).

```bash
cd netfilter-hook   # or basic-kernel-module, ipv4-packet-inspection, ip-rate-limiter
make                # builds the .ko against /lib/modules/$(uname -r)/build
sudo insmod netfilter_module.ko     # basic-kernel-module: basic_module.ko
sudo dmesg | tail
sudo rmmod netfilter_module         # basic-kernel-module: basic_module
make clean
```

To run the rate limiter test script, build the module first, then run it from inside `ip-rate-limiter/`:

```bash
cd ip-rate-limiter
make
chmod +x test_rate_limit.sh
./test_rate_limit.sh
```

Loading kernel modules needs root and affects the whole host's networking. Use a disposable virtual machine.

## Repository layout

```
basic-kernel-module/     basic_module.c, Makefile, Readme.md
netfilter-hook/          netfilter_module.c, Makefile, README.md
ipv4-packet-inspection/  netfilter_module.c, Makefile, README.md
ip-rate-limiter/         netfilter_module.c, Makefile, test_rate_limit.sh
```

## Limitations

These modules are coursework, not a finished product.

- **Educational prototype:** the code shows kernel and Netfilter concepts and is not hardened.
- **No synchronization around the shared hash table (ip-rate-limiter):** the hook can run on several CPUs at once, but lookups, inserts and counter updates are not protected by a lock or RCU.
- **Entries stay allocated until module unload (ip-rate-limiter):** nothing expires or evicts entries, so memory grows with the number of distinct source addresses seen.
- **Simplified logging and rate-limiting design:** the rate limiter uses a fixed-window counter keyed only by source address. Logging is plain per-packet `printk` with no rate limit, and return values such as `nf_register_net_hook`'s are not checked.
- **Not production firewall software:** don't use it to protect real systems. Use `nftables`/`iptables` or another maintained firewall instead.

## Attribution

Developed collaboratively by Noa Aizen and David Zaydenberg during the Excellenteam program. Both contributed to the implementation of the Linux kernel and networking exercises.

The `MODULE_AUTHOR` values in the source files are kept exactly as they were in the original coursework.
