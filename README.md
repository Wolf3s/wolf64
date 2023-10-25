# Wolfenstein 64

A port of Wolfenstein 3D and Spear of Destiny to the Nintendo 64 console,
adapted from [Wolf4SDL](https://github.com/11001011101001011/Wolf4SDL) and
using [libdragon](https://libdragon.dev/).

## Features

- Analog movement with the control stick
- One slot save/load using SRAM
- N64 Mouse support
- Cheat menu

## Download

ROMs built from the shareware versions of the game are included in the repository:

- [wolfdemo.z64](./wolfdemo64.z64?raw=1)
- [speardemo.z64](./speardemo64.z64?raw=1)

## Building

[Libdragon
Unstable](https://github.com/DragonMinded/libdragon/wiki/Unstable-branch) is
required. See the [Install
Instructions](https://github.com/DragonMinded/libdragon/wiki/Installing-libdragon)
for how to set it up. Ensure your `N64_INST` environment variable is set to the
libdragon install path.

To build the full version of Wolfenstein or Spear
([GOG](https://www.gog.com/en/game/wolfenstein_3d),
[Steam](https://store.steampowered.com/app/2270/Wolfenstein_3D/),
[Humble](https://www.humblebundle.com/store/wolfenstein-3d)), the data
files must be copied from the game's installation folder into the wolf64 `data`
folder. The filenames of each of them must be all lowercase.

| Game | Required Files |
|-:|-|
| Wolfenstein      | audiohed.wl6 audiot.wl6 gamemaps.wl6 maphead.wl6 vgadict.wl6 vgagraph.wl6 vgahead.wl6 vswap.wl6 |
| Spear of Destiny | audiohed.sod audiot.sod gamemaps.sod maphead.sod vgadict.sod vgagraph.sod vgahead.sod vswap.sod |

Data files from the shareware demo versions are provided already in the `data`
folder.

Then use one of these commands to build the appropriate ROM.

```sh
make -j GAME=wolf
make -j GAME=spear
make -j GAME=wolfdemo
make -j GAME=speardemo
```

## Controls

| Input | Command |
|-:|-|
| A | Strafe |
| B | Use |
| Z | Attack |
| R | Run |
| L | Map |
| Start | Menu |
| Control Stick | Move/Turn |
| C-Left/C-Right | Strafe |
| C-Up/C-Down | Switch Weapons |

## See Also

Other classic id software games with open source N64 ports:

- [Omnispeak64](https://github.com/Ryzee119/Omnispeak64)
- [64doom](https://github.com/jnmartin84/64doom)
