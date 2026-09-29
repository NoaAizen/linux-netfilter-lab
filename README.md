# Linux Netfilter Lab

Linux kernel networking exercises developed collaboratively by Noa Aizen and David Zaydenberg during the Excellenteam program.

The four exercises build on each other, starting with a minimal loadable kernel module and ending with a Netfilter hook that rate-limits IPv4 traffic per source address.

| Exercise | Topic | Module |
|---|---|---|
| [ex1](ex1/) | Basic Linux kernel module | `basic_module.ko` |
| [ex2](ex2/) | Netfilter `PRE_ROUTING` hook | `netfilter_module.ko` |
| [ex3](ex3/) | IPv4 packet inspection | `netfilter_module.ko` |
| [ex4](ex4/) | Per-source-IP rate limiting | `netfilter_module.ko` |

## Exercises

### ex1: Basic Linux kernel module

A minimal loadable kernel module showing the module lifecycle:

- `module_init` / `module_exit` entry points
- Kernel logging with `printk(KERN_INFO ...)`
- Module metadata (`MODULE_LICENSE`, `MODULE_AUTHOR`, `MODULE_DESCRIPTION`)

It logs `Hello, Check Point!` on load and `Goodbye, Check Point!` on unload.

### ex2: Netfilter PRE_ROUTING hook

Registers a hook with the Netfilter framework:

- `struct nf_hook_ops` registered with `nf_register_net_hook` on `init_net`
- IPv4 (`PF_INET`) at `NF_INET_PRE_ROUTING` with `NF_IP_PRI_FIRST` priority
- Logs every intercepted packet and returns `NF_ACCEPT`

The hook only observes traffic. Every packet is accepted.

### ex3: IPv4 packet inspection

Extends the ex2 hook to read the IPv4 header:

- Checks `skb->protocol` for `ETH_P_IP`
- Reads the header with `ip_hdr(skb)`
- Logs source and destination addresses using the `%pI4` format specifier
- Skips logging for loopback sources (`127.0.0.0/8`)

All packets are still accepted. The loopback check only affects logging.

### ex4: Per-source-IP rate limiting

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

`ex4/test_rate_limit.sh` loads the module, sends 200 pings to `127.0.0.1`, prints matching kernel log lines and unloads the module.

## Building and running

These are out-of-tree modules. You need a Linux system with headers for the running kernel (for example `linux-headers-$(uname -r)` on Debian/Ubuntu).

```bash
cd ex2            # or ex1, ex3, ex4
make              # builds the .ko against /lib/modules/$(uname -r)/build
sudo insmod netfilter_module.ko     # ex1: basic_module.ko
sudo dmesg | tail
sudo rmmod netfilter_module         # ex1: basic_module
make clean
```

To run the ex4 test script, build ex4 first, then run it from inside `ex4/`:

```bash
cd ex4
make
chmod +x test_rate_limit.sh
./test_rate_limit.sh
```

Loading kernel modules needs root and affects the whole host's networking. Use a disposable virtual machine.

## Repository layout

```
ex1/  basic_module.c, Makefile, Readme.md
ex2/  netfilter_module.c, Makefile, README.md
ex3/  netfilter_module.c, Makefile, README.md
ex4/  netfilter_module.c, Makefile, test_rate_limit.sh
```

## Limitations

These modules are coursework, not a finished product.

- **Educational prototype:** the code shows kernel and Netfilter concepts and is not hardened.
- **No synchronization around the shared hash table (ex4):** the hook can run on several CPUs at once, but lookups, inserts and counter updates are not protected by a lock or RCU.
- **Entries stay allocated until module unload (ex4):** nothing expires or evicts entries, so memory grows with the number of distinct source addresses seen.
- **Simplified logging and rate-limiting design:** ex4 uses a fixed-window counter keyed only by source address. Logging is plain per-packet `printk` with no rate limit, and return values such as `nf_register_net_hook`'s are not checked.
- **Not production firewall software:** don't use it to protect real systems. Use `nftables`/`iptables` or another maintained firewall instead.

## Attribution

Developed collaboratively by Noa Aizen and David Zaydenberg during the Excellenteam program. Both contributed to the implementation of the Linux kernel and networking exercises.

The `MODULE_AUTHOR` values in the source files are kept exactly as they were in the original coursework.
