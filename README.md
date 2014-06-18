# xmppench - XMPP Server Benchmark

## License:

This tool falls under GPLv3 license, see gpl-3.0.txt, due to Swiften's licensing.

## Dependencies:

- Swiften
	Requires latest Swiften version from the Git repository.
- Boost

## Building Instructions:

### Using Scons inside of a Swift repository checkout

```bash
git clone git://swift.im/swift
cd swift
hg clone http://code.google.com/p/xmppench/
./scons xmppench
```

### Using Makefile

1. Adjust Makefile to your needs; i.e. so that xmppench builds
2. `make`

Alternatively you can adjust the qmake project file and build it using qmake. That requires heavy Qt though.

## Help

`xmppench --help`

## Known Issues

- Might block in the "Finish sessions..."-phase. Will be fixed by a future version of Swiften.

## Author

Tobias Markmann  
xmpp:   tm@ayena.de  
mailto: tm@ayena.de

## Version History

0.1
- initial release
