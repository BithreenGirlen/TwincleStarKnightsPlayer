# TwincleStarKnightsPlayer
某寝室再生機

## Runtime requirement
- Windows OS later than Windows 10
- MSVC 2015-2022 (x64)

## How to play
Select a folder containing spine resources through folder-select-dialogue.
<pre>
st_1083001 
  ├ st_1083001.atlas.txt
  ├ st_1083001.png
  └ st_1083001.skel.txt
</pre>

Then, scenario file would be searched assuming the following paths.
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
│  │    │  └ ...
│  │    └ ...
│  ├ ...
│  └ Stills
│    ├ ...
│    ├ st_1083001 // spine folder
│    └ ...
└ ...
</pre>
If the scenario file found, voice files and texts for the scene will be set up. But in order to show text, it is necessary to install `游明朝` in the machine.

## Mouse functions
| Input  | Action  |
| --- | --- |
| Mouse wheel | Scale up/down |
| Left button + mouse wheel | Speed up/down the animation. |
| Left button click | Switch to the next animation. |
| Left button drag | Move view point |
| Middle button | Reset scaling, animation speed, and view point. |
| Right button + mouse wheel | Play the next/previous audio file. |
| Right button + left button | Move Window |

## Keyboard functions
| Input  | Action  |
| --- | --- |
| A | Enable/disable premultiplied alpha. |
| B | Prefer/ignore blend-mode specified by the slot. |
| C | Switch text color between black and white. |
| T | Show/hide text. |
| Esc | Close the application. |
| Up | Move on to the next folder. |
| Down | Move on to the previous folder. |
| PageUp | Speed up the audio playback rate. |
| PageDown | Speed down the audio playback rate. |
| Home | Reset the audio playback rate.|  

- Some scene would be better rendered ignoring blend-mode specified by the slot.

## ガチャ演出
To render gacha animation, it is necessary to configure z-buffer (or depth-buffer).  
To be precise, arranging `-m0.png` at the front, and `-m1.png` at the back will work.

<pre>
tf_1083001
├ tf_1083001_m0.atlas.txt
├ tf_1083001_m0.png
├ tf_1083001_m0.skel.txt
├ tf_1083001_m1.atlas.txt
├ tf_1083001_m1.png
└ tf_1083001_m1.skel.txt
</pre>

Unfortunately, SFML does not support z-buffering, so I utilised other game engine for this purpose.  
[DxlibSpineTest](https://github.com/BithreenGirlen/DxlibSpineTest) with z-buffer enabled would play the gacha animation.

## Build dependency
- [JSON for Modern C++ v3.11.3](https://github.com/nlohmann/json/releases/tag/v3.11.3)
- [SFML-2.6.1](https://www.sfml-dev.org/download/sfml/2.6.1/)
- [spine-cpp-4.0](https://github.com/EsotericSoftware/spine-runtimes/tree/4.0)

When building, supply the above libraries under the project directory. 
<pre>
TwincleStarKnightsPlayer
  ├ deps
  │  ├ nlohmann // JSON for Modern C++
  │  │   └ json.hpp
  │  ├ SFML-2.6.1 // static libraries and headers of SFML
  │  │   ├ include
  │  │   │   └ ...
  │  │   └ lib
  │  │       └ ...
  │  └ spine-cpp // sources and headers of spine-cpp 4.0
  │    ├ include
  │    │ └ ...
  │    └ src
  │      └ ...
  └ ...
</pre>
