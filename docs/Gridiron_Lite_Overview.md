# Gridiron Lite - Epics, Stories & Project Structure

Nov 10, 2026 - @Mitchell

## Overview

Gridiron Lite (working title) is a college football game with a full dynasty mode, written in C++ on a thin custom engine. It ships under 50 MB and holds 60 fps on a decade-old laptop. Ten epics take it from a window with a field to a shippable game, and every one of them is judged against the budgets below.

| Budget | Target | Hard ceiling |
| --- | --- | --- |
| Install size | 20 MB | 50 MB |
| Frame rate (integrated GPU, \~2016) | 60 fps locked | 60 fps, 1% lows above 55 |
| Simulation step | under 0.5 ms | 1 ms |
| RAM | 128 MB | 256 MB |
| Cold start to menu | under 1 s | 2 s |
| Sim one full dynasty week (all FBS games) | under 50 ms | 250 ms |
| Save file | under 1 MB | 4 MB |

**Tech stack:** C++20 written as "C with classes" (no exceptions, no RTTI, light STL use). It uses SDL3 for windowing, input and audio, and OpenGL 3.3 for rendering. The build runs on CMake with LTO. The look is low-poly vertex-colored 3D with palette-swapped uniforms, and teams are fictional, with a full team and roster editor.

**Story format:** each story reads *As a \[role\], I want \[thing\], so that \[reason\]*, with acceptance criteria (AC). Sizes use points: S = 1-2, M = 3-5, L = 8.

## Epics and stories

Eleven epics hold 59 stories. Story IDs carry their epic number (E2-3 = epic 2, story 3), so tickets can be copied straight into Jira, Linear or GitHub Issues.

### E1 - Engine foundation

Goal: a window, input, a fixed-timestep loop and memory arenas, with nothing allocating during play.

| ID | Story | Acceptance criteria | Size |
| --- | --- | --- | --- |
| E1-1 | As a developer, I want a CMake project with Debug, Release and Profile configs, so that every build is reproducible. | One command builds on Windows, Linux and macOS. Release uses LTO, `-fno-exceptions` and `-fno-rtti`. | S |
| E1-2 | As a player, I want the game to open a window and respond to keyboard and gamepad, so that I can control it. | SDL3 window opens. Input is mapped to actions, not raw keys, and hot-plugging a pad works. | M |
| E1-3 | As a developer, I want a fixed 60 Hz sim step decoupled from rendering, so that physics is deterministic. | Same inputs and seed give the same result every time. Render interpolates between sim states. | M |
| E1-4 | As a developer, I want arena and frame allocators, so that gameplay never calls `new` or `malloc`. | A debug hook asserts if the heap is touched after load. | M |
| E1-5 | As a developer, I want a seedable fast RNG (PCG or xorshift), so that sims and replays are reproducible. | RNG state is a few bytes and is saved with games. | S |
| E1-6 | As a developer, I want a debug overlay with frame time, sim time and memory, so that budgets stay visible. | Toggle with F3. Shows the 1% low frame time. | S |

### E2 — On-field simulation

Goal: 22 players and a ball moving, colliding and resolving plays, rendered as colored dots first.

| ID | Story | Acceptance criteria | Size |
| --- | --- | --- | --- |
| E2-1 | As a developer, I want field state stored as structure-of-arrays for 22 players, so that the sim is cache-friendly. | Whole field state fits in under 4 KB. | S |
| E2-2 | As a player, I want my ball carrier to accelerate, cut and stop based on ratings, so that speed and agility matter. | SPD and AGI ratings change top speed and turn rate measurably. | M |
| E2-3 | As a player, I want to throw passes with lead and arc, so that passing feels skill-based. | Ball follows a 3D arc. Lob vs bullet changes the arc. THP and THA ratings affect speed and error. | L |
| E2-4 | As a player, I want catches, drops, deflections and interceptions, so that coverage matters. | Catch outcome uses CTH vs defender position and rating, plus the RNG. | L |
| E2-5 | As a player, I want tackles, broken tackles and fumbles, so that physical play matters. | Contact resolution compares STR, TAK and carrier ratings. No physics engine. | M |
| E2-6 | As a player, I want blocks to engage and shed, so that the line of scrimmage matters. | Blocker–rusher pairs hold or shed based on BLK vs STR and AGI. | L |

