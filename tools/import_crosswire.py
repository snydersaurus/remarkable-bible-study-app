#!/usr/bin/env python3
"""Convert the CrossWire KJV OSIS source into a small offline app bundle.

The CrossWire source stores each verse as an OSIS start marker, a sequence of
word elements, and an end marker.  This importer preserves those word groups
and the embedded Strong's lemmas while removing the presentation markup.  The
runtime reads the resulting binary directly on the Paper Pro Move.
"""

from __future__ import annotations

import argparse
import html
import re
import struct
from collections import OrderedDict
from pathlib import Path


BOOKS = OrderedDict(
    [
        ("Gen", "Genesis"),
        ("Exod", "Exodus"),
        ("Lev", "Leviticus"),
        ("Num", "Numbers"),
        ("Deut", "Deuteronomy"),
        ("Josh", "Joshua"),
        ("Judg", "Judges"),
        ("Ruth", "Ruth"),
        ("1Sam", "1 Samuel"),
        ("2Sam", "2 Samuel"),
        ("1Kgs", "1 Kings"),
        ("2Kgs", "2 Kings"),
        ("1Chr", "1 Chronicles"),
        ("2Chr", "2 Chronicles"),
        ("Ezra", "Ezra"),
        ("Neh", "Nehemiah"),
        ("Esth", "Esther"),
        ("Job", "Job"),
        ("Ps", "Psalms"),
        ("Prov", "Proverbs"),
        ("Eccl", "Ecclesiastes"),
        ("Song", "Song of Solomon"),
        ("Isa", "Isaiah"),
        ("Jer", "Jeremiah"),
        ("Lam", "Lamentations"),
        ("Ezek", "Ezekiel"),
        ("Dan", "Daniel"),
        ("Hos", "Hosea"),
        ("Joel", "Joel"),
        ("Amos", "Amos"),
        ("Obad", "Obadiah"),
        ("Jonah", "Jonah"),
        ("Mic", "Micah"),
        ("Nah", "Nahum"),
        ("Hab", "Habakkuk"),
        ("Zeph", "Zephaniah"),
        ("Hag", "Haggai"),
        ("Zech", "Zechariah"),
        ("Mal", "Malachi"),
        ("Matt", "Matthew"),
        ("Mark", "Mark"),
        ("Luke", "Luke"),
        ("John", "John"),
        ("Acts", "Acts"),
        ("Rom", "Romans"),
        ("1Cor", "1 Corinthians"),
        ("2Cor", "2 Corinthians"),
        ("Gal", "Galatians"),
        ("Eph", "Ephesians"),
        ("Phil", "Philippians"),
        ("Col", "Colossians"),
        ("1Thess", "1 Thessalonians"),
        ("2Thess", "2 Thessalonians"),
        ("1Tim", "1 Timothy"),
        ("2Tim", "2 Timothy"),
        ("Titus", "Titus"),
        ("Phlm", "Philemon"),
        ("Heb", "Hebrews"),
        ("Jas", "James"),
        ("1Pet", "1 Peter"),
        ("2Pet", "2 Peter"),
        ("1John", "1 John"),
        ("2John", "2 John"),
        ("3John", "3 John"),
        ("Jude", "Jude"),
        ("Rev", "Revelation"),
    ]
)

VERSE_RE = re.compile(
    r'<verse\s+osisID="([^"]+)"[^>]*/>(.*?)<verse\s+eID="[^"]+"\s*/>',
    re.DOTALL,
)
WORD_RE = re.compile(r"<w\b([^>]*)>(.*?)</w>", re.DOTALL)
ATTR_RE = re.compile(r'([A-Za-z_:][\w:.-]*)="([^"]*)"')
TAG_RE = re.compile(r"<[^>]+>")
STRONG_RE = re.compile(r"strong:([HG]\d+)")


def clean_text(value: str) -> str:
    value = TAG_RE.sub("", value)
    value = html.unescape(value)
    value = re.sub(r"\s+", " ", value).strip()
    value = re.sub(r"\s+([,.;:!?])", r"\1", value)
    value = re.sub(r"([\(\[])\s+", r"\1", value)
    return value


def strong_id(value: str) -> str:
    return value[0] + str(int(value[1:]))


def write_string(handle, value: str) -> None:
    raw = value.encode("utf-8")
    handle.write(struct.pack("<I", len(raw)))
    handle.write(raw)


def parse_osis(source: str):
    verses = []
    chapter_sets = {osis: set() for osis in BOOKS}

    for match in VERSE_RE.finditer(source):
        osis_id = match.group(1)
        parts = osis_id.split(".")
        if len(parts) != 3 or parts[0] not in BOOKS:
            continue
        osis, chapter, number = parts
        chapter = int(chapter)
        number = int(number)
        body = match.group(2)
        tokens = []

        def replace_word(word_match):
            attrs = dict(ATTR_RE.findall(word_match.group(1)))
            text = clean_text(word_match.group(2))
            ids = [strong_id(item) for item in STRONG_RE.findall(attrs.get("lemma", ""))]
            if text:
                tokens.append((text, ids))
            return word_match.group(2)

        plain = WORD_RE.sub(replace_word, body)
        text = clean_text(plain)
        if not text:
            continue
        verses.append(
            {
                "book": osis,
                "chapter": chapter,
                "verse": number,
                "text": text,
                "tokens": tokens,
            }
        )
        chapter_sets[osis].add(chapter)

    missing = [osis for osis in BOOKS if not chapter_sets[osis]]
    if missing:
        raise RuntimeError(f"missing books in OSIS source: {', '.join(missing)}")
    return verses, chapter_sets


def write_binary(output: Path, verses, chapter_sets) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open("wb") as handle:
        handle.write(b"WSB1")
        handle.write(struct.pack("<HHI", 1, len(BOOKS), len(verses)))
        for osis, name in BOOKS.items():
            write_string(handle, name)
            chapters = sorted(chapter_sets[osis])
            handle.write(struct.pack("<H", len(chapters)))
            for chapter in chapters:
                handle.write(struct.pack("<H", chapter))

        for verse in verses:
            handle.write(struct.pack("<HHH", list(BOOKS).index(verse["book"]),
                                     verse["chapter"], verse["verse"]))
            write_string(handle, verse["text"])
            handle.write(struct.pack("<I", len(verse["tokens"])))
            for text, ids in verse["tokens"]:
                write_string(handle, text)
                handle.write(struct.pack("<B", len(ids)))
                for item in ids:
                    write_string(handle, item)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    source = args.source.read_text(encoding="utf-8")
    verses, chapter_sets = parse_osis(source)
    write_binary(args.output, verses, chapter_sets)
    print(f"wrote {args.output} ({len(verses):,} verses)")


if __name__ == "__main__":
    main()
