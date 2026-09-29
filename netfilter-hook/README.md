# README

## Netfilter Kernel Module

This is a simple Linux kernel module that demonstrates the use of the Netfilter framework to intercept and log incoming packets. The module logs a message each time a packet is intercepted, and when the module is loaded or unloaded.

### Input

- **No direct input**: This module works by intercepting network packets.

### Output

- **When a packet is intercepted**: `"Netfilter Module: Packet intercepted."`
- **When the module is loaded**: `"Netfilter Module: Loaded and hook registered"`
- **When the module is unloaded**: `"Netfilter Module: Unloaded and hook unregistered."`

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

Run the following command to compile the module:

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

### 4. Check Kernel Log Messages

To view the messages logged by the module, check the kernel log:

```bash
dmesg | tail
```

You should see the following messages:

- **After loading the module**:

  ```
  Netfilter Module: Loaded and hook registered
  ```

- **When a packet is intercepted** (this will appear multiple times if packets are intercepted):

  ```
  Netfilter Module: Packet intercepted.
  ```

- **After unloading the module**:

  ```
  Netfilter Module: Unloaded and hook unregistered.
  ```
