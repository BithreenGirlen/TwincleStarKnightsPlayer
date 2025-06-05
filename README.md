# TwincleStarKnightsPlayer
某寝室再生機

## Runtime requirement
- Windows OS later than Windows 10
- MSVC 2015-2022 (x64)

## How to play
First, prepare the files, commented below, with proper directory.

<pre>
...
├ Adventure
│  ├ ...
│  ├ ImportChara // scenario folder
│  │  ├ ...
│  │  ├ CharaScenario1083001.book.json
│  │  └ ...
│  └ ...
├ AssetBundles
│  ├ ...
│  ├ Sound
│  │  ├ ...
│  │  └ Voice
│  │    ├ ...
│  │    ├ ImportChara
│  │    │  ├ ...
│  │    │  ├ 1083001 // voice folder
│  │    │  │  ├ ...
│  │    │  │  ├ b083_009_000001.m4a
│  │    │  │  └ ...
│  │    │  └ ...
│  │    └ ...
│  ├ ...
│  └ Stills
│    ├ ...
│    ├ st_1083001 // spine folder
│    │  ├ st_1083001.atlas
│    │  ├ st_1083001.png
│    │  └ st_1083001.skel
│    └ ...
└ ...
</pre>

Then, select any of spine folder in `AssetBundles/Stills/st_XXXXXXX` from the application.  
The scene will be set up based on `CharaScenarioXXXXXXX.book.json` if exists.

## Mouse functions
| Input | Action |
| --- | --- |
| Mouse wheel | Scale up/down. Combinating with <kbd>Ctrl</kbd> retains window size. |
| Left button + mouse wheel | Speed up/down the animation. |
| Left button drag | Move view-point |
| Middle button | Reset scaling, animation speed, and view-point to default. |
| Right button + mouse wheel | Fast-forward/rewind the text. |
| Right button + left button | Start moving window. Left-click to end moving. |

## Keyboard functions
| Input | Action |
| --- | --- |
| <kbd>B</kbd> | Prefer/ignore blend-mode specified by slots. |
| <kbd>C</kbd> | Toggle text colour between black and white. |
| <kbd>T</kbd> | Show/hide text. |
| <kbd>Esc</kbd> | Close the application. |
| <kbd>Up</kbd> | Open the previous folder. |
| <kbd>Down</kbd> | Open the next folder. |
| <kbd>Right</kbd> | Fast-forward the text. |
| <kbd>Left</kbd> | Rewind the text. |

- Some scenes require entering `B` key to force blend-mode `Normal`instead of `Multiply`.

## Preferences
The following preferences can be configured through `setting.txt` in the same directory of the executable file.
- Font file with which the text will be drawn.
- File extension of scene resources.

## External libraries
- [JSON for Modern C++ v3.11.3](https://github.com/nlohmann/json/releases/tag/v3.11.3)
- [SFML-2.6.1](https://www.sfml-dev.org/download/sfml/2.6.1/)
- [spine-cpp-4.0](https://github.com/EsotericSoftware/spine-runtimes/tree/4.0)

## Build
1. Run `deps/CMakeLists.txt` to obtain external libraries.
2. Open `TwincleStarKnightsPlayer.sln` with Visual Studio.
3. Select `Build Solution` on menu item.

<details><summary>deps directory will be as follows</summary>
  
<pre>
TwincleStarKnightsPlayer
  ├ deps
  │  ├ nlohmann // JSON for Modern C++
  │  │   └ json.hpp
  │  ├ SFML-2.6.1 // static libraries and headers of SFML for VC17
  │  │   ├ include
  │  │   │   └ ...
  │  │   └ lib
  │  │       └ ...
  │  ├ spine-cpp // sources and headers of spine-cpp 4.0
  │  │ ├ include
  │  │ │ └ ...
  │  │ └ src
  │  │   └ ...
  │  ├ CMakeLists.txt
  │  └ setting.txt
  └ ...
</pre>
  
</details>

