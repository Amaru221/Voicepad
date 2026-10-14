<div align="center">

<img src="Assets/header.svg" alt="VoicePad — anime girl with headphones (orange and neon-green theme)" width="100%">

[Español](README.md) &nbsp;·&nbsp; **English**

</div>

# VoicePad — Complete user guide

VoicePad is a **voice changer and soundboard** for Windows (similar to
Voicemod), with a **retro-futuristic console** look in amber. It takes your
microphone, transforms your voice, and "delivers" it to a **virtual
microphone** that Discord, games, OBS, etc. use as if it were your real
microphone.

This guide is meant to be understood by someone who has **never** used anything
like this. You don't need to know how to program to use it.

> Just want to use it? Skip to section **3. Your first time, step by step**.
> If you need to build it yourself, go to section **12**.

---

## Table of contents

1. What it is and what it's for
2. Before you start (install VB-Audio Cable)
3. Your first time, step by step
4. The screen: every panel explained
5. RESET and SAVE buttons in each section
6. Profiles: save and load your configurations
7. Color themes (SKIN)
8. Resizing the panels
9. Keyboard shortcuts
10. Where your settings are stored
11. FAQ and troubleshooting
12. Install and build (for developers)

---

## 1. What it is and what it's for

Picture a "socket" between your microphone and the other applications:

```
  Your microphone  ──►  VOICEPAD (changes your voice)  ──►  CABLE Input
                                                               │
  Discord / game / OBS  ◄── select  ──  CABLE Output  ◄────────┘
```

- If you choose **NORMAL**, your voice passes through unchanged (like a direct mic).
- If you choose **ROBOT**, **DEMON**, **CHIPMUNK**, etc., your voice comes out transformed.
- The **soundboard** plays sounds (laser, drum…) that the other person
  hears **inside** the same voice.

None of this is heard on your speakers (to avoid feedback): use **headphones**.

---

## 2. Before you start (install VB-Audio Cable)

VoicePad needs a free "virtual audio cable" called **VB-Audio Virtual Cable**.
It's what lets your processed voice reach Discord.

1. Go to <https://vb-audio.com/Cable/>
2. Download **VBCABLE_Setup_x64.exe**.
3. Run it **as administrator** (right-click → *Run as administrator*).
4. **Restart your computer.**

After restarting you should see two new devices in Windows:

- **`CABLE Input (VB-Audio Virtual Cable)`**
- **`CABLE Output (VB-Audio Virtual Cable)`**

> `CABLE Input` is *where VoicePad writes*. `CABLE Output` is *where the other
> apps read from*. Don't mix them up.

---

## 3. Your first time, step by step

1. Run **VoicePad.exe**.
2. In the **DEVICE ROUTING** panel (top left):
   - Click **AUTO-ROUTE**. This automatically picks your microphone as the
     *INPUT* and `CABLE Input` as the *OUTPUT*.
   - Click **ENGAGE**. You'll see the panel say **STREAM ACTIVE**.
3. Under **VOICE PRESETS**, click **ROBOT** (or any other). Notice how the
   **EFFECT MATRIX** faders move on their own: that's how each preset looks.
4. Open **Discord** (or your game):
   - Settings → **Voice & Video** → **Input device (microphone)** →
     choose **`CABLE Output (VB-Audio Virtual Cable)`**.
5. Talk. The other person should hear your voice with the effect.

**To test without disturbing anyone:** open Windows Sound Recorder or Audacity,
select `CABLE Output` as the input, record your voice, and listen to it.

> If you want no surprises, start with the **NORMAL** preset: it should sound
> like your normal voice.

---

## 4. The screen: every panel explained

The window is split into **panels** (a mosaic). Each panel has a **header**
with its name and, in some cases, small buttons.

### Top bar (at the very top)

- **VOICEPAD // VOX-CONTROL CONSOLE**: the name.
- **Status lamps**:
  - `ENGINE`: green = audio engine active.
  - `VOICE`: lights up when it detects your voice.
  - `LINK`: lights up when audio is going out.
