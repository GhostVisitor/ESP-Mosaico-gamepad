"""
test_hid.py - read and decode the CH552 gamepad HID input reports.

Usage:
    python tools/test_hid.py [seconds]        # default 15 s
    (press the keys while it runs)

Report layout (7 bytes, no report ID):
    [0] X  [1] Y  [2] Rx  [3] Ry
    [4] low nibble = hat (0..7, 8 = center), high nibble = padding
    [5] buttons 1..8   [6] buttons 9..16

Works with the 'hidapi' package:  python -m pip install hidapi
"""

import sys
import time

try:
    import hid
except ImportError:
    print("[!] hidapi not installed:  python -m pip install hidapi")
    sys.exit(2)

VID = 0x0079
PID = 0x0006

BTN = {0: 'A', 1: 'B', 2: 'X', 3: 'Y',
       4: 'LB', 5: 'RB', 6: 'LT', 7: 'RT',
       8: 'Back', 9: 'Start', 10: 'L3', 11: 'R3'}
HAT = {0: 'Up', 1: 'Up-Right', 2: 'Right', 3: 'Down-Right',
       4: 'Down', 5: 'Down-Left', 6: 'Left', 7: 'Up-Left', 8: 'center'}


def open_device():
    nodes = list(hid.enumerate(VID, PID))
    if not nodes:
        print("[!] no HID node with VID=%04X PID=%04X" % (VID, PID))
        return None
    # prefer the top-level collection "Generic Desktop / Game Pad"
    pick = None
    for n in nodes:
        up = n.get('usage_page') if isinstance(n, dict) else getattr(n, 'usage_page', None)
        us = n.get('usage') if isinstance(n, dict) else getattr(n, 'usage', None)
        if up == 0x01 and us == 0x05:
            pick = n
            break
    if pick is None:
        pick = nodes[0]
    path = pick.get('path') if isinstance(pick, dict) else pick.path
    name = (pick.get('product_string') if isinstance(pick, dict) else pick.product_string) or '?'

    try:                                    # hidapi >= 0.14 style
        dev = hid.Device(path=path)
    except AttributeError:                  # older style
        dev = hid.device()
        dev.open_path(path)
    return dev, name


def read_report(dev, size=16, timeout=200):
    try:
        return dev.read(size, timeout)
    except TypeError:                       # older signature
        try:
            dev.nonblocking = 1
        except Exception:
            pass
        time.sleep(timeout / 1000.0)
        return list(dev.read(size))


def decode(r):
    btns = r[5] | (r[6] << 8)
    names = [BTN[i] for i in sorted(BTN) if btns & (1 << i)]
    return "[X=%3d Y=%3d Rx=%3d Ry=%3d hat=%-10s] %s" % (
        r[0], r[1], r[2], r[3], HAT.get(r[4] & 0x0F, '?'),
        '+'.join(names) if names else '-')


def main():
    secs = float(sys.argv[1]) if len(sys.argv) > 1 else 15.0
    op = open_device()
    if op is None:
        return 1
    dev, name = op
    print("[hid] opened: %s" % name)
    print("[hid] listening %.0f s - PRESS THE KEYS NOW (Ctrl+C to quit)" % secs)
    print("-" * 72)

    deadline = time.time() + secs
    last = None
    seen = 0
    try:
        while time.time() < deadline:
            r = read_report(dev)
            if not r:
                continue
            seen += 1
            line = decode(r)
            if line != last:
                last = line
                print("%7.3f  %s" % (time.time() % 1000.0, line))
    finally:
        try:
            dev.close()
        except Exception:
            pass

    print("-" * 72)
    print("[hid] reports received: %d" % seen)
    if seen == 0:
        print("[!] no report at all -> the firmware never sent / the endpoint is wrong")
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