### E3 — Rendering

Goal: low-poly, vertex-colored players and stadiums drawn in very few draw calls.

| ID | Story | Acceptance criteria | Size |
| --- | --- | --- | --- |
| E3-1 | As a developer, I want a minimal OpenGL 3.3 renderer with one shader per material type, so that the renderer stays tiny. | Under 2,000 lines. Under 50 draw calls per frame in a game. | L |
| E3-2 | As a player, I want uniforms in my team's colors, so that 130+ teams share one model. | Colors come from a per-team palette. Helmet stripe and pants pattern are IDs, not textures. | M |
| E3-3 | As a developer, I want all 22 players drawn with instancing, so that player drawing is one or two draw calls. | Instance buffer holds transform, palette index and pose. | M |
| E3-4 | As a player, I want stadiums built from parameters, so that each team has a home field without bespoke models. | Capacity, bowl shape, end zone art and colors generate the stadium at load. | L |
| E3-5 | As a player, I want a broadcast camera and a behind-the-QB camera, so that I can read the field. | Camera switches smoothly. No clipping into players. | M |
| E3-6 | As a player, I want readable field markings, numbers and an end zone name, so that the field looks right. | Markings are generated geometry or a single 256 px atlas. | S |

### E4 — Animation

Goal: a few dozen clips that blend, instead of thousands of motion-captured ones.

| ID | Story | Acceptance criteria | Size |
| --- | --- | --- | --- |
| E4-1 | As a developer, I want a 16–20 bone skeleton with quantized keyframes, so that animation data stays tiny. | All clips together are under 1 MB. | M |
| E4-2 | As a player, I want stance, run, cut, throw, catch, block, tackle and celebrate animations, so that players read clearly. | Around 30 clips cover the game. | L |
| E4-3 | As a player, I want run speed to blend between jog and sprint, so that motion matches velocity. | Foot sliding is not visible at gameplay camera distance. | M |
| E4-4 | As a developer, I want skinning on the GPU, so that 22 skinned players cost little CPU. | Under 0.3 ms CPU for all player poses. | M |

### E5 — Playbook and play calling

Goal: plays are data, and calling them is fast and fun.

| ID | Story | Acceptance criteria | Size |
| --- | --- | --- | --- |
| E5-1 | As a designer, I want a binary play format (formation plus per-player assignment plus route waypoints), so that a playbook is a few KB. | One play is under 128 bytes. A text format compiles to binary. | M |
| E5-2 | As a player, I want a play-call screen grouped by formation, so that I can pick a play in seconds. | Three button presses or fewer from formation to snap. | M |
| E5-3 | As a player, I want hot routes and audibles, so that I can react to the defense. | Change one receiver's route or the whole play before the snap. | M |
| E5-4 | As a player, I want different playbooks per team style (air raid, option, pro), so that teams feel different. | At least 3 offensive and 3 defensive books ship. | L |
| E5-5 | As a player, I want to see route art before the snap, so that I know my reads. | Routes draw over the field on a held button. | S |

### E6 — AI

Goal: believable opponents built on per-player state machines and simple play-calling logic.

| ID | Story | Acceptance criteria | Size |
| --- | --- | --- | --- |
| E6-1 | As a player, I want defenders to play man and zone coverage correctly, so that reading the defense matters. | Zone defenders hold areas and pass off receivers. Man defenders trail and break on the ball. | L |
| E6-2 | As a player, I want defenders to pursue with good angles, so that runs aren't free yards. | Pursuit aims at the predicted carrier position. | M |
| E6-3 | As a player, I want an AI quarterback that progresses through reads, so that CPU offense is credible. | Reads follow the play's read order, with timing affected by AWR. | L |
| E6-4 | As a player, I want the CPU to call plays by situation, so that it punts on 4th and long and goes for it when trailing late. | Play calling uses down, distance, field position, score and clock. | M |
| E6-5 | As a player, I want difficulty levels that change AI awareness, not ratings cheats, so that harder feels fair. | Four levels tweak reaction time and read accuracy only. | S |

