## Simple Linux Kernel Module

This is a simple Linux kernel module that demonstrates basic module initialization and cleanup. When loaded, it logs a message to the kernel log, and when unloaded, it logs another message.

### Input

- **No input**

### Output

- **When the module is loaded**: `"Hello, Check Point!"`
- **When the module is unloaded**: `"Goodbye, Check Point!"`

## How to Use

### 1. Create a Makefile

Create a file named `Makefile` in the same directory as your module source code (`basic_module.c`) with the following content:

```makefile
obj-m += basic_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

**Note**: Replace `basic_module.o` with the object file name matching your source file if it's different.

### 2. Compilation

Run the following command to compile the module:

```bash
make
```

This will generate a `basic_module.ko` file, which is the compiled kernel module.

### 3. Load and Unload the Module

- **To load the module into the kernel**:

  ```bash
  sudo insmod basic_module.ko
  ```

- **To unload the module from the kernel**:

  ```bash
  sudo rmmod basic_module
  ```

### 4. Check Kernel Log Messages

To view the messages logged by the module, check the kernel log:

```bash
dmesg | tail
```

You should see the following messages:

- **After loading the module**:

  ```
  Hello, Check Point!
  ```

- **After unloading the module**:

  ```
  Goodbye, Check Point!
  ```
