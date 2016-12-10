Fixes:

If no "end" section is found, one will be inserted 100 time units after the end of the last note in
the chart. e.g. if the last note is at time 12345 and has a duration of 64, the end section will be
inserted at time (12345 + 64 + 100) = 12509.
