# The Word — reMarkable Paper Pro Move

`The Word` is a native QML offline KJV word-study app for the Paper Pro
Move. The current build includes:

- the full 66-book KJV corpus (31,102 verses)
- individual tappable word tokens
- Strong's number, original-language lemma, transliteration, gloss,
  derivation, and definition
- related lexical-family links that jump to another tagged word in the verse
- verse search, chapter reading, and book/chapter/verse browsing
- page-sized Strong's occurrence lookup with offline navigation
- a desktop preview and screenshot mode
- a frontend/backend AppLoad split with a compact bundled corpus

## Preview

The machine used to author this app does not have Qt installed. On a machine
with Qt 6.4 or newer:

```bash
brew install qt ninja
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
./build/word_study
```

That opens a small interactive desktop preview. For an exact-size Move
screenshot:

```bash
./build/word_study --panel 954x1696 --shot /tmp/word-study-move.png
open /tmp/word-study-move.png
```

The existing sibling apps document the Paper Pro SDK and AppLoad deployment
workflow in `../football-scoreboard/docs/PLATFORM.md`. The release build for
this app should use an SDK no newer than the tablet's Qt version.

## Data bundle

`tools/import_crosswire.py` converts the CrossWire KJV OSIS source into
`data/bible.bin`. The binary preserves the KJV verse text, tagged word groups,
and embedded Strong's IDs. `data/strongs.json` supplies the Strong's Hebrew and
Greek dictionary entries, including derivation and KJV usage fields. Both files are copied into the AppLoad bundle by
`build.sh` and deployed beside the backend.

The state shape is:

```text
verse → word token → Strong's ID → lexicon entry → related IDs
```

The source and attribution details are in `data/SOURCES.md`.