- **EPOCH** / **LOCAL**: time counters.
- **SKIN**: the **color theme** selector (see section 7).
- **PROCESS: ON / BYPASS**: turns all processing on or off. In `BYPASS` your
  voice passes through as-is.

### DEVICE ROUTING (top left)

- **INPUT // MIC SOURCE**: which microphone is yours.
- **OUTPUT // VIRTUAL CABLE**: where the voice is delivered (must be `CABLE
  Input`).
- **AUTO-ROUTE**: configures it automatically.
- **RESCAN**: searches for devices again (if you plug in a new mic).
- **ENGAGE / DISENGAGE**: turns the engine on or off.
- Bottom: the stream status and the frequency (`48000 HZ`).

### VOICE PRESETS (left, the large block)

- **8 buttons** with the presets: NORMAL, FEMALE, DEEP, ROBOT, DEMON, CHIPMUNK,
  CAVE, ANIME.
- Click one to apply it.
- Below, a **detail card** shows:
  - The preset's name and tag.
  - A description of what it's for.
  - A **FACTOR / WEIGHT** table with the value of pitch, formant, drive, etc.
  - An **EVENT TIMELINE** with the latest changes.

### EFFECT MATRIX (center bottom)

Here you fine-tune the sound by hand. Each row is a parameter; **drag the
slider** to change it:

| Control | What it does |
|---|---|
| **PITCH** | Raises or lowers the pitch (from low to high). |
| **FORMANT** | Changes the "thickness"/timbre (male ↔ female voice without changing pitch). |
| **GATE** | Silences background noise when you're not speaking (works together with **NOISE CANCEL**). |
| **DRIVE** | Adds distortion/saturation. |
| **ROBOT** | Metallic robot effect. |
| **CHORUS** | Wider voice, as if doubled. |
| **ECHO** | Repetition. |
| **REVERB** | Space (cave, large room). |
| **COMP** | Compression (evens out loud and soft volumes). |
| **OUT** | Final output volume. |
| **DEN.** | Noise-cancellation threshold, in **dB** (e.g. −45 dB). |
| **BOARD** | Volume of the soundboard sounds. |

- **NOISE CANCEL** (button at the very top): turns on **noise cancellation**.
  It's a gate/expander: **below the threshold it silences the sound**
  (background noise) and **above it lets your voice through untouched**. It adds
  no artifacts or noise.
- **DEN.** (slider, in dB) = the level above which sound is let through:
  - Raise it (less negative, e.g. −35 dB) if you want it to **cut more**
    background noise (you'll need to speak a bit louder).
  - Lower it (e.g. −55 dB) if your voice is soft and gets cut off.
  - If your voice is cut at the start of a phrase, lower the threshold until it
    comes through well.
- **Double-click** a slider to return it to the middle (neutral value).

### SIGNAL MAP (center top)

- A live **oscilloscope** with your voice.
- **Drag** to move the view (pan).
- **Double-click** to **zoom** (x1 ↔ x4).
- Hover over the **numbered markers** (1–6) to see their reading.

### SOUNDBOARD MATRIX (top right)

It plays inside your voice (the other person hears it). It has three zones:

- **FAVOURITES** (top): your sounds marked as favourites, handy to reach.
  **Click one to play it.**
- **Search box** (below): type to **filter the list by name**. The **X** button
  clears the search.
- **LIBRARY** (full list): all sounds (the 8 from the kit + the ones you
  import). **Click a row to play it.** The thin bar at the bottom shows
  progress.
  - **Right-click** a row to open a menu: *PLAY*, *Add/Remove from
    favourites*, and *Delete sound* (only for sounds you imported).
- **IMPORT**: adds your own sounds. Several formats are supported
  (**WAV, AIFF, FLAC, Ogg, MP3, WMA, M4A…**) and you can select **several at
  once**. Imported sounds and your favourites are **saved** and stay there when
  you reopen the app.
- **STOP** (or the header's RESET button): stops all sounds.
- **USER** sounds are yours; **KIT** sounds are included.

### PROFILES (bottom right) — see section 6

- **PROFILE NAME**: type a name.
- Dropdown list with your saved profiles.
- **SAVE**: saves the current configuration under that name.
- **LOAD**: loads the selected profile.
- **DELETE**: deletes the selected profile.

### EVENT REGISTRY (right, table)

- A log of everything that happens (time, type, source, message, level).
- **Click a header** (TIME, TYPE, SOURCE…) to **sort** by that column. Click
  again to reverse the order.
- Mouse wheel to scroll.

### SOURCE HEALTH (bottom, left)

- A table with the status of your devices: role (CAPTURE/RENDER), state
  (LIVE/IDLE), frequency, buffer, and level (RMS in dB).
- Also sortable by columns.

### OPERATOR MANUAL (bottom right)

- Quick help inside the app itself (mouse/touch only).

### Bottom bar (at the very bottom)

- A help message and the link status (`LINK ESTABLISHED` / `LINK DOWN`).

---

## 5. RESET and SAVE buttons in each section

Many panels have two small buttons in their header:

- **RESET**: returns that section to its default values.
  - *DEVICE ROUTING* → auto-configures the microphone and cable again.
  - *VOICE PRESETS* → returns to the **NORMAL** preset.
  - *EFFECT MATRIX* → returns the sliders to those of the active preset.
  - *SOUNDBOARD* → stops all sounds.
  - *SIGNAL MAP* → resets the zoom and the view offset.
  - *EVENT REGISTRY / SOURCE HEALTH* → clears the sort and selection.
- **SAVE**: saves the current state of the whole app (microphone, cable,
  settings, imported sounds, and favourites) so it's **remembered** the next
  time you open VoicePad.

> Also, when you close the app, **everything is saved automatically**.

---

## 6. Profiles: save and load your configurations

A **profile** is a complete "snapshot" of VoicePad: the microphone, the cable,
and all voice settings. Useful for having, for example, a "Serious Discord"
profile and a "Fun gaming" profile.

1. Set everything as you like (preset, sliders, microphone…).
2. In the **PROFILES** panel, type a name (e.g. `MY GAMING PROFILE`).
3. Click **SAVE**. It will appear in the list.
4. When you want to return to that state, select it in the list and click
   **LOAD**.
5. To delete it, select it and click **DELETE**.

Profiles are stored in a file and **stay there** even after you close the
program.

---

## 7. Color themes (SKIN)

At the top right, the **SKIN** dropdown changes the color of the whole
interface. Included themes:

- **AMBER** (amber, the classic)
- **PHOSPHOR** (phosphor green)
- **ICE** (ice blue)
- **EMBER** (ember orange)
- **SAPPHIRE** (blue)
- **VIOLET** (violet)
- **MONO** (paper white)

The chosen theme is **remembered** the next time you open VoicePad.

---

## 8. Resizing the panels

The window is a **resizable mosaic**. Between the panels there are thin
**bars**:

- **Drag** the **vertical** bar to widen/narrow the columns.
- **Drag** the **horizontal** bar (above the bottom row) to enlarge or shrink
  that row.
- You can also drag the **window** edge to make it bigger.

This way you can, for example, enlarge the **SIGNAL MAP** or the
**SOUNDBOARD** to your liking.

---

## 9. Keyboard shortcuts (optional)

They work even when a game has focus:

| Shortcut | Action |
|---|---|
| `Ctrl+Shift+1` … `Ctrl+Shift+8` | Apply preset 1–8 |
| `Ctrl+Shift+0` | Toggle processing on/off (bypass) |
| `Ctrl+Shift+S` | Stop all soundboard sounds |

> Some games with anti-cheat systems may block the shortcuts. In that case, use
> the interface with the mouse.

---

## 10. Where your settings are stored

Everything lives in `%APPDATA%\VoicePad\`:

- `config.json` → your current state (microphone, cable, and settings,
  including noise cancellation).
- `profiles.json` → your saved profiles.
- `theme.json` → the chosen color theme.
- `sounds.json` → your imported sounds and soundboard favourites.

You can copy that folder to back up your settings.

---

## 11. FAQ and troubleshooting

**`CABLE Input` / `CABLE Output` don't show up.**
Install VB-Audio Cable as administrator and **restart**. Then click
**RESCAN**.

**`AUTO-ROUTE` fails.**
This almost always means VB-Cable isn't installed or you didn't restart.

**They can't hear me in Discord.**
Check that Discord's microphone is `CABLE Output`. And that in VoicePad the
*OUTPUT* is `CABLE Input` and it's **ENGAGED**.

**I hear echo or squealing.**
Don't use speakers; use **headphones**. The echo is your voice coming out of the
speakers and going back into the microphone.

**There's a delay (latency).**
Set Windows to **48 kHz** (Settings → Sound → your device → Advanced properties)
and a low buffer (256–512). Avoid Bluetooth for the microphone.

**The game doesn't pick up the virtual mic.**
Disable "exclusive mode" for `CABLE Output`: Control Panel → Sound → Recording
→ `CABLE Output` → Properties → *Advanced* tab → uncheck *Allow applications to
take exclusive control*.

**I want two different setups.**
Use **PROFILES** (section 6).

**I restart and my changes are forgotten.**
Click **SAVE** in the section you changed, or simply close the app (it saves
itself).

---

## 12. Install and build (for developers)

VoicePad is built with **C++ / JUCE**. JUCE does **not** support MinGW, so you
need **Visual Studio with MSVC**.

### Requirements

- **Visual Studio 2022 or 2026** with the *"Desktop development with C++"*
  workload — <https://visualstudio.microsoft.com/vs/community/>
- **CMake** (comes with Visual Studio; otherwise <https://cmake.org/download/>)
- **Git** — <https://git-scm.com/download/win>
- **VB-Audio Virtual Cable** (see section 2)

### Build

Open **"Developer PowerShell for VS"** and run:

```powershell
git clone https://github.com/Amaru221/Voicepad.git
cd Voicepad
.\build.ps1
```

- `build.ps1` detects your Visual Studio version, cleans old caches, and builds.
- The first time takes a few minutes (it downloads JUCE).
- The resulting program is:
  `build\VoicePad_artefacts\Release\VoicePad.exe`

Rebuild from scratch:

```powershell
.\build.ps1 -Clean
```

### Offline self-tests

Run the audio self-tests without an audio device:

```powershell
.\build\VoicePad_artefacts\Release\VoicePad.exe --selftest
```

A report is written to `%TEMP%\voicepad_selftest.txt`.

### Project structure

```
voice-pad/
  CMakeLists.txt          Configuration (JUCE FetchContent)
  build.ps1               Build script
  README.md               This guide (Spanish)
  README.en.md            This guide (English)
  Assets/
    icon.png              Application icon (exe and taskbar)
    header.svg            Banner shown at the top of the README
  Tools/
    make_icon.py          Icon generator
  Source/
    Main.cpp              Entry point
    MainComponent.*       Panel mosaic + CRT overlay + splitters
    Theme.*               Palette and color themes
    EventLog.h            Event log
    Config/ConfigManager.* State, profiles, and sound library persistence
    Audio/                WASAPI engine, DSP chain, pitch/formant, noise cancellation
    Soundboard/           Synthesized kit + user sounds + favourites
    Hotkeys/              Global hotkeys (Win32)
    Presets/              Preset definitions
    Tests/                Offline audio self-tests (--selftest)
    UI/                   Panels, LookAndFeel, and widgets
    third_party/          signalsmith-stretch (+ signalsmith-linear)
```
