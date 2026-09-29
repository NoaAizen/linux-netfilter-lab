# README

## Netfilter Kernel Module for Packet Inspection

This is a simple Linux kernel module that uses the Netfilter framework to intercept IPv4 packets and log their source and destination IP addresses to the kernel log. The module filters out packets originating from loopback addresses (`127.x.x.x`). It logs a message each time a qualifying packet is intercepted, as well as when the module is loaded or unloaded.

### Input

- **No direct input**: The module operates by intercepting network packets automatically.

### Output

- **When a qualifying packet is intercepted**:

  ```
  Netfilter Module: Packet intercepted - Source: <Source IP>, Destination: <Destination IP>
  ```

- **When the module is loaded**:

  ```
  Netfilter Module: Loaded and hook registered
  ```

- **When the module is unloaded**:

  ```
  Netfilter Module: Unloaded and hook unregistered.
  ```

## How to Use

### 1. Create a Makefile

Create a file named `Makefile` in the same directory as your module source code (`netfilter_module.c`) with the following content:

```makefile
obj-m += netfilter_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

### 2. Compilation

Compile the kernel module by running:

```bash
make
```

This will generate a `netfilter_module.ko` file, which is the compiled kernel module.

### 3. Load and Unload the Module

- **To load the module into the kernel**:

  ```bash
  sudo insmod netfilter_module.ko
  ```

- **To unload the module from the kernel**:

  ```bash
  sudo rmmod netfilter_module
  ```

### 4. Generate Network Traffic

To test the module, generate some network traffic. For example:

- **Ping an external host**:

  ```bash
  ping -c 5 google.com
  ```

- **Access a website**:

  ```bash
  curl http://example.com
  ```

### 5. Check Kernel Log Messages

To view the messages logged by the module, check the kernel log:

```bash
dmesg | tail -n 50
```

You should see messages similar to:

- **After loading the module**:

  ```
  Netfilter Module: Loaded and hook registered
  ```

- **When a packet is intercepted** (excluding packets from `127.x.x.x` addresses):

  ```
  Netfilter Module: Packet intercepted - Source: 192.168.1.100, Destination: 93.184.216.34
  ```

- **After unloading the module**:

  ```
  Netfilter Module: Unloaded and hook unregistered.
  ```

## Important Notes

- **Filtering Logic**: The module currently filters out packets originating from loopback addresses (`127.x.x.x`).