### E7 — Rules, HUD and game flow

Goal: a complete game of football from kickoff to final whistle.

| ID | Story | Acceptance criteria | Size |
| --- | --- | --- | --- |
| E7-1 | As a player, I want downs, distance, first-down line and scoring, so that the game works. | Touchdowns, PATs, 2-point tries, field goals and safeties all score correctly. | M |
| E7-2 | As a player, I want a game clock with college rules, so that clock management matters. | Clock stops on first downs, incompletions and out of bounds. Timeouts and two-minute situations work. | M |
| E7-3 | As a player, I want kickoffs, punts and field goals with a kick meter, so that special teams are playable. | Kick power and accuracy follow input and ratings. | L |
| E7-4 | As a player, I want common penalties (false start, offside, holding, pass interference), so that the game feels real. | Penalty chance comes from ratings and discipline. Accept or decline works. | M |
| E7-5 | As a player, I want college overtime rules, so that tied games resolve correctly. | Possession alternates from the 25. 2-point shootout follows current NCAA rules. | M |
| E7-6 | As a player, I want a scorebug and play-result banner, so that I always know the situation. | HUD draws from one font atlas and one UI atlas. | S |

### E8 — Audio

Goal: a stadium that sounds alive in a few MB.

| ID | Story | Acceptance criteria | Size |
| --- | --- | --- | --- |
| E8-1 | As a developer, I want a small software mixer on SDL3 audio, so that there is no audio middleware. | 32 voices, volume and pan, under 0.2 ms per frame. | M |
| E8-2 | As a player, I want crowd volume that rises with game tension, so that big moments feel big. | Crowd level follows score margin, down and field position. | M |
| E8-3 | As a player, I want hits, whistles, cadence calls and a band loop, so that the game sounds like football. | All sound effects together are under 3 MB (Ogg Vorbis or Opus). | M |
| E8-4 | As a player, I want menu and band music, so that the game has character. | Tracker modules (.xm/.it) played with a small library. Under 500 KB total. | S |

### E9 — Dynasty mode

Goal: multi-season college football with recruiting, all held in memory.

| ID | Story | Acceptance criteria | Size |
| --- | --- | --- | --- |
| E9-1 | As a player, I want 130+ teams in conferences with realistic schedules, so that seasons feel like college football. | Schedule generator respects conference games and rivalries. | L |
| E9-2 | As a player, I want other games simulated by a fast stats model, so that weeks advance instantly. | One week of all FBS games sims in under 50 ms. Box scores and stat leaders are produced. | L |
| E9-3 | As a player, I want rankings, conference championships, bowls and a playoff, so that seasons have stakes. | Poll ranking runs each week. Bracket and bowl tie-ins are configurable. | L |
| E9-4 | As a player, I want recruiting with interest, visits and commitments, so that I build my roster. | Recruit pool generated each year. Interest responds to prestige, playing time and pitches. | L |
| E9-5 | As a player, I want players to progress, graduate, redshirt and transfer, so that rosters turn over. | Off-season pipeline runs in under 1 s. | M |
| E9-6 | As a player, I want instant save and load, so that dynasty never waits on disk. | Save is a versioned, compressed dump of the world structs. Under 1 MB. | M |
| E9-7 | As a player, I want procedurally generated player names, so that the roster feels real without shipping a name database. | Names are generated from a seed plus small first and last name tables. | S |

### E10 — Data, editor and modding

Goal: fictional teams by default, with a community able to customize everything.

| ID | Story | Acceptance criteria | Size |
| --- | --- | --- | --- |
| E10-1 | As a player, I want to edit team names, colors, mascots and stadiums, so that I can build the league I want. | All team fields are editable in game. | L |
| E10-2 | As a player, I want to edit rosters and ratings, so that I can match real seasons myself. | All player fields are editable. Changes save to a roster file. | M |
| E10-3 | As a player, I want to import and export roster and league files, so that the community can share them. | Shared file is a single binary under 1 MB with a version header. | M |
| E10-4 | As a modder, I want the play format documented, so that I can make playbooks. | Text play format and compiler are documented and included. | S |

