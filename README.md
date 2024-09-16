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
| R | Draw multiple spines in normal/reversed order. |
| T | Show/hide text. |
| Esc | Close the application. |
| Up | Move on to the next folder. |
| Down | Move on to the previous folder. |
| PageUp | Speed up the audio playback rate. |
| PageDown | Speed down the audio playback rate. |
| Home | Reset the audio playback rate.|  

- Some scene would be better rendered ignoring blend-mode specified by slots.

## ガチャ演出
To render gacha animation as intended, draw-order is important.  
To be precise, the animation of `*_m1.png` should be first drawn and that of `*_m0.png` be drawn on that.

<pre>
tf_1083001
├ tf_1083001_m0.atlas.txt
├ tf_1083001_m0.png
├ tf_1083001_m0.skel.txt
├ tf_1083001_m1.atlas.txt
├ tf_1083001_m1.png
└ tf_1083001_m1.skel.txt
</pre>

A feature to reverse draw-order (`R` key) is for this purpose.

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
