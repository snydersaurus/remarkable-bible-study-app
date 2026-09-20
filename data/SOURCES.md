# Offline Bible data sources and notices

This file documents the third-party data shipped in the app. The data is not
covered by the MIT license used for the application code and launcher icon.
The files below retain their upstream licensing and attribution requirements.

## `bible.bin`

`bible.bin` is a generated, compact runtime index produced by
`tools/import_crosswire.py` from the CrossWire Bible Society KJV OSIS source:

- Source repository: <https://gitlab.com/crosswire-bible-society/kjv>
- Module metadata and attribution: <https://gitlab.com/crosswire-bible-society/kjv/-/raw/master/kjv.conf>

The source is the KJV 1769 module with embedded Strong's numbers. CrossWire's
module metadata identifies the distribution license as GPL and attributes the
embedded material to the KJV tradition, the Bible Foundation, the KJV2003
Project at CrossWire, and its listed contributors. The metadata also states
that the KJV2003 Project text is offered freely for any purpose. Those upstream
notices govern the generated Bible bundle; this project does not relicense it
as MIT.

## `strongs.json`

`strongs.json` is the Strong's dictionary bundle distributed with:

- Data bundle: <https://github.com/exergonic/bible>
- Open Scriptures source: <https://github.com/openscriptures/strongs>
- Open Scriptures licensing details: <https://github.com/openscriptures/strongs/pull/12/files>

The exergonic/bible documentation describes the underlying Strong's
dictionaries (1890/1894) as public-domain material and credits the JSON
conversion to Open Scriptures under CC BY-SA. Open Scriptures' repository also
documents the public-domain status of the original dictionary material and the
licenses used for its converted source files. The app uses the dictionary
fields for lemma, transliteration, pronunciation, gloss, derivation, and KJV
usage.

## Attribution summary

When redistributing this app, keep this notice with the data bundle and retain
the upstream notices linked above. The application code and launcher icon are
MIT licensed in the repository root; that MIT license does not replace or
override the licenses for `bible.bin` or `strongs.json`.
