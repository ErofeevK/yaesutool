# Changelog

## Version 1.1.0

### FT-60:
- **Renamed Power Level**: Changed "Med" to "Mid" to align with radio documentation and screen readings.
- **Channel Name Prefix**: You can now prefix the channel name with a `"` symbol to display the frequency while keeping the name in the radio. This symbol was chosen to avoid conflicts with valid characters in channel names.
- **Added Step Column**: An optional Step column has been added. If not provided, the step will default according to the band plan.
- **Fixed Rounding Issue**: Resolved a rounding issue in channel frequency setup.
- **Memory Dump Update**: Reduced the number of updated bytes in the radio memory dump.


## Version 1.0.0

This is the initial release. The source code is derived from [sergev/yaesutool](https://github.com/sergev/yaesutool), 
with the following changes made to the original sources:

- Fixed warnings
- Added CMake support
- Implemented CI build on GitHub