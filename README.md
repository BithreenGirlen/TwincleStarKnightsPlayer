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
│  ├ ImportChara // Scenario folder
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
│  │    │  ├ 1083001 // Voice folder
│  │    │  │  ├ ...
│  │    │  │  ├ b083_009_000001.m4a
│  │    │  │  └ ...
│  │    │  └ ...
│  │    └ ...
│  ├ ...
│  └ Stills
│    ├ ...
│    ├ st_1083001 // Spine folder; a folder to be selected
│    │  ├ st_1083001.atlas
│    │  ├ st_1083001.png
│    │  └ st_1083001.skel
│    └ ...
└ ...
</pre>

Then, select `AssetBundles/Stills/st_XXXXXXX` folder from the application.  
The scene will be set up based on `CharaScenarioXXXXXXX.book.json` if exists.

## Mouse functions

| Input | Action |
| --- | --- |
| Wheel scroll | Scale up/down. Combining <kbd>Ctrl</kbd> retains window size. |
| Left-pressed + wheel scroll | Speed up/down the animation. |
| Left-drag | Move view-point. |
| Middle-click | Reset scaling, animation speed, and view-point to default. |
| Right-pressed + wheel scroll | Fast-forward/rewind the text. |
| Right-pressed + left-click | Start moving borderless window. Left-click to end moving. |

## Keyboard functions
| Input | Action |
| --- | --- |
| <kbd>B</kbd> | Turn on/off forcing blend-mode-normal. |
| <kbd>C</kbd> | Toggle text colour between black and white. |
| <kbd>H</kbd> | Hide/show help. |
| <kbd>S</kbd> | Save the current frame as image. |
| <kbd>T</kbd> | Hide/show the text. |
| <kbd>Q</kbd> | Turn off/on the synchronisation of animation with text. |
| <kbd>Esc</kbd> | Close the application. |
| <kbd>Up</kbd> | Open the previous folder. |
| <kbd>Down</kbd> | Open the next folder. |
| <kbd>Right</kbd> | Fast-forward the text. |
| <kbd>Left</kbd> | Rewind the text. |

- Some scenes require forcing blend-mode `Normal` instead of `Multiply`.
- When synchronisation is turned off, left click shifts the animation.

## Preferences

The following preferences can be configured through `setting.txt` in the same directory of the executable file.
- Font file with which the text will be drawn.
- File extension of scene resources.

## External libraries

- [JSON for Modern C++ v3.12.0](https://github.com/nlohmann/json/releases/tag/v3.12.0)
- [SFML-3.0.2](https://www.sfml-dev.org/download/sfml/3.0.2/)
- [spine-cpp-4.0](https://github.com/EsotericSoftware/spine-runtimes/tree/4.0)

## Build

1. Configure `deps/CMakeLists.txt` to obtain external libraries.
2. Open `TwincleStarKnightsPlayer.sln` with Visual Studio.
3. Select `Build Solution` on menu item.

<details><summary>deps directory will be as follows</summary>
  
<pre>
src
├ deps
│  ├ nlohmann // JSON for Modern C++
│  │  └ json.hpp
│  ├ SFML-3.0.2 // static libraries and headers of SFML for VC17
│  │  ├ include
│  │  │  └ ...
│  │  └ lib
│  │     └ ...
│  ├ spine-cpp-4.0 // sources and headers of spine-cpp 4.0
│  │ ├ include
│  │ │  └ ...
│  │ └ src
│  │    └ ...
│  ├ CMakeLists.txt
│  └ setting.txt
└ ...
</pre>
  
</details>
