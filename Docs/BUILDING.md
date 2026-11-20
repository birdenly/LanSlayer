# Building

### DepotDownloader CI Setup
For DepotDownloader CI authentication to work, you must first log in locally and download something using DepotDownloader with the `-remember-password` flag. This generates the `account.config` file containing your saved authentication data.

Configure the following GitHub repository secrets:
- `STEAM_USERNAME` — your Steam username.
- `STEAM_CONFIG` — your Base64-encoded `account.config` contents.

Generate the Base64 value with:
```sh
base64 -w 0 ~/.local/share/IsolatedStorage/*/*/Url.*/AssemFiles/account.config
```

Copy the command's output and paste it into the `STEAM_CONFIG` secret.

# Applying the .r2 patches:
Install [radare2](https://github.com/radareorg/radare2)
```sh
r2 -q -w -nn -i Launcher/patches/DOOM/r2/DOOMx64.exe.r2 -c 'wci' DOOMx64.exe
r2 -q -w -nn -i Launcher/patches/DOOM/r2/DOOMx64vk.exe.r2 -c 'wci' DOOMx64vk.exe
```

## Windows
### Launcher
```sh
cd Launcher
cmake . -B build -G "Visual Studio 18 2026"
cmake --build build --config Release --parallel %NUMBER_OF_PROCESSORS%
```

## Linux
### Launcher
```sh
cd Launcher
cmake . -B build -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw64.cmake
cmake --build build --config Release --parallel "$(nproc)"
```

See [the workflows](../.github/workflows/)
