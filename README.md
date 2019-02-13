# ICalLib
Parser and writer for ICalendar files.


## TODO

- [ ] files with unix line endings can only be parsed with `ifstream::binary`, otherwise the parser freaks out upon the end of the first line.
- [ ] rearrange the codes to better reflect properties and objects. see [here](https://en.wikipedia.org/wiki/File:ICalendarSpecification.png):
      ![](https://upload.wikimedia.org/wikipedia/commons/c/c0/ICalendarSpecification.png)
- [ ] parse functions should be more brutal when it's clear that something's not a syntax error but a format error. I.e., when there "BEGIN:XXX", with an unknown "XXX", it should be a file format error, not just a syntax error
- [ ] set default params according to RFC
- [ ] refine structures "but if one occurs, so MUST the other."