### E11 — Tooling, performance and release

Goal: keep the budgets enforced automatically, then ship.

| ID | Story | Acceptance criteria | Size |
| --- | --- | --- | --- |
| E11-1 | As a developer, I want an asset pipeline that converts source models, sounds and plays to packed binary, so that the runtime never parses text. | One command builds a single `game.pak`. | L |
| E11-2 | As a developer, I want CI that fails when size or speed budgets are broken, so that bloat never sneaks in. | CI checks binary size, pak size and a headless benchmark run. | M |
| E11-3 | As a developer, I want headless sim and replay tests, so that gameplay changes are caught. | 1,000 seeded games sim headless with stat ranges checked. | M |
| E11-4 | As a player, I want settings for resolution, vsync, controls and volume, so that the game fits my setup. | Settings save to a tiny text config. | S |
| E11-5 | As a player, I want a single small download per platform, so that installing is instant. | Windows, Linux and macOS builds under 50 MB each. | M |
| E11-6 | As a developer, I want the build verified on Windows, Linux and macOS in CI, so that E1-1's portability claim stays true. | A CI matrix builds the Debug, Release and Profile configs from a clean checkout on all three platforms. | M |

## Milestones

The epics group into five milestones, and each one ends in a build you can play. The first gate matters most: if the game isn't fun as colored dots, art won't save it.

&#91;embedded content: milestone roadmap · 5 phases, 5 gates\]

Start E11-2 (the CI budget check) during M1, even though the rest of E11 lands at the end, so bloat is caught from the first commit.

## Project structure

The code splits into a platform layer, an engine, the game, and offline tools. Dependencies only point downward: `game` may use `engine`, and `engine` never includes anything from `game`. That keeps the sim testable headless and the engine reusable.

```
gridiron-lite/
├─ CMakeLists.txt
├─ cmake/                    # toolchain flags, LTO, size options
├─ third_party/              # vendored, pinned: SDL3, glad, stb_vorbis, libxmp-lite
├─ src/
│  ├─ platform/              # the only code that touches the OS
│  │  ├─ window.cpp/.h
│  │  ├─ input.cpp/.h        # action mapping, gamepad hot-plug
│  │  ├─ audio_device.cpp/.h
│  │  └─ file_io.cpp/.h      # pak + save file reads/writes
│  ├─ core/                  # no game knowledge, no OS calls
│  │  ├─ arena.h             # arena + frame allocators
│  │  ├─ array.h             # fixed / small vectors, no std::vector in hot code
│  │  ├─ math.h              # vec2/3, mat4, quat, fixed-point helpers
│  │  ├─ rng.h               # PCG32
│  │  ├─ hash.h
│  │  └─ profiler.h          # scoped timers for the F3 overlay
│  ├─ engine/
│  │  ├─ render/
│  │  │  ├─ gl_device.cpp     # GL state, buffers, shaders
│  │  │  ├─ mesh.cpp          # vertex-colored meshes
│  │  │  ├─ skinning.cpp      # GPU skinning, instancing
│  │  │  ├─ palette.cpp       # team color remaps
│  │  │  ├─ camera.cpp
│  │  │  ├─ ui_batch.cpp      # 2D quads, font atlas
│  │  │  └─ shaders/          # embedded as strings at build time
│  │  ├─ anim/                # skeleton, clips, blend
│  │  ├─ audio/               # mixer, voices, tracker playback
│  │  └─ assets/              # pak reader, asset handles
│  ├─ game/
│  │  ├─ sim/                 # E2: on-field simulation, no rendering
│  │  │  ├─ field_state.h     # SoA for 22 players + ball
│  │  │  ├─ movement.cpp
│  │  │  ├─ passing.cpp
│  │  │  ├─ contact.cpp       # blocks, tackles, fumbles
│  │  │  └─ kicking.cpp
│  │  ├─ rules/               # E7: downs, clock, scoring, penalties, OT
│  │  ├─ plays/               # E5: play format, playbooks, play-call logic
│  │  ├─ ai/                  # E6: per-player FSMs, coverage, QB reads, coach
│  │  ├─ dynasty/             # E9: world, schedule, stat sim, polls, recruiting
│  │  │  ├─ world.h           # Team, Player, Recruit packed structs
│  │  │  ├─ schedule.cpp
│  │  │  ├─ stat_sim.cpp
│  │  │  ├─ rankings.cpp
│  │  │  ├─ recruiting.cpp
│  │  │  ├─ offseason.cpp
│  │  │  └─ names.cpp         # procedural name generator
│  │  ├─ present/             # turns sim state into render + audio calls
│  │  ├─ ui/                  # menus, play-call screen, HUD, editor
│  │  └─ save/                # versioned binary save/load
│  └─ main.cpp               # loop: input -> fixed sim steps -> present
├─ data/                     # SOURCE assets, never shipped as-is
│  ├─ models/                # .obj / .gltf low-poly sources
│  ├─ anims/
│  ├─ audio/
│  ├─ music/                 # .xm / .it
│  ├─ plays/                 # text play definitions
│  ├─ league/                # default fictional teams, conferences
│  └─ names/                 # first/last name tables
├─ tools/                    # offline, never shipped
│  ├─ pakbuild/              # data/ -> build/game.pak
│  ├─ playc/                 # text plays -> binary
│  ├─ meshc/                 # quantize meshes + anims
│  └─ budget_check/          # CI size + perf gate
├─ tests/
│  ├─ sim_tests.cpp          # deterministic replays
│  ├─ dynasty_tests.cpp      # stat ranges over 1,000 seasons
│  └─ bench/                 # headless benchmarks run in CI
└─ docs/
   ├─ play_format.md
   └─ roster_format.md
```

