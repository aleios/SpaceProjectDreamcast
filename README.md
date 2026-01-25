# SPJ - Space Project (yeah...)

Small demo space SHMUP for the Dreamcast. Uses [KallistiOS](https://github.com/KallistiOS) and [SH4ZAM](https://github.com/gyrovorbis/sh4zam).

The code here should not be considered as a reference for anything good. It's god awful and is the product of hacking around to get used to KOS, SH4ZAM and the PVR.

## Build Instructions

> git submodule update --init --recursive

For emulator:
> make emu

For dc-load:
> make run

For disc:
> make cdi

### Current hardcoded asset paths

Will be replaced with configurable values eventually but required for now.

Without these the game will abort.

- animations/player
- animations/ui
- sprites/ui
- sprites/ships
- sprites/font_16x16
- fonts/main_font

## Asset Attribution

### Music

Author: David KBD

URL: https://www.davidkbd.com/

Asset: Interstellar EDM Metal Music Pack

Asset URL: https://davidkbd.itch.io/interstellar-edm-metal-music-pack

License: CC By 4.0 (https://creativecommons.org/licenses/by/4.0/)

Modifications:
- Converted to ADX format

### Sprites

Author: Dylestorm

URL: www.livingtheindie.com

Asset: Pixel SHMUP Free 1.3

Asset URL: https://livingtheindie.itch.io/pixel-shmup-ships-free

License: https://livingtheindie.com/asset-license/

Uses:
- Player
- Enemy
- UI

Modifications:
- Packed into texture atlas 

### Fonts

Author: CodeMan38 (cody@zone38.net)

URL: https://fonts.google.com/specimen/Press+Start+2P

Asset: Press Start 2P

License: OFL 1.1 (OFL-PressStart2P.txt)

--

Author: Adobe

URL: https://fonts.google.com/noto/specimen/Noto+Sans+JP

Asset: Noto Sans JP

License: OFL 1.1 (OFL-NotoSansJP.txt)

## Special Thanks

- Falco Girgis ([Gyrovorbis](https://github.com/gyrovorbis)) for [sh4zam](https://github.com/gyrovorbis/sh4zam) and the 1000 other examples of using the pvr
- sizious and nextgeniuspro for libadx(+modifications)
- [KallistiOS](https://github.com/KallistiOS) contributors