Package: cryptopp:x64-linux@8.7.0

**Host Environment**

- Host: x64-linux
- Compiler: Clang 20.1.2
-    vcpkg-tool version: 2025-07-21-d4b65a2b83ae6c3526acd1c6f3b51aff2a884533
    vcpkg-scripts version: 120deac306 2025-08-27 (32 hours ago)

**To Reproduce**

`vcpkg install `

**Failure logs**

```
Downloading https://github.com/abdes/cryptopp-cmake/archive/51d00a28d761b7ae1585ee554a55fd8e6b943c9a.tar.gz -> abdes-cryptopp-cmake-51d00a28d761b7ae1585ee554a55fd8e6b943c9a.tar.gz
Successfully downloaded abdes-cryptopp-cmake-51d00a28d761b7ae1585ee554a55fd8e6b943c9a.tar.gz
-- Extracting source /tmp/vcpkg/downloads/abdes-cryptopp-cmake-51d00a28d761b7ae1585ee554a55fd8e6b943c9a.tar.gz
-- Using source at /tmp/vcpkg/buildtrees/cryptopp/src/8e6b943c9a-b1154bba98.clean
Downloading https://github.com/weidai11/cryptopp/archive/511806c0eba8ba5b5cedd4b4a814e96df92864a6.tar.gz -> weidai11-cryptopp-511806c0eba8ba5b5cedd4b4a814e96df92864a6.tar.gz
Successfully downloaded weidai11-cryptopp-511806c0eba8ba5b5cedd4b4a814e96df92864a6.tar.gz
-- Extracting source /tmp/vcpkg/downloads/weidai11-cryptopp-511806c0eba8ba5b5cedd4b4a814e96df92864a6.tar.gz
-- Applying patch patch.patch
-- Using source at /tmp/vcpkg/buildtrees/cryptopp/src/6df92864a6-b4419a9589.clean
-- Configuring x64-linux-dbg
-- Configuring x64-linux-rel
-- Building x64-linux-dbg
-- Building x64-linux-rel
CMake Error at buildtrees/versioning_/versions/cryptopp/7e3fcbde366bc4af8cc2082053b671352963410d/portfile.cmake:75 (file):
  file RENAME failed to rename

    /tmp/vcpkg/packages/cryptopp_x64-linux/debug/share/pkgconfig/cryptopp.pc

  to

    /tmp/vcpkg/packages/cryptopp_x64-linux/lib/pkgconfig/cryptopp.pc

  because: No such file or directory

Call Stack (most recent call first):
  scripts/ports.cmake:206 (include)



```

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "name": "aether",
  "version": "1.0.0",
  "description": "Advanced Luau Scripting Environment",
  "homepage": "https://github.com/aether-project/aether",
  "license": "MIT",
  "dependencies": [
    {
      "name": "nlohmann-json",
      "version>=": "3.9.1"
    },
    {
      "name": "cryptopp",
      "version>=": "8.7.0"
    },
    {
      "name": "boost-system",
      "version>=": "1.75.0"
    },
    {
      "name": "boost-filesystem",
      "version>=": "1.75.0"
    },
    {
      "name": "boost-thread",
      "version>=": "1.75.0"
    },
    {
      "name": "spdlog",
      "version>=": "1.8.0"
    },
    {
      "name": "catch2",
      "version>=": "2.13.1"
    }
  ],
  "builtin-baseline": "3426db05b996481ca31e95fff3734cf23e0f51bc",
  "overrides": [
    {
      "name": "cryptopp",
      "version": "8.7.0"
    },
    {
      "name": "boost-filesystem",
      "version": "1.75.0"
    },
    {
      "name": "boost-system",
      "version": "1.75.0"
    },
    {
      "name": "boost-thread",
      "version": "1.75.0"
    },
    {
      "name": "catch2",
      "version": "2.13.1"
    },
    {
      "name": "nlohmann-json",
      "version": "3.9.1"
    },
    {
      "name": "spdlog",
      "version": "1.8.0"
    }
  ]
}

```
</details>
