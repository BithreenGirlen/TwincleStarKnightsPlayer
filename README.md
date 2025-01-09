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
│    │  ├ st_1083001.atlas.txt
│    │  ├ st_1083001.png
│    │  └ st_1083001.skel.txt
│    └ ...
└ ...
</pre>

Then, select any of spine folder in `AssetBundles/Stills/st_XXXXXXX` from the application.  
The scene will be set up based on `CharaScenarioXXXXXXX.book.json` if exists.

## Mouse functions
| Input  | Action  |
| --- | --- |
| Mouse wheel | Scale up/down |
| Left button + mouse wheel | Speed up/down the animation. |
| Left button click | Switch to the next animation. |
| Left button drag | Move view-point |
| Middle button | Reset scaling, animation speed, and view-point. |
| Right button + mouse wheel | Show the next/previous text. |
| Right button + left button | Move window |

## Keyboard functions
| Input  | Action  |
| --- | --- |
| A | Enable/disable premultiplied alpha. |
| B | Prefer/ignore blend-mode specified by slots. |
| C | Switch text colour between black and white. |
| R | Switch draw-order between filename asc/descending order. |
| T | Show/hide text. |
| Esc | Close the application. |
| Up | Move on to the next folder. |
| Down | Move on to the previous folder. |
| PageUp | Speed up the audio playback rate. |
| PageDown | Speed down the audio playback rate. |
| Home | Reset the audio playback rate.|  

- Some scene requires ignoring blend-mode specified by slots.

## Preferences
The following preferences can be configured through `setting.txt` in the same directory of the executable file.
- Font file with which the scene text will be drawn.
- File extension of scene resources.

## External libraries
- [JSON for Modern C++ v3.11.3](https://github.com/nlohmann/json/releases/tag/v3.11.3)
- [SFML-2.6.1](https://www.sfml-dev.org/download/sfml/2.6.1/)
- [spine-cpp-4.0](https://github.com/EsotericSoftware/spine-runtimes/tree/4.0)

## Build
1. Run `deps/CMakeLists.txt` to obtain external libraries.
2. Open `TwincleStarKnightsPlayer.sln` with Visual Studio.
3. Select `Build Solution` on menu item.

The `deps` folder will be as follows:
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
