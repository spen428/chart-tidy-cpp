# chart-tidy

Automatically find and fix common issues with Guitar Hero III charts. Also provides a few metrics, such as average and peak notes-per-second.

## How to use

Usage: `chart-tidy CHART_FILE [-fb] > NEW_CHART_FILE`, where the `-fb` flag disables automatic conversion of track events into tap/force notes (see *How problems are fixed* below). Example output:

	```
	$ ./chart-tidy ../test/ltest.chart > ltest.fixed.chart
	Inserted end event at time 113636
	Cannot fix no_leading_measure: offset is less than 1
	Sustain gap too small between [34968, 1000001, 24] and [34992, 1000100, 24]
	Duration changed from 24 to 0 (-24)
	Sustain gap too small between [34992, 1000100, 24] and [35016, 1000001, 0]
	Duration changed from 24 to 0 (-24)
	Sustain gap too small between [35112, 1000001, 24] and [35136, 1000010, 0]
	Duration changed from 24 to 0 (-24)
	Sustain gap too small between [37080, 1001000, 24] and [37104, 1000100, 0]
	Duration changed from 24 to 0 (-24)
	```

## What it can detect

0. **Markers for tap notes and force notes** Since charting tool FeedBack crashes when loading charts containing tap notes and force notes, charters often mark them with FeedBack track events instead, e.g. `E *` for force and `E t` for tap. These must be replaced with `N 5 0` and `N 6 0` respectively before importing the chart into the game.
1. **No practice sections** A chart should contain at least one practice section. A chart with no practice sections cannot be played in practice mode, and may also cause syncronisation issues if pausing during play.
2. **No end event** The end of a song chart should be marked with the `E "end"` event in the `[Events]` section of the chart file.
3. **No leading measure** If the first notes of a chart appear in the very first measure, the HOPO calculation can be incorrect during that measure. By having at least one empty measure before the first note this is prevented.
4. **Inadequate sustain gap** If the gap between the end of a sustain note and the start of the next note is too small, it may not be possible to play the song without audio dropout from releasing sustain notes too early.
5. **Unsupported characters** Some characters are not part of the in game font, and so will not be displayed properly if used in the song title, artist name, or practice section names. WIP
6. **Preview window not set** With no defined preview window, the song preview will simply start at the beginning of the song. WIP
7. **Extended sustain chords** Extended sustain chords, as found in later Guitar Hero games, are not (yet) supported in Guitar Hero III. WIP
8. **Other unsupported note sequences** WIP

## How problems are fixed

0. Track event markers are automatically replaced unless the `-fb` flag is passed when calling the application.

1. A practice section `E "section Start"` will be inserted at time 0.

2. `E "end"` will be inserted 100 time units after the end of the last note in the chart. e.g. if the last note is at time 12345 and has a duration of 64, the end section will be inserted at time (12345 + 64 + 100) = 12509.

3. If the song's `offset` is greater than or equal to 1, all notes and events in all tracks will be shifted forwards by 1 second, the offset will be decreased by 1 second, and a measure of 2/4 at 120 BPM will be inserted at time 0. If the song's offset is less than 1, the fix cannot be applied. The charter should add one second of silence to the beginning of their audio track and then increase the chart's offset by 1 in order to allow the fix to be applied.

4. The length of the sustain notes will be decreased to their maximum legal length. By default the minimum gap is the length of a 1/32 note, which is 24 time units. By default if the note or chord after the sustain is the same, this fix is not applied.

5. Any unsupported characters are replaced with a hyphen `-`.

6. The preview window is set to begin at the chart's offset value. This does nothing if the offset value is 0, but may cut any leading silence out of the preview if it is greater than zero.

7. The extended sustain is broken up into its equivalent set of individual HOPO chords. If an extended sustain involves the releasing of a note, then the **inadequate sustain gap** fix will also be applied if necessary. For example:

	```
	||||           ||||       | |           | |
	|||*           |||*       | |           | |
	|||   becomes  |||   and  ||*  becomes  * *
	||*            ***        ||            ||
	|o             oo         ||            ||
	o              o          oo            oo
	```

8. WIP