| Module | Owns | Depends on | Must never |
| --- | --- | --- | --- |
| `platform` | Window, input, audio device, files | SDL3 | Know about football |
| `core` | Memory, math, RNG, containers, profiling | Nothing | Allocate from the heap after startup |
| `engine` | Rendering, animation, audio mixing, asset loading | `core`, `platform` | Include anything from `game` |
| `game/sim` | Player movement, ball, contact, kicking | `core` | Render, play sound or read the clock |
| `game/rules`, `plays`, `ai` | Football rules, playbooks, decisions | `core`, `game/sim` | Render or play sound |
| `game/dynasty` | League world, schedule, stats, recruiting | `core` | Run the on-field sim for CPU vs CPU games |
| `game/present`, `ui` | Drawing and sound for the game state | everything above | Change sim state directly |
| `tools` | Offline asset conversion and budget checks | `core` | Ship in the game build |

The split between `sim` and `present` is the key rule. Because the sim never draws, you can run 1,000 games headless in tests, record replays as inputs only, and later swap the art style without touching gameplay.

## Build config and guardrails

CI rejects any change that breaks a budget, so the game can't grow without someone deciding it should. Release builds use these compiler and linker settings:

```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)   # LTO

if(MSVC)
  add_compile_options(/GR- /EHs-c- /O2 /Gy /GS-)
  add_link_options(/OPT:REF /OPT:ICF)
else()
  add_compile_options(-fno-exceptions -fno-rtti -O2
                      -ffunction-sections -fdata-sections
                      -fvisibility=hidden)
  add_link_options(-Wl,--gc-sections -s)
endif()
```

| Check | When | Fails if |
| --- | --- | --- |
| Executable size | Every merge | Over 4 MB, or grew more than 5% in one change |
| `game.pak` size | Every merge | Over 40 MB |
| Headless game benchmark | Every merge | Average sim step over 0.5 ms |
| Dynasty week benchmark | Every merge | One week of all games over 50 ms |
| Heap-after-load assert | Every test run | Any heap allocation during gameplay |
| 1,000-game stat test | Nightly | Average points, yards or turnovers fall outside set ranges |
| Replay determinism | Nightly | Any replay produces a different final state |

Two habits keep this honest. First, `budget_check` prints the size of every asset category on each build, so growth is visible before it becomes a problem. Second, any new dependency in `third_party/` needs a stated size cost in its pull request.
