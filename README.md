# vk-force-priority
vk-force-priority allows you to control the global priority of any vulkan application.

## Building from Source

### Dependencies
Before building, you will need:
- GCC >= 9
- Vulkan Headers

### Building

**These instructions use `--prefix=/usr`, which is generally not recommened since the .so will be installed in directories that are meant for the package manager. The alternative is not setting the prefix, it will then be installed in `/usr/local`. But you need to make sure that `ld` finds the library since /usr/local is very likely not in the default path.** 

In general, prefer using distro provided packages.

```
git clone https://github.com/DadSchoorse/vk-force-priority.git
cd vk-force-priority
```

#### 64bit

```
meson --buildtype=release --prefix=/usr builddir
ninja -C builddir install
```
#### 32bit

Make sure that `PKG_CONFIG_PATH=/usr/lib32/pkgconfig` and `--libdir=lib32` are correct for your distro and change them if needed. On Debian based distros you need to replace `lib32` with `lib/i386-linux-gnu`, for example.
```
ASFLAGS=--32 CFLAGS=-m32 CXXFLAGS=-m32 PKG_CONFIG_PATH=/usr/lib32/pkgconfig meson --prefix=/usr --buildtype=release --libdir=lib32 -Dwith_json=false builddir.32
ninja -C builddir.32 install
```

## Usage

```ini
ENABLE_VKPRIORITY=1 VK_PRIORITY=medium yourcommand
```

Possible `VK_PRIORITY` values are `low`, `medium`, `high` or `realtime`. `high` and `realtime` might fail with `VK_ERROR_NOT_PERMITTED_EXT`.
