# Offline Bible data sources

`bible.bin` is generated from the CrossWire Bible Society KJV OSIS source:

<https://gitlab.com/crosswire-bible-society/kjv>

The source is the KJV 1769 module data with embedded Strong's numbers. The
CrossWire module metadata grants use of the KJV2003-derived work and identifies
the module distribution license as GPL. The importer is
`tools/import_crosswire.py`.

`strongs.json` is the Strong's dictionary bundle distributed with:

<https://github.com/exergonic/bible>

That bundle identifies the Strong's dictionary as public-domain material, with
the JSON conversion attributed to Open Scriptures under CC BY-SA. The app uses
the dictionary fields for lemma, transliteration, pronunciation, gloss, and
definition.
