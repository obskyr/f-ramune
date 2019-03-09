#!/usr/bin/env python3

import argparse
import json
import os
import struct
import sys
import serial
from binascii import crc32
from collections import OrderedDict
from contextlib import contextmanager

BAUD_RATE = 115200
MIN_TIMEOUT = 1

PROTOCOL_VERSION = 0
ENDIANNESS = '>'

def serial_without_dtr(port, *args, **kwargs):
    """Construct a Serial object with DTR immediately disabled.
    On systems where pySerial supports this, this will prevent an Arduino from
    resetting when a serial connection is opened. On other systems, a hardware
    workaround (e.g. a 10 µF capacitor between RST and ground) is needed.
    """
    ser = serial.Serial(None, *args, **kwargs)
    ser.port = port
    ser.dtr = 0
    if port is not None: # To match what pySerial does.
        ser.open()
    return ser

def appropriate_timeout(length):
    """Return a reasonable timeout value for transferring `length`
    bytes at F-Ramune's baud rate."""
    return max(MIN_TIMEOUT, 1.5 * (length / (BAUD_RATE // 8)))

@contextmanager
def temp_timeout(ser, timeout):
    original_timeout = ser.timeout
    ser.timeout = timeout
    yield
    ser.timeout = original_timeout

class Framune(object):
    def __init__(self, serial_port):
        if hasattr(serial_port, 'port'):
            self.serial_port = serial_port.port
            self._serial = serial_port
        else:
            self.serial_port = serial_port
            self._serial = serial_without_dtr(serial_port, BAUD_RATE,
                                              timeout=MIN_TIMEOUT,
                                              inter_byte_timeout=MIN_TIMEOUT)
        self._chip = MemoryChip(None, None, None, None, framune=self)
    
    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.close()

    def close(self):
        self._serial.close()
    
    @property
    def chip(self):
        return self._chip
    
    @chip.setter
    def chip(self, chip):
        self._set_and_analyze_chip(chip)
    
    def _read(self, length=1):
        try:
            data = self._serial.read(length)
            if len(data) < length:
                raise TimeoutError
        except (serial.SerialTimeoutException, TimeoutError):
            raise TimeoutError("F-Ramune did not respond in time.")
        else:
            return data
    
    def _write(self, data):
        self._serial.write(data)

    def _read_uint(self, fmt, length):
        return struct.unpack(fmt, self._read(length))[0]
    
    def _write_uint(self, fmt, n):
        self._write(struct.pack(fmt, n))

    def _read_byte(self):
        return self._read_uint(ENDIANNESS + 'B', 1)
    
    def _write_byte(self, n):
        self._write_uint(ENDIANNESS + 'B', n)
    
    def _read_uint16(self):
        return self._read_uint(ENDIANNESS + 'H', 2)

    def _write_uint16(self, n):
        self._write_uint(ENDIANNESS + 'H', n)

    def _read_uint32(self):
        return self._read_uint(ENDIANNESS + 'I', 4)

    def _write_uint32(self, n):
        self._write_uint(ENDIANNESS + 'I', n)

    def _command(self, command):
        self._write_byte(command)
        if self._read_byte() == command:
            self._write_byte(0x00)
        else:
            self._write_byte(0x01)
            raise ConnectionError("Command didn't reach F-Ramune intact.")
    
    def _set_and_analyze_chip(self, chip):
        self._command(0x01)
        self._write(chip.known_status_to_bytes())
        self._write(chip.to_bytes())
        self._chip = MemoryChip.from_bytes(
            self._read(MEMORY_CHIP_KNOWN_DATA_STRUCTURE_SIZE),
            self._read(MEMORY_CHIP_DATA_STRUCTURE_SIZE),
            framune=self
        )

    def get_version(self):
        """Return the protocol version of the F-Ramune."""
        self._command(0x00)
        return self._read_uint16()
    
    def version_matches(self):
        """Return True if the F-Ramune has the same protocol version
        as the script does; False if not.
        """
        return self.get_version() == PROTOCOL_VERSION
    
    def analyze(self):
        self._set_and_analyze_chip(MemoryChip(None, None, None, None))

    def read(self, address, length):
        """Return up to `length` bytes read starting at `address` from
        the memory chip currently connected to the F-Ramune.
        """
        self._command(0x02)
        self._write_uint32(address)
        self._write_uint32(length)
        length = self._read_uint32()

        with temp_timeout(self._serial, appropriate_timeout(length)):
            data = self._read(length)
        
        received_crc = self._read_uint32()
        computed_crc = crc32(data)
        if received_crc != computed_crc:
            raise ConnectionError("The computed checksum didn't match the one "
                                  "received from the F-Ramune.")
        
        return data
    
    def write(self, address, data):
        """Write the bytes `data` to the memory chip currently connected to
        the F-Ramune, starting at `address`."""
        length = len(data)
        self._command(0x03)
        # Unused at the moment. Who needs EEPROM support anyway...
        is_slow = self._read_byte()
        self._write_uint32(address)
        self._write_uint32(length)
        length = self._read_uint32()
        data = data[:length]

        with temp_timeout(self._serial, appropriate_timeout(length)):
            self._write(data)
            # Receiving the CRC really only transfers 4 bytes, but the F-Ramune
            # operates on all of the bytes written to compute it, so it takes
            # time, and thus needs a more lenient timeout. appropriate_timeout
            # does that job well enough (it's a bit too lenient here, even).
            received_crc = self._read_uint32()
        error_code = self._read_byte()
        computed_crc = crc32(data)
        if received_crc != computed_crc:
            raise ConnectionError("The computed checksum didn't match the one "
                                  "received from the F-Ramune.")
        if error_code != 0:
            raise ConnectionError("Write failed. "
                                  "Is there really a memory chip connected?")
        return length

def framune_updating_property(internal_name):
    def getter(self):
        return getattr(self, internal_name)
    prop = property(getter)
    def setter(self, value):
        setattr(self, internal_name, value)
        if self._framune is not None:
            self._framune.chip = self
    with_setter = prop.setter(setter)
    return with_setter

MEMORY_CHIP_DATA_STRUCTURE = OrderedDict((
    ('is_operational', '?'),
    ('size', 'I'),
    ('is_nonvolatile', '?'),
    ('is_eeprom', '?')
))
MEMORY_CHIP_DATA_STRUCTURE_FMT = ENDIANNESS + \
    ''.join(MEMORY_CHIP_DATA_STRUCTURE.values())
MEMORY_CHIP_DATA_STRUCTURE_SIZE = \
    struct.calcsize(MEMORY_CHIP_DATA_STRUCTURE_FMT)
MEMORY_CHIP_KNOWN_DATA_STRUCTURE_FMT = ENDIANNESS + \
    '?' * len(MEMORY_CHIP_DATA_STRUCTURE)
MEMORY_CHIP_KNOWN_DATA_STRUCTURE_SIZE = \
    struct.calcsize(MEMORY_CHIP_KNOWN_DATA_STRUCTURE_FMT)
class MemoryChip(object):
    def __init__(self, is_operational=None, size=None,
                 is_nonvolatile=None, is_eeprom=None, framune=None):
        self._is_operational = is_operational
        self._size = size
        self._is_nonvolatile = is_nonvolatile
        self._is_eeprom = is_eeprom
        self._framune = framune

    @classmethod
    def from_bytes(cls, known, properties, framune=None):
        known = struct.unpack(MEMORY_CHIP_KNOWN_DATA_STRUCTURE_FMT, known)
        values = struct.unpack(MEMORY_CHIP_DATA_STRUCTURE_FMT, properties)
        return cls(**{
            k: (v if is_known else None)
            for k, v, is_known
            in zip(MEMORY_CHIP_DATA_STRUCTURE, values, known)
        }, framune=framune)
    
    def __repr__(self):
        return "<MemoryChip: {}>".format(', '.join('{}={}'.format(
            attr, getattr(self, attr)
        ) for attr in MEMORY_CHIP_DATA_STRUCTURE))

    is_operational = framune_updating_property('_is_operational')
    size           = framune_updating_property('_size')
    is_nonvolatile = framune_updating_property('_is_nonvolatile')
    is_eeprom      = framune_updating_property('_is_eeprom')

    def known_status_to_bytes(self):
        return bytes(int(getattr(self, attr) is not None)
            for attr in MEMORY_CHIP_DATA_STRUCTURE)

    def to_bytes(self):
        return struct.pack(MEMORY_CHIP_DATA_STRUCTURE_FMT, *(
            getattr(self, attr) or 0 for attr in MEMORY_CHIP_DATA_STRUCTURE
        ))

def main(*argv):
    script_name = os.path.basename(__file__)

    # Tiny details!
    class KindArgumentParser(argparse.ArgumentParser):
        def error(self, message):
            print(self.usage % {'prog': self.prog}, file=sys.stderr, end="\n\n")
            print(self.description % {'prog': self.prog}, file=sys.stderr, end="\n\n")
            print("Run \"{} --help\" for more detailed information!".format(self.prog), file=sys.stderr)
            sys.exit(1)

    # More tiny details!
    class ProperHelpFormatter(argparse.RawTextHelpFormatter):
        def add_usage(self, usage, actions, groups, prefix=None):
            if prefix is None:
                prefix = 'Usage: '
            return super(ProperHelpFormatter, self).add_usage(usage, actions, groups, prefix)

    parser = KindArgumentParser(
        prog=script_name,
        usage="%(prog)s [-h] [--analyze] [--no-version-check] <port> "
              "<version|analyze|read|write> ...",
        description="Interface with an F-Ramune (memory chip programmer and tester).\n\n"
        "Examples:\n"
        "%(prog)s COM5 analyze\n"
        "%(prog)s /dev/ttyS2 read -a 0x1000 -s 0x100 -o data.hex\n"
        "%(prog)s /dev/tty.usbserial-A6004byf write -i data.hex",
        formatter_class=ProperHelpFormatter,
        add_help=False
    )
        
    try: # Even more tiny details!
        parser._positionals.title = "Positional arguments"
        parser._optionals.title = "Optional arguments"
    except AttributeError:
        pass

    parser.add_argument(
        'port',
        help="The serial port your F-Ramune is connected to."
    )
    parser.add_argument(
        'command', metavar='command',
        help="What to do. Valid commands are: \"version\", \"analyze\", \"read\", and \"write\".",
        choices=('version', 'analyze', 'read', 'write')
    )
    
    parser.add_argument(
        '-h', '--help',
        action='help', default=argparse.SUPPRESS,
        help="Show this help and exit."
    )
    parser.add_argument(
        '--analyze',
        action='store_true',
        help="Before running the command, analyze the chip to set the correct properties."
    )
    parser.add_argument(
        '--no-version-check',
        action='store_true',
        help="Skip verifying that the script's and the F-Ramune's protocol versions match."
    )
    parser.add_argument(
        '-a', '--address', metavar='address', type=int, default=0,
        help="Used with the \"read\" and \"write\" commands. The address to start at."
             "Defaults to 0."
    )
    parser.add_argument(
        '-s', '--size', metavar='size', type=int, default=None,
        help="Used with the \"read\" and \"write\" commands. The number of bytes."
             "Required for reading if not using --analyze."
    )
    parser.add_argument(
        '-i', metavar='path',
        help="Used with the \"write\" command. The file to get the data to write from.\n"
             "By omitting this and piping input, the data can be gotten from stdin."
    )
    parser.add_argument(
        '-o', metavar='path',
        help="Used with the \"read\" command. The file to save the read data to.\n"
             "By omitting this and piping output, the data can be output to stdout."
    )
    parser.add_argument(
        '-j', '--json', action='store_true',
        help="Used with the \"analyze\" command. Outputs the chip information in JSON form."
    )

    arguments = parser.parse_args(argv)

    if arguments.command == 'read' and arguments.o is None and not sys.stdout.isatty():
        print("No output specified! Please either specify -o or pipe output.",
              file=sys.stderr)
        return 1
    if arguments.command == 'write' and arguments.i is None and not sys.stdin.isatty():
        print("No input specified! Please either specify -i or pipe input.",
              file=sys.stderr)
        return 1

    with Framune(arguments.port) as framune:
        if not arguments.no_version_check and arguments.command != 'version':
            version = framune.get_version()
            if version < PROTOCOL_VERSION:
                print("The connected F-Ramune is running outdated software! "
                      "Please update it.",
                      file=sys.stderr)
                return 1
            elif version > PROTOCOL_VERSION:
                print("The connected F-Ramune is running a newer protocol version "
                      "than this program!\n"
                      "Please update {}.".format(script_name),
                      file=sys.stderr)
                return 1
        
        if arguments.analyze and not arguments.analyze:
            framune.analyze()
        
        if arguments.command == 'version':
            print(framune.get_version())
        elif arguments.command == 'analyze':
            framune.analyze()
            if arguments.json:
                properties = OrderedDict(
                    (k, getattr(framune.chip, k))
                    for k in MEMORY_CHIP_DATA_STRUCTURE
                )
                print(json.dumps(properties, indent=4))
            else:
                yn = lambda x: "Yes" if x else "No"
                def format_size(size):
                    n = size
                    for unit in ("", "KiB", "MiB"):
                        if n < 1024:
                            break
                        n /= 1024
                    else:
                        unit = "GiB"
                    n = "{}".format(round(n)) if n - int(n) < 0.00001 else "~{:.1f}".format(n)
                    return "{} ({} {})".format(size, n, unit)
                rows = (
                    ('is_operational', "Is operational:  ", yn),
                    ('size',           "Size:            ", format_size),
                    ('is_nonvolatile', "Is non-volatile: ", yn),
                    ('is_eeprom',      "Is EEPROM:       ", yn)
                )
                for attr, label, transformer in rows:
                    attr = getattr(framune.chip, attr)
                    value = transformer(attr) if attr is not None else "Unknown"
                    print("{}{}".format(label, value))

    return 0

if __name__ == '__main__':
    sys.exit(main(*sys.argv[1:]))
