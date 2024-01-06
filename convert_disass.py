#!/usr/local/bin/python3

import re
import sys

if __name__ == "__main__":
#    print(f"Arguments count: {len(sys.argv)}")
#    for i, arg in enumerate(sys.argv):
#        print(f"Argument {i:>6}: {arg}")

    for filename in sys.argv[1:]:
        print(f"Converting file: {filename}")

#        i = 20

        with open(filename, 'r') as inf:
            with open(filename + '.dis', 'w+') as outf:
                for line in inf:
                    outf.write(re.sub('^\d*\t\d*\t', '', line))

#                    print('ORIG:', line)
#                    print('LINE:', re.sub('^\d*\t\d*\t', '', line))
#
#                    i += 1
#                    if i > 200:
#                        break

